#!/usr/bin/env bash
set -u

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BIN="${PROJECT_ROOT}/ds_cli"
TMP_DIR="${PROJECT_ROOT}/tests"
PASS_COUNT=0
FAIL_COUNT=0

run_case() {
    local name="$1"
    local input="$2"
    local expected="$3"
    local output_file="${TMP_DIR}/${name}.tmp"

    printf "%s" "${input}" | "${BIN}" > "${output_file}" 2>&1

    if grep -q "${expected}" "${output_file}"; then
        echo "PASS: ${name}"
        PASS_COUNT=$((PASS_COUNT + 1))
    else
        echo "FAIL: ${name}"
        echo "  Expected pattern: ${expected}"
        echo "  See output: ${output_file}"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    fi
}

if [[ ! -x "${BIN}" ]]; then
    echo "Binary not found. Run: make build"
    exit 1
fi

echo "Running tests..."

run_case \
    "stack_normal" \
    $'1\n1\n10\n1\n20\n3\n4\n4\n' \
    "Stack (top -> bottom): 20 10"

run_case \
    "queue_underflow" \
    $'2\n2\n4\n4\n' \
    "Queue underflow"

run_case \
    "list_delete_missing" \
    $'3\n1\n5\n2\n9\n4\n4\n' \
    "Value not found in linked list"

run_case \
    "invalid_menu_input" \
    $'x\n4\n' \
    "Invalid input"

stress_input=$'1\n'
for i in $(seq 1 11); do
    stress_input+=$'1\n'"${i}"$'\n'
done
stress_input+=$'4\n4\n'
run_case \
    "stack_overflow_stress" \
    "${stress_input}" \
    "Stack overflow"

echo
echo "Test Summary: PASS=${PASS_COUNT} FAIL=${FAIL_COUNT}"

if [[ ${FAIL_COUNT} -ne 0 ]]; then
    exit 1
fi

exit 0
