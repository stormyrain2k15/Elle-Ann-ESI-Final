#pragma once
#ifndef ELLE_FIESTA_CONNECTION_H
#define ELLE_FIESTA_CONNECTION_H

#include "FiestaCipher.h"
#include "FiestaConsoleTrace.h"
#include "FiestaPacket.h"

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

namespace Fiesta {

struct InPacket {
    uint16_t              opcode = 0;
    std::vector<uint8_t>  payload;
};

class Connection {
public:
    using PacketCallback     = std::function<void(const InPacket&)>;
    using DisconnectCallback = std::function<void(const std::string& why)>;

    Connection() {
        WSADATA d;
        WSAStartup(MAKEWORD(2, 2), &d);
    }

    ~Connection() {
        Disconnect("destructor");
        WSACleanup();
    }

    bool Connect(const std::string& host, uint16_t port) {
        Disconnect("re-connect");

        m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (m_sock == INVALID_SOCKET) return false;

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(port);
        if (inet_pton(AF_INET, host.c_str(), &addr.sin_addr) != 1) {

            addrinfo hints{}; addrinfo* res = nullptr;
            hints.ai_family = AF_INET; hints.ai_socktype = SOCK_STREAM;
            const std::string portStr = std::to_string(port);
            if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res) != 0) {
                closesocket(m_sock);
                m_sock = INVALID_SOCKET;
                return false;
            }
            std::memcpy(&addr, res->ai_addr, sizeof(sockaddr_in));
            freeaddrinfo(res);
        }
        if (connect(m_sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
            return false;
        }

        m_running = true;
        m_recvThread = std::thread([this] { RecvLoop(); });
        return true;
    }

    void Disconnect(const std::string& why = "client disconnect") {
        bool wasRunning = m_running.exchange(false);
        if (m_sock != INVALID_SOCKET) {
            shutdown(m_sock, SD_BOTH);
            closesocket(m_sock);
            m_sock = INVALID_SOCKET;
        }
        if (m_recvThread.joinable() &&
            m_recvThread.get_id() != std::this_thread::get_id()) {
            m_recvThread.join();
        }
        if (wasRunning && m_onDisconnect) m_onDisconnect(why);
    }

    bool Send(uint16_t opcode, const std::vector<uint8_t>& payload) {
        std::lock_guard<std::mutex> lk(m_sendMx);
        if (m_sock == INVALID_SOCKET) return false;

        Fiesta::Trace::OnTx(opcode, payload);

        std::vector<uint8_t> wire = BuildPacket(opcode, payload);

        const size_t headerLen = (wire[0] == 0xFF) ? 3 : 1;
        m_cipher.EncryptOut(wire.data() + headerLen, wire.size() - headerLen);

        const char* p   = (const char*)wire.data();
        size_t      rem = wire.size();
        while (rem > 0) {
            int n = send(m_sock, p, (int)rem, 0);
            if (n <= 0) {
                Disconnect("send failed");
                return false;
            }
            p   += n;
            rem -= (size_t)n;
        }
        m_pktOut.fetch_add(1,            std::memory_order_relaxed);
        m_bytesOut.fetch_add(wire.size(), std::memory_order_relaxed);
        return true;
    }

    Cipher& MutableCipher() { return m_cipher; }
    const Cipher& GetCipher() const { return m_cipher; }

    uint64_t PacketsIn()  const { return m_pktIn.load(std::memory_order_relaxed); }
    uint64_t PacketsOut() const { return m_pktOut.load(std::memory_order_relaxed); }
    uint64_t BytesIn()    const { return m_bytesIn.load(std::memory_order_relaxed); }
    uint64_t BytesOut()   const { return m_bytesOut.load(std::memory_order_relaxed); }

    void SetOnPacket(PacketCallback cb)         { m_onPacket = std::move(cb); }
    void SetOnDisconnect(DisconnectCallback cb) { m_onDisconnect = std::move(cb); }

    bool IsConnected() const { return m_sock != INVALID_SOCKET && m_running.load(); }

private:
    void RecvLoop() {
        std::vector<uint8_t> buf;
        buf.reserve(4096);
        uint8_t chunk[2048];

        while (m_running) {
            int n = recv(m_sock, (char*)chunk, sizeof(chunk), 0);
            if (n <= 0) {
                if (m_running) Disconnect("recv eof / error");
                return;
            }
            buf.insert(buf.end(), chunk, chunk + n);

            while (true) {
                if (buf.empty()) break;

                size_t headerLen = 1;
                size_t bodyLen   = buf[0];
                if (bodyLen == 0xFF) {
                    if (buf.size() < 3) break;
                    bodyLen = (size_t)buf[1] | ((size_t)buf[2] << 8);
                    headerLen = 3;
                }
                if (buf.size() < headerLen + bodyLen) break;
                if (bodyLen < 2) {

                    buf.erase(buf.begin(), buf.begin() + 1);
                    continue;
                }

                std::vector<uint8_t> body(buf.begin() + headerLen,
                                          buf.begin() + headerLen + bodyLen);
                m_cipher.DecryptIn(body);

                InPacket pkt;

                pkt.opcode  = (uint16_t)((body[0] << 8) | body[1]);
                pkt.payload.assign(body.begin() + 2, body.end());

                if (m_onPacket) m_onPacket(pkt);

                m_pktIn.fetch_add(1,    std::memory_order_relaxed);
                m_bytesIn.fetch_add(bodyLen + headerLen,
                                    std::memory_order_relaxed);

                buf.erase(buf.begin(), buf.begin() + headerLen + bodyLen);
            }
        }
    }

    SOCKET                  m_sock = INVALID_SOCKET;
    std::atomic<bool>       m_running{false};
    std::atomic<uint64_t>   m_pktIn{0};
    std::atomic<uint64_t>   m_pktOut{0};
    std::atomic<uint64_t>   m_bytesIn{0};
    std::atomic<uint64_t>   m_bytesOut{0};
    std::thread             m_recvThread;
    std::mutex              m_sendMx;
    Cipher                  m_cipher;
    PacketCallback          m_onPacket;
    DisconnectCallback      m_onDisconnect;
};

}
#endif
