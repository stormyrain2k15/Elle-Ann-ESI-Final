#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &>/dev/null && pwd)"
REPO_ROOT="$(cd -- "${SCRIPT_DIR}/.." &>/dev/null && pwd)"

CMAKE_FLAGS=(-DCMAKE_BUILD_TYPE=Debug -DCMAKE_POLICY_VERSION_MINIMUM=3.5)

run_harness () {
    local label="$1"
    local src_dir="$2"
    local extra_cmake_flags="${3:-}"
    local build_target="${4:-}"
    local run_mode="${5:-ctest}"

    local build_dir="${src_dir}/build"
    rm -rf "${build_dir}"
    mkdir -p "${build_dir}"

    echo
    echo "==================================================================="
    echo "  ${label}"
    echo "==================================================================="

    if [[ -n "${extra_cmake_flags}" ]]; then
        cmake -S "${src_dir}" -B "${build_dir}" "${CMAKE_FLAGS[@]}" ${extra_cmake_flags}
    else
        cmake -S "${src_dir}" -B "${build_dir}" "${CMAKE_FLAGS[@]}"
    fi

    if [[ -n "${build_target}" ]]; then
        cmake --build "${build_dir}" --target "${build_target}" -j"$(nproc)"
    else
        cmake --build "${build_dir}" -j"$(nproc)"
    fi

    if [[ "${run_mode}" == "doctest-binary" ]]; then
        "${build_dir}/elle_tests"
    else
        (cd "${build_dir}" && ctest --output-on-failure)
    fi
}

run_harness "Intuition (39 cases)"   "${REPO_ROOT}/Services/Elle.Service.Intuition"
run_harness "Probability (85 cases)" "${REPO_ROOT}/Services/Elle.Service.Probability"
run_harness "Composer (17 cases)"    "${REPO_ROOT}/Services/Elle.Service.Composer"
run_harness "Language (48 cases)"    "${REPO_ROOT}/Services/Elle.Service.Language" "-DELLE_WITH_ODBC=OFF" "elle_tests" "doctest-binary"
run_harness "_Shared (40 cases)"     "${REPO_ROOT}/Services/_Shared/tests"

echo
echo "==================================================================="
echo "  All five Linux ctest harnesses green (229 total cases)."
echo "==================================================================="
