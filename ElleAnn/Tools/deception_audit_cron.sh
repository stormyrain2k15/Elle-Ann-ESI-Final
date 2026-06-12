#!/usr/bin/env bash
set -euo pipefail

ELLE_HTTP_URL="${ELLE_HTTP_URL:-http://127.0.0.1:8000}"
ELLE_ADMIN_KEY="${ELLE_ADMIN_KEY:-}"
ELLE_AUDIT_OUT_DIR="${ELLE_AUDIT_OUT_DIR:-./deception_audit}"

while (( "$#" )); do
    case "$1" in
        --url)           ELLE_HTTP_URL="$2"; shift 2 ;;
        --url=*)         ELLE_HTTP_URL="${1#*=}"; shift ;;
        --admin-key)     ELLE_ADMIN_KEY="$2"; shift 2 ;;
        --admin-key=*)   ELLE_ADMIN_KEY="${1#*=}"; shift ;;
        --out-dir)       ELLE_AUDIT_OUT_DIR="$2"; shift 2 ;;
        --out-dir=*)     ELLE_AUDIT_OUT_DIR="${1#*=}"; shift ;;
        -h|--help)
            cat <<'EOF'
deception_audit_cron.sh — weekly snapshot of Elle deception signal/feedback rollups.

Usage:
  deception_audit_cron.sh [--url URL] [--admin-key KEY] [--out-dir DIR]

Options (or set via env):
  --url        Base URL of the Elle HTTP service        (env: ELLE_HTTP_URL,         default http://127.0.0.1:8000)
  --admin-key  AUTH_ADMIN token for /api/admin routes   (env: ELLE_ADMIN_KEY,        default empty -> no header sent)
  --out-dir    Directory to write CSVs into             (env: ELLE_AUDIT_OUT_DIR,    default ./deception_audit)

Output:
  Four date-stamped CSV files per run, atomic via .tmp + mv:
    <out-dir>/<YYYY-MM-DD>_deception_feedback.csv
    <out-dir>/<YYYY-MM-DD>_deception_signals.csv
    <out-dir>/<YYYY-MM-DD>_deception_speakers.csv
    <out-dir>/<YYYY-MM-DD>_deception_correlation.csv

Cron example (every Sunday at 03:15):
  15 3 * * 0 /opt/elleann/Tools/deception_audit_cron.sh \
    --url https://elle.local --admin-key "$(cat /etc/elle/admin.key)" \
    --out-dir /var/log/elle/deception_audit >> /var/log/elle/deception_audit.log 2>&1
EOF
            exit 0
            ;;
        *)
            echo "deception_audit_cron: unknown argument: $1" >&2
            echo "deception_audit_cron: see --help" >&2
            exit 2
            ;;
    esac
done

if ! command -v curl >/dev/null 2>&1; then
    echo "deception_audit_cron: curl missing on PATH" >&2
    exit 2
fi

mkdir -p "${ELLE_AUDIT_OUT_DIR}"

DATESTAMP="$(date -u +%Y-%m-%d)"
SECTIONS=(feedback signals speakers correlation)

FAILED=0
for section in "${SECTIONS[@]}"; do
    target="${ELLE_AUDIT_OUT_DIR}/${DATESTAMP}_deception_${section}.csv"
    tmp="${target}.tmp"

    if [[ -n "${ELLE_ADMIN_KEY}" ]]; then
        http_code=$(curl -sS -o "${tmp}" -w '%{http_code}' \
            -H "X-Admin-Key: ${ELLE_ADMIN_KEY}" \
            "${ELLE_HTTP_URL}/api/admin/deception/audit_csv?section=${section}" 2>/dev/null || true)
    else
        http_code=$(curl -sS -o "${tmp}" -w '%{http_code}' \
            "${ELLE_HTTP_URL}/api/admin/deception/audit_csv?section=${section}" 2>/dev/null || true)
    fi
    [[ -z "${http_code}" ]] && http_code="000"

    if [[ "${http_code}" != "200" ]]; then
        echo "[deception-audit] FAIL section=${section} http=${http_code}" >&2
        rm -f "${tmp}"
        FAILED=$((FAILED + 1))
        continue
    fi

    if [[ ! -s "${tmp}" ]]; then
        echo "[deception-audit] FAIL section=${section} empty response" >&2
        rm -f "${tmp}"
        FAILED=$((FAILED + 1))
        continue
    fi

    mv -f "${tmp}" "${target}"
    rows=$(($(wc -l < "${target}") - 1))
    if (( rows < 0 )); then rows=0; fi
    echo "[deception-audit] OK   section=${section} rows=${rows} -> ${target}"
done

if (( FAILED > 0 )); then
    echo "[deception-audit] ${FAILED} of ${#SECTIONS[@]} section(s) failed for ${DATESTAMP}." >&2
    exit 1
fi

echo "[deception-audit] All ${#SECTIONS[@]} sections snapshotted for ${DATESTAMP}."
