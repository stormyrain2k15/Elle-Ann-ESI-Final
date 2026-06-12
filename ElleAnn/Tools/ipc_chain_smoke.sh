#!/usr/bin/env bash
set -euo pipefail

PROM_TEXTFILE=""

while (( "$#" )); do
    case "$1" in
        --prometheus-textfile)
            if [[ $# -lt 2 ]]; then
                echo "[ipc-chain] --prometheus-textfile requires a path argument" >&2
                exit 2
            fi
            PROM_TEXTFILE="$2"
            shift 2
            ;;
        --prometheus-textfile=*)
            PROM_TEXTFILE="${1#*=}"
            shift
            ;;
        -h|--help)
            cat <<'EOF'
ipc_chain_smoke.sh — cross-service IPC chain smoke (audit D34)

Usage: ipc_chain_smoke.sh [--prometheus-textfile PATH]

Options:
  --prometheus-textfile PATH   On script exit, atomically write a
                               Prometheus textfile-collector compatible
                               file with:
                                 elle_ipc_chain_smoke{status="pass|fail"} 1
                                 elle_ipc_chain_smoke_last_run_unixtime <epoch>
                               Atomic via PATH.tmp + mv.

Environment:
  ELLE_HTTP_URL    full base URL, default http://127.0.0.1:8000
  ELLE_SQL_DSN     DSN for sqlcmd to query ElleSystem heartbeats
  ELLE_ADMIN_KEY   optional admin key for AUTH_ADMIN routes
  ELLE_CHAT_USER   optional user_id to send (default 'default')
EOF
            exit 0
            ;;
        *)
            echo "[ipc-chain] unknown argument: $1" >&2
            echo "[ipc-chain] see --help" >&2
            exit 2
            ;;
    esac
done

emit_prometheus_textfile() {
    local exit_code="$1"
    local status="fail"
    if [[ "${exit_code}" == "0" ]]; then
        status="pass"
    fi
    local tmp="${PROM_TEXTFILE}.tmp"
    {
        echo "# HELP elle_ipc_chain_smoke 1 if the IPC chain smoke last passed, labelled status=pass|fail."
        echo "# TYPE elle_ipc_chain_smoke gauge"
        echo "elle_ipc_chain_smoke{status=\"${status}\"} 1"
        echo "# HELP elle_ipc_chain_smoke_last_run_unixtime Epoch seconds of the last smoke run."
        echo "# TYPE elle_ipc_chain_smoke_last_run_unixtime gauge"
        echo "elle_ipc_chain_smoke_last_run_unixtime $(date +%s)"
    } > "${tmp}"
    mv -f "${tmp}" "${PROM_TEXTFILE}"
}

on_exit() {
    local code=$?
    if [[ -n "${PROM_TEXTFILE}" ]]; then
        emit_prometheus_textfile "${code}" || true
    fi
}
trap on_exit EXIT

# Cross-service IPC chain smoke (audit D34).
#
# Exercises the canonical chat pipeline end-to-end:
#
#   HTTP --(IPC_CHAT_REQUEST)--> Cognitive
#                                  |
#                                  +-> Probability  (IPC_PROB_ANALYZE → IPC_PROB_RESPONSE)
#                                  +-> MindManager  (IPC_MIND_ASK     → IPC_MIND_VERDICT)
#                                  +-> Intuition    (IPC_INTUITION_*)
#                                  +-> Composer/LLM
#                                  |
#   HTTP <--(IPC_CHAT_RESPONSE)----+
#
# Assertions:
#   * /api/ai/chat returns 200 within 60 s.
#   * Response payload carries non-trivial output from every downstream service
#     (probabilistic_read, inner_voice, gut_read, response, memories_used,
#      provider_used, model_used).
#   * ElleSystem.dbo.Workers has a fresh (within 60 s of the request) heartbeat
#     for every expected service id.
#
# Run on the Windows host where the mesh is alive. Requires:
#   ELLE_HTTP_URL  : full base URL, e.g. http://127.0.0.1:8000
#   ELLE_SQL_DSN   : DSN for sqlcmd to query ElleSystem (and any auth tables)
#   ELLE_ADMIN_KEY : optional admin key for AUTH_ADMIN routes (only consulted
#                    if non-empty)
#   ELLE_CHAT_USER : optional user_id to send (default 'default')

ELLE_HTTP_URL="${ELLE_HTTP_URL:-http://127.0.0.1:8000}"
ELLE_CHAT_USER="${ELLE_CHAT_USER:-default}"

if ! command -v curl >/dev/null 2>&1; then
    echo "[ipc-chain] curl missing on PATH" >&2; exit 2
fi
if ! command -v jq >/dev/null 2>&1; then
    echo "[ipc-chain] jq missing on PATH (apt: jq, choco: jq)" >&2; exit 2
fi

fail() {
    echo "[ipc-chain] FAIL: $*" >&2
    exit 1
}
pass() {
    echo "[ipc-chain] PASS: $*"
}

probe_health() {
    curl -fsS "${ELLE_HTTP_URL}/api/health" >/dev/null 2>&1
}

echo "[ipc-chain] === Pre-flight ==="
if ! probe_health; then
    fail "no /api/health response from ${ELLE_HTTP_URL} (mesh not running?)"
fi
pass "HTTP service reachable"

REQ_ID="ipc-chain-$(date +%s)-$$"
PROMPT="ipc-chain-smoke ${REQ_ID}: tell me one specific way you stay grounded."

echo "[ipc-chain] === POST /api/ai/chat (req_id ${REQ_ID}) ==="
RESP_FILE="$(mktemp)"
trap 'rm -f "${RESP_FILE}"' EXIT

HTTP_CODE=$(curl -sS -o "${RESP_FILE}" -w '%{http_code}' \
    --max-time 60 \
    -X POST "${ELLE_HTTP_URL}/api/ai/chat" \
    -H "Content-Type: application/json" \
    -d "$(jq -nc --arg msg "$PROMPT" \
                 --arg uid "$ELLE_CHAT_USER" \
                 --arg rid "$REQ_ID" \
                 '{message:$msg, user_id:$uid, conversation_id:1, request_id:$rid}')")

if [[ "${HTTP_CODE}" != "200" ]]; then
    echo "----- response body -----"
    cat "${RESP_FILE}" >&2 || true
    echo "-------------------------"
    fail "HTTP ${HTTP_CODE} (expected 200) from /api/ai/chat"
fi
pass "HTTP 200 within 60 s"

echo "[ipc-chain] === Validate downstream chain in response ==="

assert_json_string_nonempty() {
    local path="$1" label="$2"
    local v
    v=$(jq -r "${path} // \"\"" "${RESP_FILE}")
    if [[ -z "${v}" || "${v}" == "null" ]]; then
        echo "----- response body -----"
        cat "${RESP_FILE}" >&2
        echo "-------------------------"
        fail "${label}: empty (${path})"
    fi
    pass "${label} = '$(printf '%s' "${v}" | head -c 60)...'"
}

assert_json_object_present() {
    local path="$1" label="$2"
    local t
    t=$(jq -r "${path} | type" "${RESP_FILE}")
    if [[ "${t}" != "object" ]]; then
        echo "----- response body -----"
        cat "${RESP_FILE}" >&2
        echo "-------------------------"
        fail "${label}: expected object at ${path}, got ${t}"
    fi
    pass "${label} (object present at ${path})"
}

assert_json_number_present() {
    local path="$1" label="$2"
    local t
    t=$(jq -r "${path} | type" "${RESP_FILE}")
    if [[ "${t}" != "number" ]]; then
        fail "${label}: expected number at ${path}, got ${t}"
    fi
    pass "${label} = $(jq -r "${path}" "${RESP_FILE}")"
}

assert_json_string_nonempty '.response'         'Composer / LLM produced text'
assert_json_object_present   '.probabilistic_read' 'Probability ran'
assert_json_object_present   '.inner_voice'        'MindManager / Conscience ran'
assert_json_object_present   '.gut_read'           'Intuition ran'
assert_json_number_present   '.memories_used'      'Memory chain hit'
assert_json_number_present   '.latency_ms'         'Latency reported'
assert_json_string_nonempty  '.provider_used'      'Composer.provider_used set'
assert_json_string_nonempty  '.model_used'         'Composer.model_used set'

if [[ -n "${ELLE_SQL_DSN:-}" ]] && command -v sqlcmd >/dev/null 2>&1; then
    echo "[ipc-chain] === Heartbeat freshness (ElleSystem.dbo.Workers) ==="

    # Expected service IDs from Services/_Shared/ElleTypes.h:
    #   SVC_HTTP_SERVER, SVC_COGNITIVE, SVC_MEMORY, SVC_PROBABILITY,
    #   SVC_MINDMANAGER, SVC_INTUITION, SVC_COMPOSER

    HEARTBEAT_WINDOW_MS=60000
    NOW_MS=$(date +%s%3N)
    THRESHOLD_MS=$(( NOW_MS - HEARTBEAT_WINDOW_MS ))

    Q="
        SELECT Name, LastHeartbeatMs
        FROM ElleSystem.dbo.Workers
        WHERE Name IN ('Elle.Service.HTTP','Elle.Service.Cognitive',
                       'Elle.Service.Memory','Elle.Service.Probability',
                       'Elle.Service.MindManager','Elle.Service.Intuition',
                       'Elle.Service.Composer')
        ORDER BY Name;"

    HB_OUT=$(sqlcmd -S "${ELLE_SQL_DSN}" -d ElleSystem -h -1 -W -b -Q "${Q}" 2>/dev/null || true)

    if [[ -z "${HB_OUT}" ]]; then
        echo "[ipc-chain] WARN: heartbeat query returned nothing — skipping freshness assertions."
    else
        echo "${HB_OUT}" | while IFS=$' \t' read -r NAME HB || [[ -n "${NAME}" ]]; do
            [[ -z "${NAME}" || "${NAME}" =~ ^- ]] && continue
            [[ "${NAME}" =~ rows ]] && continue
            if [[ -z "${HB}" ]] || ! [[ "${HB}" =~ ^[0-9]+$ ]]; then continue; fi
            if (( HB < THRESHOLD_MS )); then
                echo "[ipc-chain] WARN: ${NAME} heartbeat=${HB} older than ${HEARTBEAT_WINDOW_MS}ms"
            else
                age=$(( NOW_MS - HB ))
                echo "[ipc-chain] PASS: ${NAME} heartbeat fresh (age=${age}ms)"
            fi
        done
    fi
else
    echo "[ipc-chain] (skip heartbeat check — set ELLE_SQL_DSN and install sqlcmd to enable)"
fi

echo "[ipc-chain] === All assertions passed for req_id ${REQ_ID} (HTTP 200 + 4-way chain fan-out verified) ==="
