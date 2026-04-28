#!/usr/bin/env bash
set -u

SRC="${SRC:-program.c}"
BIN="${BIN:-program}"
PROGRAM="${PROGRAM:-./$BIN}"
WORK_DIR="${WORK_DIR:-./.tmp}"

TOTAL=0
PASS=0
FAIL=0

mkdir -p "$WORK_DIR"

build_program() {
  if ! gcc "$SRC" -o "$BIN"; then
    echo "Compilation failed: gcc $SRC -o $BIN"
    exit 1
  fi
}

run_case_contains() {
  case_name="$1"
  input_data="$2"
  expected_pattern="$3"

  TOTAL=$((TOTAL + 1))
  out_file="$WORK_DIR/${TOTAL}_${case_name}.out"

  printf "%b" "$input_data" | "$PROGRAM" > "$out_file" 2>&1

  if grep -Eq -- "$expected_pattern" "$out_file"; then
    PASS=$((PASS + 1))
    echo "[PASS] $case_name"
  else
    FAIL=$((FAIL + 1))
    echo "[FAIL] $case_name"
    echo "  Expected pattern: $expected_pattern"
    echo "  Output file: $out_file"
  fi
}

run_case_and_compare_file() {
  case_name="$1"
  input_data="$2"
  expected_file="$3"

  TOTAL=$((TOTAL + 1))
  out_file="$WORK_DIR/${TOTAL}_${case_name}.out"

  printf "%b" "$input_data" | "$PROGRAM" > "$out_file" 2>&1

  if diff -u "$expected_file" "$out_file" > "$WORK_DIR/${TOTAL}_${case_name}.diff"; then
    PASS=$((PASS + 1))
    echo "[PASS] $case_name"
  else
    FAIL=$((FAIL + 1))
    echo "[FAIL] $case_name"
    echo "  See diff: $WORK_DIR/${TOTAL}_${case_name}.diff"
  fi
}

create_expected_for_exact_case() {
  cat > "$WORK_DIR/expected_exact_display.txt" <<'EOF'

=== STACK MENU ===
1. Insert
2. Delete
3. Display
4. Exit
Enter choice: Enter value to insert: Inserted: 10

=== STACK MENU ===
1. Insert
2. Delete
3. Display
4. Exit
Enter choice: Enter value to insert: Inserted: 20

=== STACK MENU ===
1. Insert
2. Delete
3. Display
4. Exit
Enter choice: Stack elements (top to bottom): 20 10

=== STACK MENU ===
1. Insert
2. Delete
3. Display
4. Exit
Enter choice: Exiting program.
EOF
}

run_stress_case() {
  TOTAL=$((TOTAL + 1))
  case_name="stress_50_insert_50_delete"
  out_file="$WORK_DIR/${TOTAL}_${case_name}.out"

  {
    for i in $(seq 1 50); do
      echo "1"
      echo "$i"
    done
    for _ in $(seq 1 50); do
      echo "2"
    done
    echo "3"
    echo "4"
  } | "$PROGRAM" > "$out_file" 2>&1

  if grep -Eq "Underflow: stack is empty." "$out_file" && grep -Eq "Exiting program." "$out_file"; then
    PASS=$((PASS + 1))
    echo "[PASS] $case_name"
  else
    FAIL=$((FAIL + 1))
    echo "[FAIL] $case_name"
    echo "  Expected stress behavior not found."
    echo "  Output file: $out_file"
  fi
}

main() {
  build_program
  create_expected_for_exact_case

  run_case_contains "insert_operation" "1\n42\n4\n" "Inserted: 42"
  run_case_contains "delete_operation" "1\n99\n2\n4\n" "Deleted: 99"
  run_case_contains "display_operation" "1\n11\n1\n22\n3\n4\n" "Stack elements \(top to bottom\): 22 11"

  run_case_contains "underflow_case" "2\n4\n" "Underflow: stack is empty."
  run_case_contains "empty_display_case" "3\n4\n" "Stack is empty."
  run_case_contains "overflow_case" "1\n1\n1\n2\n1\n3\n1\n4\n1\n5\n1\n6\n4\n" "Overflow: stack is full."

  run_case_and_compare_file \
    "exact_output_case" \
    "1\n10\n1\n20\n3\n4\n" \
    "$WORK_DIR/expected_exact_display.txt"

  run_stress_case

  echo
  echo "===================="
  echo "Total: $TOTAL"
  echo "PASS : $PASS"
  echo "FAIL : $FAIL"
  echo "===================="

  [ "$FAIL" -eq 0 ]
}

main "$@"
