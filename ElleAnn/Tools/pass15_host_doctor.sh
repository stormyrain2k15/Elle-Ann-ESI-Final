#!/usr/bin/env bash
# pass15_host_doctor.sh — runs all three pending Pass-15 host verifications
# end-to-end and emits one consolidated pass/fail summary.
#
# Required on PATH:
#   sqlcmd  (for SQL apply)        — install: Microsoft ODBC + mssql-tools18
#   bash    (this script + ipc_chain_smoke.sh)
#   curl    (for ipc_chain_smoke.sh)
#   jq      (for ipc_chain_smoke.sh)
#   gradle wrapper at Tools/Android/gradlew
#
# Env:
#   ELLE_SQL_DSN     (required)  — sqlcmd -S server,port[\instance]
#   ELLE_HTTP_URL    (default http://127.0.0.1:8000)
#   ELLE_CHAT_USER   (default 'default')
#   ELLE_SKIP_ANDROID=1 to skip the gradle lint step (no Android SDK present)
#   ELLE_SKIP_SQL=1 to skip the SQL apply step (already applied)
#   ELLE_SKIP_IPC=1 to skip the IPC chain smoke (mesh not running)

set -uo pipefail

REPO_ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." &>/dev/null && pwd)"
cd "$REPO_ROOT" || { echo "cannot cd to $REPO_ROOT" >&2; exit 2; }

SQL_FILE="SQL/_Shared/01_sql_fallback_poison.sql"
SMOKE_SCRIPT="Tools/ipc_chain_smoke.sh"
ANDROID_DIR="Tools/Android"

PASS=()
FAIL=()
SKIP=()

step_pass() { PASS+=("$1"); echo "  [PASS] $1"; }
step_fail() { FAIL+=("$1"); echo "  [FAIL] $1"; }
step_skip() { SKIP+=("$1"); echo "  [SKIP] $1"; }

banner() {
    echo
    echo "================================================================"
    echo "  $1"
    echo "================================================================"
}

banner "1. Apply SQL fallback poison schema to ElleCore"
if [[ "${ELLE_SKIP_SQL:-0}" == "1" ]]; then
    step_skip "SQL apply (ELLE_SKIP_SQL=1)"
elif [[ -z "${ELLE_SQL_DSN:-}" ]]; then
    step_fail "ELLE_SQL_DSN env var not set — cannot apply SQL"
elif ! command -v sqlcmd >/dev/null 2>&1; then
    step_fail "sqlcmd not on PATH — install Microsoft ODBC + mssql-tools18"
elif [[ ! -f "$SQL_FILE" ]]; then
    step_fail "missing $SQL_FILE"
else
    if sqlcmd -S "$ELLE_SQL_DSN" -d ElleCore -b -i "$SQL_FILE" >/tmp/p15_sql.log 2>&1; then
        VERIFY=$(sqlcmd -S "$ELLE_SQL_DSN" -d ElleCore -h -1 -W -Q \
            "SELECT COUNT(*) FROM sys.procedures WHERE name LIKE 'usp_SQLFallbackPoison%'" 2>/dev/null \
            | head -1 | tr -d ' \r\n')
        if [[ "$VERIFY" == "3" ]]; then
            step_pass "SQL apply (3/3 procs created)"
        else
            step_fail "SQL apply: expected 3 procs, got $VERIFY (see /tmp/p15_sql.log)"
        fi
    else
        step_fail "sqlcmd exit non-zero (see /tmp/p15_sql.log)"
    fi
fi

banner "2. Cross-service IPC chain smoke (/api/ai/chat)"
if [[ "${ELLE_SKIP_IPC:-0}" == "1" ]]; then
    step_skip "IPC chain smoke (ELLE_SKIP_IPC=1)"
elif [[ ! -x "$SMOKE_SCRIPT" ]]; then
    step_fail "missing or non-executable $SMOKE_SCRIPT"
else
    if bash "$SMOKE_SCRIPT" >/tmp/p15_ipc.log 2>&1; then
        step_pass "IPC chain smoke (all 9 chain fan-out assertions green)"
    else
        step_fail "IPC chain smoke failed (see /tmp/p15_ipc.log)"
        tail -20 /tmp/p15_ipc.log | sed 's/^/      | /' || true
    fi
fi

banner "3. Android lintDebug (Tools/Android)"
if [[ "${ELLE_SKIP_ANDROID:-0}" == "1" ]]; then
    step_skip "Android lint (ELLE_SKIP_ANDROID=1 — no SDK present)"
elif [[ ! -x "$ANDROID_DIR/gradlew" ]]; then
    step_fail "$ANDROID_DIR/gradlew missing or non-executable"
elif [[ -z "${ANDROID_HOME:-${ANDROID_SDK_ROOT:-}}" ]]; then
    step_fail "ANDROID_HOME / ANDROID_SDK_ROOT not set — install Android SDK"
else
    if (cd "$ANDROID_DIR" && ./gradlew lintDebug --no-daemon --quiet) >/tmp/p15_lint.log 2>&1; then
        step_pass "Android lintDebug (BUILD SUCCESSFUL)"
    else
        step_fail "Android lintDebug failed (see /tmp/p15_lint.log)"
        tail -20 /tmp/p15_lint.log | sed 's/^/      | /' || true
    fi
fi

banner "Summary"
echo "  Passed:  ${#PASS[@]}"
echo "  Failed:  ${#FAIL[@]}"
echo "  Skipped: ${#SKIP[@]}"
echo
if (( ${#FAIL[@]} == 0 )); then
    if (( ${#SKIP[@]} > 0 )); then
        echo "  → All non-skipped checks passed."
    else
        echo "  → ALL THREE PASS-15 HOST VERIFICATIONS GREEN."
        echo "    Flip Anti-Slop matrix rows 8/15/D34 to ✅ in Docs/ANTI_SLOP_AUDIT_TRACKING.md."
    fi
    exit 0
fi
echo "  → ${#FAIL[@]} check(s) failed — see logs above and in /tmp/p15_*.log."
exit 1
