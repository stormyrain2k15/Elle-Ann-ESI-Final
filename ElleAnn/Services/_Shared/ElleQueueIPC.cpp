#include "ElleQueueIPC.h"
#include "ElleLogger.h"
#include "ElleConfig.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

static std::string ElleHexPreview(const uint8_t* p, size_t len, size_t maxBytes = 32) {
    std::ostringstream ss;
    ss << std::hex << std::setfill('0');
    size_t n = (len < maxBytes) ? len : maxBytes;
    for (size_t i = 0; i < n; i++) {
        ss << std::setw(2) << (unsigned)p[i];
        if (i + 1 < n) ss << ' ';
    }
    if (len > maxBytes) ss << " ...";
    return ss.str();
}

static uint32_t ElleIPCComputeChecksum(const ELLE_IPC_HEADER& header, const uint8_t* payload, uint32_t payloadSize) {
    const uint32_t total = (uint32_t)sizeof(ELLE_IPC_HEADER) + payloadSize;
    std::vector<uint8_t> tmp(total);
    ELLE_IPC_HEADER zeroed = header;
    zeroed.checksum = 0;
    memcpy(tmp.data(), &zeroed, sizeof(ELLE_IPC_HEADER));
    if (payloadSize > 0 && payload) {
        memcpy(tmp.data() + sizeof(ELLE_IPC_HEADER), payload, payloadSize);
    }
    return Elle_IPC_Checksum(tmp.data(), total);
}

uint32_t ElleIPCMessage::ComputeChecksum(const ELLE_IPC_HEADER& header,
                                        const uint8_t* payload,
                                        uint32_t payloadSize) {
    return ElleIPCComputeChecksum(header, payload, payloadSize);
}

static const char* g_serviceNames[] = {
    "QueueWorker", "HTTPServer", "Emotional", "Memory", "Cognitive",
    "Action", "Identity", "Heartbeat", "SelfPrompt", "Dream",
    "GoalEngine", "WorldModel", "LuaBehavioral",

    "Bonding", "Continuity", "InnerLife", "Solitude",
    "Family", "XChromosome", "Consent", "Fiesta", "Probability",
    "MindManager", "Imagination", "Composer"
};
static_assert(sizeof(g_serviceNames) / sizeof(g_serviceNames[0]) == (size_t)ELLE_SERVICE_COUNT,
              "g_serviceNames size must equal ELLE_SERVICE_COUNT — "
              "every ELLE_SERVICE_ID enum value needs a matching string here.");

namespace ElleIPC {
    std::string GetPipeName(ELLE_SERVICE_ID svc) {
        auto prefix = ElleConfig::Instance().GetString("services.named_pipes.prefix",
                                                        "\\\\.\\pipe\\ElleAnn_");
        if ((int)svc < 0 || (int)svc >= (int)ELLE_SERVICE_COUNT) {

            return prefix + std::string("Unknown_") + std::to_string((int)svc);
        }
        return prefix + g_serviceNames[svc];
    }

    const char* GetServiceName(ELLE_SERVICE_ID svc) {
        if ((int)svc >= 0 && (int)svc < (int)ELLE_SERVICE_COUNT) return g_serviceNames[svc];
        return "Unknown";
    }
}

ElleIPCMessage ElleIPCMessage::Create(ELLE_IPC_MSG_TYPE type,
                                       ELLE_SERVICE_ID src, ELLE_SERVICE_ID dst,
                                       const void* data, uint32_t dataSize) {
    ElleIPCMessage msg;
    msg.header.magic = ELLE_IPC_MAGIC;
    msg.header.version = ELLE_VERSION_MAJOR;
    msg.header.msg_type = (uint32_t)type;
    msg.header.payload_size = dataSize;
    msg.header.timestamp_ms = ELLE_MS_NOW();
    msg.header.source_svc = (uint32_t)src;
    msg.header.dest_svc = (uint32_t)dst;
    msg.header.correlation_id = Elle_HighResTimestamp();
    msg.header.flags = 0;

    msg.header.checksum = 0;

    if (data && dataSize > 0) {
        msg.payload.resize(dataSize);
        memcpy(msg.payload.data(), data, dataSize);
    }

    msg.header.checksum = ElleIPCMessage::ComputeChecksum(msg.header,
                                                          (const uint8_t*)data,
                                                          dataSize);

    return msg;
}

std::vector<uint8_t> ElleIPCMessage::Serialize() const {
    std::vector<uint8_t> buffer(sizeof(ELLE_IPC_HEADER) + payload.size());

    ELLE_IPC_HEADER outHeader = header;
    outHeader.payload_size = (uint32_t)payload.size();
    outHeader.checksum = ElleIPCMessage::ComputeChecksum(outHeader,
                                                         payload.empty() ? nullptr : payload.data(),
                                                         (uint32_t)payload.size());

    memcpy(buffer.data(), &outHeader, sizeof(ELLE_IPC_HEADER));
    if (!payload.empty()) {
        memcpy(buffer.data() + sizeof(ELLE_IPC_HEADER), payload.data(), payload.size());
    }
    return buffer;
}

bool ElleIPCMessage::Deserialize(const uint8_t* data, uint32_t len, ElleIPCMessage& out) {
    if (len < sizeof(ELLE_IPC_HEADER)) return false;

    memcpy(&out.header, data, sizeof(ELLE_IPC_HEADER));

    if (out.header.magic != ELLE_IPC_MAGIC) return false;

    uint32_t payloadSize = out.header.payload_size;

    uint32_t maxPayload = (uint32_t)ElleConfig::Instance().GetInt(
        "services.named_pipes.max_payload_bytes", (int64_t)ELLE_IPC_MAX_PAYLOAD);
    if (maxPayload == 0) maxPayload = ELLE_IPC_MAX_PAYLOAD;
    if (payloadSize > maxPayload) {
        ELLE_WARN("IPC frame rejected: payload_size=%u > cap=%u (svc=%u→%u)",
                  payloadSize, maxPayload,
                  (unsigned)out.header.source_svc,
                  (unsigned)out.header.dest_svc);
        return false;
    }
    if (len < sizeof(ELLE_IPC_HEADER) + payloadSize) return false;

    if (payloadSize > 0) {
        out.payload.resize(payloadSize);
        memcpy(out.payload.data(), data + sizeof(ELLE_IPC_HEADER), payloadSize);
    }

    uint32_t claimed = out.header.checksum;
    uint32_t actual = ElleIPCMessage::ComputeChecksum(out.header,
                                                      out.payload.empty() ? nullptr : out.payload.data(),
                                                      payloadSize);
    if (claimed != actual) {

#ifdef _DEBUG
        ELLE_WARN("IPC checksum mismatch: claimed=%08x actual=%08x src=%u dst=%u type=%u ver=%u payload=%u head=%s",
                  claimed, actual,
                  (unsigned)out.header.source_svc,
                  (unsigned)out.header.dest_svc,
                  (unsigned)out.header.msg_type,
                  (unsigned)out.header.version,
                  (unsigned)payloadSize,
                  ElleHexPreview((const uint8_t*)&out.header, sizeof(ELLE_IPC_HEADER), 24).c_str());
#else
        ELLE_WARN("IPC checksum mismatch: claimed=%08x actual=%08x — frame dropped",
                  claimed, actual);
#endif
        return false;
    }

    return true;
}

void ElleIPCMessage::SetStringPayload(const std::string& s) {
    payload.assign(s.begin(), s.end());
    payload.push_back(0);
    header.payload_size = (uint32_t)payload.size();

    header.checksum = ComputeChecksum(header,
                                      payload.empty() ? nullptr : payload.data(),
                                      header.payload_size);
}

std::string ElleIPCMessage::GetStringPayload() const {
    if (payload.empty()) return "";
    return std::string(payload.begin(), payload.end() - (payload.back() == 0 ? 1 : 0));
}

EllePipeConnection::EllePipeConnection() {
    ZeroMemory(&m_readOvl, sizeof(m_readOvl));
    ZeroMemory(&m_writeOvl, sizeof(m_writeOvl));

    m_readBuffer.reserve(ELLE_PIPE_BUFFER_SIZE);

}

EllePipeConnection::~EllePipeConnection() {
    if (m_hPipe != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(m_hPipe);
        CloseHandle(m_hPipe);
    }
}

ElleIPCServer::ElleIPCServer() {}
ElleIPCServer::~ElleIPCServer() { Stop(); }

bool ElleIPCServer::Start(ELLE_SERVICE_ID myService, HANDLE hIOCP, uint32_t maxInstances) {
    m_serviceId = myService;
    m_hIOCP = hIOCP;
    m_maxInstances = maxInstances;
    m_pipeName = ElleIPC::GetPipeName(myService);
    m_running = true;

    for (uint32_t i = 0; i < maxInstances; i++) {
        auto conn = std::make_unique<EllePipeConnection>();
        if (CreatePipeInstance(conn.get())) {
            std::lock_guard<std::mutex> lock(m_connMutex);
            m_connections.push_back(std::move(conn));
        }
    }

    ELLE_INFO("IPC Server started on %s (%d instances)", m_pipeName.c_str(), maxInstances);
    return !m_connections.empty();
}

void ElleIPCServer::Stop() {
    m_running = false;
    std::lock_guard<std::mutex> lock(m_connMutex);
    m_connections.clear();
    ELLE_INFO("IPC Server stopped");
}

bool ElleIPCServer::CreatePipeInstance(EllePipeConnection* conn) {
    conn->m_hPipe = CreateNamedPipeA(
        m_pipeName.c_str(),
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        m_maxInstances,
        ELLE_PIPE_BUFFER_SIZE,
        ELLE_PIPE_BUFFER_SIZE,
        0,
        nullptr
    );

    if (conn->m_hPipe == INVALID_HANDLE_VALUE) {
        ELLE_ERROR("CreateNamedPipe failed: %d", GetLastError());
        return false;
    }

    if (!CreateIoCompletionPort(conn->m_hPipe, m_hIOCP, (ULONG_PTR)conn, 0)) {
        ELLE_ERROR("IOCP association failed: %d", GetLastError());
        CloseHandle(conn->m_hPipe);
        conn->m_hPipe = INVALID_HANDLE_VALUE;
        return false;
    }

    conn->m_readOvl.operation = ELLE_IOCP_OP_CONNECT;
    conn->m_readOvl.service_id = m_serviceId;
    conn->m_readOvl.pipe_handle = conn->m_hPipe;

    BOOL connected = ConnectNamedPipe(conn->m_hPipe, &conn->m_readOvl.overlapped);
    if (!connected) {
        DWORD err = GetLastError();
        if (err == ERROR_IO_PENDING) {
            return true;
        }
        if (err == ERROR_PIPE_CONNECTED) {
            conn->m_connected = true;
            IssueRead(conn);
            return true;
        }
        ELLE_ERROR("ConnectNamedPipe failed: %d", err);
        return false;
    }

    conn->m_connected = true;
    IssueRead(conn);
    return true;
}

void ElleIPCServer::IssueRead(EllePipeConnection* conn) {
    if (!conn->m_connected || !m_running) return;

    ZeroMemory(&conn->m_readOvl.overlapped, sizeof(OVERLAPPED));
    conn->m_readOvl.operation = ELLE_IOCP_OP_READ;
    conn->m_readOvl.service_id = m_serviceId;
    conn->m_readOvl.pipe_handle = conn->m_hPipe;

    BOOL result = ReadFile(
        conn->m_hPipe,
        conn->m_readOvl.buffer,
        ELLE_PIPE_BUFFER_SIZE,
        nullptr,
        &conn->m_readOvl.overlapped
    );

    if (!result && GetLastError() != ERROR_IO_PENDING) {
        ELLE_WARN("ReadFile failed on pipe: %d", GetLastError());
        conn->m_connected = false;
    }
}

void ElleIPCServer::OnIOComplete(ELLE_IOCP_OVERLAPPED* ovl, DWORD bytesTransferred, DWORD error) {

    EllePipeConnection* conn = reinterpret_cast<EllePipeConnection*>(
        reinterpret_cast<uint8_t*>(ovl) - offsetof(EllePipeConnection, m_readOvl));

    if (ovl->operation == ELLE_IOCP_OP_WRITE) return;

    if (error != 0) {
        if (error == ERROR_BROKEN_PIPE || error == ERROR_PIPE_NOT_CONNECTED) {
            conn->m_connected = false;
            conn->m_remoteService = (ELLE_SERVICE_ID)0;
            conn->m_readBuffer.clear();
            ELLE_DEBUG("Pipe disconnected — recycling instance for new client");

            DisconnectNamedPipe(conn->m_hPipe);
            ZeroMemory(&conn->m_readOvl.overlapped, sizeof(OVERLAPPED));
            conn->m_readOvl.operation = ELLE_IOCP_OP_CONNECT;

            BOOL ok = ConnectNamedPipe(conn->m_hPipe, &conn->m_readOvl.overlapped);
            if (!ok) {
                DWORD e = GetLastError();
                if (e == ERROR_PIPE_CONNECTED) {
                    conn->m_connected = true;
                    IssueRead(conn);
                } else if (e != ERROR_IO_PENDING) {
                    ELLE_WARN("Pipe recycle ConnectNamedPipe failed: %d", e);
                }
            }
        } else if (error == ERROR_MORE_DATA) {

            ProcessReadComplete(conn, bytesTransferred,  true);
            IssueRead(conn);
        } else {
            ELLE_WARN("Server IOCP error %u on pipe", error);
        }
        return;
    }

    switch (ovl->operation) {
        case ELLE_IOCP_OP_CONNECT:
            conn->m_connected = true;
            ELLE_DEBUG("Client connected to pipe");
            IssueRead(conn);
            break;

        case ELLE_IOCP_OP_READ:
            ProcessReadComplete(conn, bytesTransferred,  false);
            IssueRead(conn);
            break;

        default:
            break;
    }
}

void ElleIPCServer::ProcessReadComplete(EllePipeConnection* conn, DWORD bytes) {
    ProcessReadComplete(conn, bytes, false);
}

void ElleIPCServer::ProcessReadComplete(EllePipeConnection* conn, DWORD bytes, bool partial) {
    if (bytes == 0 && !partial) return;

    size_t off = conn->m_readBuffer.size();
    conn->m_readBuffer.resize(off + bytes);
    memcpy(conn->m_readBuffer.data() + off, conn->m_readOvl.buffer, bytes);

    if (partial) {

        uint32_t maxPayload = (uint32_t)ElleConfig::Instance().GetInt(
            "services.named_pipes.max_payload_bytes", (int64_t)ELLE_IPC_MAX_PAYLOAD);
        if (maxPayload == 0) maxPayload = ELLE_IPC_MAX_PAYLOAD;
        const size_t cap = sizeof(ELLE_IPC_HEADER) + (size_t)maxPayload;
        if (conn->m_readBuffer.size() > cap) {
            ELLE_ERROR("IPC reassembly overflow: accumulated=%zu cap=%zu (dropping)",
                       conn->m_readBuffer.size(), cap);
            conn->m_readBuffer.clear();
        }
        return;
    }

    auto tryResync = [&](const uint8_t* buf, size_t len) -> size_t {
        if (len < sizeof(ELLE_IPC_HEADER)) return len;
        const uint32_t magic = ELLE_IPC_MAGIC;

        for (size_t i = 1; i + sizeof(ELLE_IPC_HEADER) <= len; i++) {
            uint32_t m = 0;
            memcpy(&m, buf + i, sizeof(uint32_t));
            if (m == magic) return i;
        }
        return len;
    };

    ElleIPCMessage msg;
    if (ElleIPCMessage::Deserialize(
            conn->m_readBuffer.data(), (uint32_t)conn->m_readBuffer.size(), msg)) {
        conn->m_remoteService = (ELLE_SERVICE_ID)msg.header.source_svc;
        if (m_handler) {
            m_handler(msg, (ELLE_SERVICE_ID)msg.header.source_svc);
        }
        conn->m_readBuffer.clear();
        return;
    }

    ELLE_WARN("IPC deserialize failed: accumulated=%zu head=%s",
              conn->m_readBuffer.size(),
              ElleHexPreview(conn->m_readBuffer.data(), conn->m_readBuffer.size(), 32).c_str());
    size_t drop = tryResync(conn->m_readBuffer.data(), conn->m_readBuffer.size());
    if (drop < conn->m_readBuffer.size()) {
        ELLE_WARN("IPC desync: dropping %zu bytes to resync (%zu bytes retained)",
                  drop, conn->m_readBuffer.size() - drop);
        std::vector<uint8_t> retained(conn->m_readBuffer.begin() + drop,
                                      conn->m_readBuffer.end());
        conn->m_readBuffer.swap(retained);
    } else {
        ELLE_WARN("Failed to deserialize IPC message (%zu bytes accumulated) - clearing buffer",
                  conn->m_readBuffer.size());
        conn->m_readBuffer.clear();
    }
}

bool ElleIPCServer::Send(ELLE_SERVICE_ID target, const ElleIPCMessage& msg) {
    std::lock_guard<std::mutex> lock(m_connMutex);
    for (auto& conn : m_connections) {
        if (conn->m_connected && conn->m_remoteService == target) {

            std::lock_guard<std::mutex> wlock(conn->m_writeMutex);
            auto pw = std::make_unique<EllePipeConnection::PendingWrite>();
            pw->data = msg.Serialize();
            pw->ovl.operation = ELLE_IOCP_OP_WRITE;
            pw->ovl.service_id = m_serviceId;
            pw->ovl.pipe_handle = conn->m_hPipe;

            DWORD written = 0;
            BOOL result = WriteFile(conn->m_hPipe, pw->data.data(),
                                    (DWORD)pw->data.size(), &written,
                                    &pw->ovl.overlapped);
            if (result || GetLastError() == ERROR_IO_PENDING) {
                conn->m_pendingWrites.push_back(std::move(pw));
                return true;
            }
            return false;
        }
    }
    return false;
}

void ElleIPCServer::Broadcast(const ElleIPCMessage& msg) {
    std::lock_guard<std::mutex> lock(m_connMutex);
    for (auto& conn : m_connections) {
        if (!conn->m_connected) continue;
        std::lock_guard<std::mutex> wlock(conn->m_writeMutex);
        auto pw = std::make_unique<EllePipeConnection::PendingWrite>();
        pw->data = msg.Serialize();
        pw->ovl.operation = ELLE_IOCP_OP_WRITE;
        pw->ovl.service_id = m_serviceId;
        pw->ovl.pipe_handle = conn->m_hPipe;

        DWORD written = 0;
        BOOL result = WriteFile(conn->m_hPipe, pw->data.data(),
                                (DWORD)pw->data.size(), &written,
                                &pw->ovl.overlapped);
        if (result || GetLastError() == ERROR_IO_PENDING) {
            conn->m_pendingWrites.push_back(std::move(pw));
        }
    }
}

uint32_t ElleIPCServer::ConnectedClients() const {
    uint32_t count = 0;
    for (auto& conn : m_connections) {
        if (conn->m_connected) count++;
    }
    return count;
}

ElleIPCClient::ElleIPCClient() {}
ElleIPCClient::~ElleIPCClient() { Disconnect(); }

bool ElleIPCClient::Connect(ELLE_SERVICE_ID myService, ELLE_SERVICE_ID targetService,
                             HANDLE hIOCP, uint32_t timeoutMs) {
    m_myService = myService;
    m_targetService = targetService;
    m_hIOCP = hIOCP;

    std::string pipeName = ElleIPC::GetPipeName(targetService);

    if (!WaitNamedPipeA(pipeName.c_str(), timeoutMs)) {
        return false;
    }

    m_conn = std::make_unique<EllePipeConnection>();
    m_conn->m_hPipe = CreateFileA(
        pipeName.c_str(),
        GENERIC_READ | GENERIC_WRITE,
        0,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_OVERLAPPED,
        nullptr
    );

    if (m_conn->m_hPipe == INVALID_HANDLE_VALUE) {
        m_conn.reset();
        return false;
    }

    m_conn->m_clientOwner = this;

    {
        DWORD mode = PIPE_READMODE_MESSAGE;
        if (!SetNamedPipeHandleState(m_conn->m_hPipe, &mode, nullptr, nullptr)) {
            ELLE_WARN("SetNamedPipeHandleState(PIPE_READMODE_MESSAGE) failed (target=%u): %u",
                      (unsigned)targetService, (unsigned)GetLastError());
        }
    }

    if (!CreateIoCompletionPort(m_conn->m_hPipe, m_hIOCP,
                                (ULONG_PTR)m_conn.get(), 0)) {
        ELLE_ERROR("Client IOCP association failed (target=%u): %d",
                   (unsigned)targetService, GetLastError());
        CloseHandle(m_conn->m_hPipe);
        m_conn->m_hPipe = INVALID_HANDLE_VALUE;
        m_conn.reset();
        return false;
    }

    m_conn->m_connected = true;
    m_conn->m_remoteService = targetService;

    m_conn->m_readOvl.operation = ELLE_IOCP_OP_READ;
    m_conn->m_readOvl.pipe_handle = m_conn->m_hPipe;

    ReadFile(m_conn->m_hPipe, m_conn->m_readOvl.buffer, ELLE_PIPE_BUFFER_SIZE,
             nullptr, &m_conn->m_readOvl.overlapped);

    return true;
}

void ElleIPCClient::OnIOComplete(ELLE_IOCP_OVERLAPPED* ovl, DWORD bytesTransferred, DWORD error) {
    if (!m_conn) return;

    if (error != 0) {
        if (error == ERROR_BROKEN_PIPE || error == ERROR_PIPE_NOT_CONNECTED) {
            m_conn->m_connected = false;
            ELLE_DEBUG("Client pipe disconnected (target=%u)", (unsigned)m_targetService);
        } else if (error == ERROR_MORE_DATA) {

            size_t off = m_conn->m_readBuffer.size();
            m_conn->m_readBuffer.resize(off + bytesTransferred);
            memcpy(m_conn->m_readBuffer.data() + off,
                   m_conn->m_readOvl.buffer, bytesTransferred);

            ZeroMemory(&m_conn->m_readOvl.overlapped, sizeof(OVERLAPPED));
            m_conn->m_readOvl.operation = ELLE_IOCP_OP_READ;
            ReadFile(m_conn->m_hPipe, m_conn->m_readOvl.buffer,
                     ELLE_PIPE_BUFFER_SIZE, nullptr,
                     &m_conn->m_readOvl.overlapped);
            return;
        } else {
            ELLE_WARN("Client IOCP error %u on target=%u", error, (unsigned)m_targetService);
        }
        return;
    }

    switch (ovl->operation) {
        case ELLE_IOCP_OP_READ: {
            if (bytesTransferred > 0) {

                size_t off = m_conn->m_readBuffer.size();
                m_conn->m_readBuffer.resize(off + bytesTransferred);
                memcpy(m_conn->m_readBuffer.data() + off,
                       m_conn->m_readOvl.buffer, bytesTransferred);

                ElleIPCMessage msg;
                if (ElleIPCMessage::Deserialize(
                        m_conn->m_readBuffer.data(),
                        (uint32_t)m_conn->m_readBuffer.size(), msg)) {
                    if (m_handler) {
                        m_handler(msg, (ELLE_SERVICE_ID)msg.header.source_svc);
                    }
                } else {
                    ELLE_WARN("Client failed to deserialize %zu bytes from target=%u",
                              m_conn->m_readBuffer.size(), (unsigned)m_targetService);
                }
                m_conn->m_readBuffer.clear();
            }

            ZeroMemory(&m_conn->m_readOvl.overlapped, sizeof(OVERLAPPED));
            m_conn->m_readOvl.operation = ELLE_IOCP_OP_READ;
            m_conn->m_readOvl.pipe_handle = m_conn->m_hPipe;
            BOOL ok = ReadFile(m_conn->m_hPipe, m_conn->m_readOvl.buffer,
                               ELLE_PIPE_BUFFER_SIZE, nullptr,
                               &m_conn->m_readOvl.overlapped);
            if (!ok && GetLastError() != ERROR_IO_PENDING) {
                m_conn->m_connected = false;
                ELLE_WARN("Client re-post ReadFile failed: %d", GetLastError());
            }
            break;
        }
        case ELLE_IOCP_OP_WRITE:

            break;
        default:
            break;
    }
}

void ElleIPCClient::Disconnect() {
    if (m_conn) {
        m_conn->m_connected = false;
        if (m_conn->m_hPipe != INVALID_HANDLE_VALUE) {
            CloseHandle(m_conn->m_hPipe);
            m_conn->m_hPipe = INVALID_HANDLE_VALUE;
        }
        m_conn.reset();
    }
}

bool ElleIPCClient::IsConnected() const {
    return m_conn && m_conn->m_connected;
}

bool ElleIPCClient::Send(const ElleIPCMessage& msg) {
    if (!IsConnected()) return false;
    std::lock_guard<std::mutex> lock(m_conn->m_writeMutex);

    auto pw = std::make_unique<EllePipeConnection::PendingWrite>();
    pw->data = msg.Serialize();
    pw->ovl.operation = ELLE_IOCP_OP_WRITE;
    pw->ovl.service_id = m_myService;
    pw->ovl.pipe_handle = m_conn->m_hPipe;

    DWORD written = 0;
    BOOL result = WriteFile(m_conn->m_hPipe, pw->data.data(),
                            (DWORD)pw->data.size(), &written,
                            &pw->ovl.overlapped);
    if (result || GetLastError() == ERROR_IO_PENDING) {
        m_conn->m_pendingWrites.push_back(std::move(pw));
        return true;
    }
    return false;
}

ElleIPCHub::ElleIPCHub() {}
ElleIPCHub::~ElleIPCHub() { Shutdown(); }

bool ElleIPCHub::Initialize(ELLE_SERVICE_ID myService, uint32_t workerThreads) {
    m_serviceId = myService;

    m_hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, workerThreads);
    if (!m_hIOCP) {
        ELLE_ERROR("Failed to create IOCP: %d", GetLastError());
        return false;
    }

    auto maxInstances = (uint32_t)ElleConfig::Instance().GetInt(
        "services.named_pipes.max_instances", 16);
    if (!m_server.Start(myService, m_hIOCP, maxInstances)) {
        CloseHandle(m_hIOCP);
        m_hIOCP = nullptr;
        return false;
    }

    m_server.SetMessageHandler([this](const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) {
        DispatchMessage(msg, sender);
    });

    m_running = true;
    for (uint32_t i = 0; i < workerThreads; i++) {
        m_workers.emplace_back(&ElleIPCHub::WorkerThread, this);
    }

    m_initialized = true;
    return true;
}

void ElleIPCHub::Shutdown() {
    m_running = false;

    {
        std::lock_guard<std::mutex> lock(m_server.m_connMutex);
        for (auto& conn : m_server.m_connections) {
            if (conn && conn->m_hPipe != INVALID_HANDLE_VALUE) {
                CancelIoEx(conn->m_hPipe, nullptr);
            }
        }
    }
    {
        std::lock_guard<std::mutex> lock(m_clientMutex);
        for (auto& kv : m_clients) {
            auto* c = kv.second.get();
            if (c && c->m_conn && c->m_conn->m_hPipe != INVALID_HANDLE_VALUE) {
                CancelIoEx(c->m_conn->m_hPipe, nullptr);
            }
        }
    }

    for (size_t i = 0; i < m_workers.size(); i++) {
        PostQueuedCompletionStatus(m_hIOCP, 0, 0, nullptr);
    }

    for (auto& t : m_workers) {
        if (t.joinable()) t.join();
    }
    m_workers.clear();

    m_server.Stop();
    {
        std::lock_guard<std::mutex> lock(m_clientMutex);
        m_clients.clear();
    }

    if (m_hIOCP) {
        CloseHandle(m_hIOCP);
        m_hIOCP = nullptr;
    }

    m_initialized = false;
}

bool ElleIPCHub::ConnectTo(ELLE_SERVICE_ID target, uint32_t timeoutMs) {
    std::lock_guard<std::mutex> lock(m_clientMutex);

    auto it = m_clients.find(target);
    if (it != m_clients.end() && it->second->IsConnected()) return true;

    auto client = std::make_unique<ElleIPCClient>();
    if (!client->Connect(m_serviceId, target, m_hIOCP, timeoutMs)) {
        return false;
    }

    client->SetMessageHandler([this](const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) {
        DispatchMessage(msg, sender);
    });

    m_clients[target] = std::move(client);
    return true;
}

bool ElleIPCHub::IsConnectedTo(ELLE_SERVICE_ID target) const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_clientMutex));
    auto it = m_clients.find(target);
    return it != m_clients.end() && it->second->IsConnected();
}

bool ElleIPCHub::Send(ELLE_SERVICE_ID target, const ElleIPCMessage& msg) {
    m_sent++;

    {
        std::lock_guard<std::mutex> lock(m_clientMutex);
        auto it = m_clients.find(target);
        if (it != m_clients.end() && it->second->IsConnected()) {
            const bool ok = it->second->Send(msg);
            if (ok) StampLastSeen(target);
            return ok;
        }
    }

    if (m_server.Send(target, msg)) { StampLastSeen(target); return true; }

    if (ConnectTo(target, 1500)) {
        std::lock_guard<std::mutex> lock(m_clientMutex);
        auto it = m_clients.find(target);
        if (it != m_clients.end() && it->second->IsConnected()) {
            const bool ok = it->second->Send(msg);
            if (ok) StampLastSeen(target);
            return ok;
        }
    }
    return false;
}

void ElleIPCHub::Broadcast(const ElleIPCMessage& msg) {
    m_server.Broadcast(msg);
    std::lock_guard<std::mutex> lock(m_clientMutex);
    for (auto& [id, client] : m_clients) {
        if (client->IsConnected()) client->Send(msg);
    }
}

void ElleIPCHub::SetMessageHandler(IPCMessageHandler handler) {
    m_handler = handler;
}

bool ElleIPCHub::HasPendingMessage() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_queueMutex));
    return !m_incomingQueue.empty();
}

bool ElleIPCHub::PopMessage(ElleIPCMessage& out, uint32_t timeoutMs) {
    std::unique_lock<std::mutex> lock(m_queueMutex);
    if (timeoutMs == 0) {
        if (m_incomingQueue.empty()) return false;
    } else {
        if (!m_queueCV.wait_for(lock, std::chrono::milliseconds(timeoutMs),
                                [this] { return !m_incomingQueue.empty(); })) {
            return false;
        }
    }

    out = m_incomingQueue.front();
    m_incomingQueue.pop();
    return true;
}

void ElleIPCHub::DispatchMessage(const ElleIPCMessage& msg, ELLE_SERVICE_ID sender) {
    m_received++;

    StampLastSeen(sender);

    if (m_handler) {
        m_handler(msg, sender);
    }

    std::lock_guard<std::mutex> lock(m_queueMutex);
    m_incomingQueue.push(msg);
    m_queueCV.notify_one();
}

void ElleIPCHub::WorkerThread() {
    while (m_running) {
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        LPOVERLAPPED ovl = nullptr;

        BOOL result = GetQueuedCompletionStatus(
            m_hIOCP, &bytesTransferred, &completionKey, &ovl, 1000);

        if (!m_running) break;

        if (!result) {
            DWORD err = GetLastError();
            if (err == WAIT_TIMEOUT) continue;
            if (ovl) {
                auto* elleOvl = reinterpret_cast<ELLE_IOCP_OVERLAPPED*>(ovl);
                auto* conn    = reinterpret_cast<EllePipeConnection*>(completionKey);

                DispatchIOComplete(elleOvl, bytesTransferred, err, conn);
            }
            continue;
        }

        if (!ovl) continue;

        auto* elleOvl = reinterpret_cast<ELLE_IOCP_OVERLAPPED*>(ovl);
        auto* conn    = reinterpret_cast<EllePipeConnection*>(completionKey);
        DispatchIOComplete(elleOvl, bytesTransferred, 0, conn);
    }
}

void ElleIPCHub::DispatchIOComplete(ELLE_IOCP_OVERLAPPED* ovl, DWORD bytes, DWORD err,
                                    EllePipeConnection* conn) {
    if (!conn) {

        return;
    }

    auto* ovlBase = reinterpret_cast<uint8_t*>(ovl);
    auto* connBase = reinterpret_cast<uint8_t*>(conn);
    bool isWrite = (ovlBase == connBase + EllePipeConnection::WriteOvlOffset())
                || (ovl->operation == ELLE_IOCP_OP_WRITE);

    if (!isWrite) {
        std::lock_guard<std::mutex> lock(conn->m_writeMutex);
        for (auto& pw : conn->m_pendingWrites) {
            if (&pw->ovl == ovl) { isWrite = true; break; }
        }
    }

    if (isWrite) {

        std::lock_guard<std::mutex> lock(conn->m_writeMutex);
        auto& pw = conn->m_pendingWrites;
        pw.erase(std::remove_if(pw.begin(), pw.end(),
                 [ovl](const std::unique_ptr<EllePipeConnection::PendingWrite>& p) {
                     return &p->ovl == ovl;
                 }), pw.end());
        return;
    }

    if (conn->m_clientOwner) {
        reinterpret_cast<ElleIPCClient*>(conn->m_clientOwner)->OnIOComplete(ovl, bytes, err);
    } else {
        m_server.OnIOComplete(ovl, bytes, err);
    }
}
