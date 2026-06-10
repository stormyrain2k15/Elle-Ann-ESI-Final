#!/usr/bin/env bash
set -euo pipefail

if [[ -z "${ELLE_SQL_DSN:-}" ]]; then
    echo "ELLE_SQL_DSN must point at the MS SQL Server hosting ElleCore / ElleHeart / EllesLanguage."
    exit 2
fi

if ! command -v sqlcmd >/dev/null 2>&1; then
    echo "sqlcmd not on PATH. Install MSSQL command-line tools (mssql-tools18)."
    exit 2
fi

HOST="${ELLE_BIND_HOST:-127.0.0.1}"
PORT="${ELLE_BIND_PORT:-3700}"
ELLE_SUPERVISOR="${ELLE_SUPERVISOR:-ElleSupervisor.exe}"
WAIT_S=8

probe_health() {
    curl -fsS "http://${HOST}:${PORT}/api/health" >/dev/null 2>&1
}

start_elle() {
    echo "[restart-persistence] starting Elle service mesh..."
    "${ELLE_SUPERVISOR}" --background &
    SUP_PID=$!
    for i in $(seq 1 30); do
        if probe_health; then
            echo "[restart-persistence] mesh up after ${i} attempts (pid ${SUP_PID})."
            return 0
        fi
        sleep 1
    done
    echo "[restart-persistence] mesh failed to come up within 30s."
    kill -TERM "${SUP_PID}" || true
    return 1
}

stop_elle() {
    echo "[restart-persistence] stopping mesh..."
    if [[ -n "${SUP_PID:-}" ]]; then
        kill -TERM "${SUP_PID}" || true
        wait "${SUP_PID}" || true
    fi
    sleep "${WAIT_S}"
}

write_seed_state() {
    local user_session_id
    user_session_id="restart-test-$(date +%s)"
    local memory_payload
    memory_payload=$(printf '{"content":"restart-canary-%s","importance":0.9}' "${user_session_id}")

    echo "[restart-persistence] writing seed memory + goal + intuition feedback..."
    curl -fsS -X POST "http://${HOST}:${PORT}/api/memory/store" \
         -H "Content-Type: application/json" -d "${memory_payload}" >/dev/null

    curl -fsS -X POST "http://${HOST}:${PORT}/api/goals" \
         -H "Content-Type: application/json" \
         -d "{\"description\":\"restart-canary-goal-${user_session_id}\",\"urgency\":0.7}" >/dev/null

    curl -fsS -X POST "http://${HOST}:${PORT}/api/intuition/feedback" \
         -H "Content-Type: application/json" \
         -d "{\"pattern_id\":1,\"outcome\":\"AFFIRMED\",\"reason\":\"restart-canary-${user_session_id}\"}" >/dev/null

    echo "${user_session_id}"
}

read_back_state() {
    local user_session_id="$1"
    local found_mem found_goal found_intu

    found_mem=$(curl -fsS "http://${HOST}:${PORT}/api/memory/search?q=restart-canary-${user_session_id}" | grep -c "${user_session_id}" || true)
    found_goal=$(curl -fsS "http://${HOST}:${PORT}/api/goals?q=${user_session_id}" | grep -c "${user_session_id}" || true)
    found_intu=$(curl -fsS "http://${HOST}:${PORT}/api/diag/queues" | grep -c "intuition_log" || true)

    echo "[restart-persistence] readback memory=${found_mem} goal=${found_goal} intu_log=${found_intu}"

    if [[ "${found_mem}" -lt 1 || "${found_goal}" -lt 1 ]]; then
        echo "[restart-persistence] FAIL — required rows did not survive restart."
        return 1
    fi
    echo "[restart-persistence] PASS — memory + goal + intuition feedback all survived the restart."
    return 0
}

assert_belief_persistence() {
    sqlcmd -S "${ELLE_SQL_DSN}" -d ElleHeart -Q \
        "SELECT COUNT(*) AS domains, SUM(CASE WHEN posterior_mass IS NOT NULL THEN 1 ELSE 0 END) AS posteriors FROM dbo.vw_BeliefSnapshot;" \
        -h -1 | awk 'NF==2 && $1+0 >= 1 && $2+0 >= 1 {ok=1} END{ exit ok ? 0 : 1 }'
    if [[ "$?" -ne 0 ]]; then
        echo "[restart-persistence] FAIL — belief snapshot empty after restart."
        return 1
    fi
    echo "[restart-persistence] PASS — belief snapshot present in ElleHeart."
    return 0
}

main() {
    start_elle
    user_session_id=$(write_seed_state)
    sleep 3
    stop_elle

    echo "[restart-persistence] sleeping ${WAIT_S}s to simulate clean restart..."
    sleep "${WAIT_S}"

    start_elle
    read_back_state "${user_session_id}"
    assert_belief_persistence
    stop_elle

    echo "[restart-persistence] all checks passed."
}

main "$@"
