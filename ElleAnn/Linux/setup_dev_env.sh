#!/usr/bin/env bash
set -euo pipefail

if [[ "${EUID}" -ne 0 ]]; then
    echo "setup_dev_env.sh must run as root (use sudo)." >&2
    exit 1
fi

PACKAGES=(
    cmake
    build-essential
    unixodbc-dev
    shellcheck
    git
    curl
)

echo "==> apt-get update"
apt-get update

echo "==> apt-get install ${PACKAGES[*]}"
DEBIAN_FRONTEND=noninteractive apt-get install -y "${PACKAGES[@]}"

echo
echo "==> Verify"
cmake --version | head -1
c++ --version | head -1
echo -n "unixodbc-dev: "; dpkg -s unixodbc-dev | grep -E '^(Status|Version):' | xargs

echo
echo "Linux dev env ready. Run ElleAnn/Linux/run_all_ctests.sh to verify."
