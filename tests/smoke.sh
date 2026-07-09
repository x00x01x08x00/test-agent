#!/usr/bin/env bash
set -euo pipefail

if [[ ${EUID} -ne 0 ]]; then
    echo "This smoke test needs root for the fd-based fanotify sample."
    echo "Run: sudo ./tests/smoke.sh"
    exit 77
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
tmp_dir="$(mktemp -d)"
watch_dir="${tmp_dir}/watch"
log_file="${tmp_dir}/fanwatch.log"
watcher_pid=""

cleanup() {
    if [[ -n "${watcher_pid}" ]] && kill -0 "${watcher_pid}" 2>/dev/null; then
        kill -TERM "${watcher_pid}" 2>/dev/null || true
        wait "${watcher_pid}" 2>/dev/null || true
    fi
    rm -rf "${tmp_dir}"
}
trap cleanup EXIT

mkdir -p "${watch_dir}"

"${repo_root}/bin/fanwatch" "${watch_dir}" >"${log_file}" 2>&1 &
watcher_pid=$!

sleep 0.5

if ! kill -0 "${watcher_pid}" 2>/dev/null; then
    wait "${watcher_pid}" 2>/dev/null || true
    watcher_pid=""
    cat "${log_file}"

    if grep -Eq 'Function not implemented|Operation not permitted|Permission denied' "${log_file}"; then
        echo "SKIP: this environment cannot run the fanotify sample"
        exit 77
    fi

    echo "FAIL: watcher exited before the test started"
    exit 1
fi

printf 'hello\n' >"${watch_dir}/sample.txt"
printf 'world\n' >>"${watch_dir}/sample.txt"
sleep 0.5

kill -INT "${watcher_pid}"
wait "${watcher_pid}" || true
watcher_pid=""

cat "${log_file}"

if ! grep -Eq 'MODIFY|CLOSE_WRITE' "${log_file}"; then
    echo "FAIL: no child file change event was observed"
    exit 1
fi

echo "PASS: child file change event observed"
