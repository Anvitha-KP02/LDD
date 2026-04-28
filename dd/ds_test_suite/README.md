# DS Program Test Suite

Simple Bash-based validator for terminal menu-driven C Data Structure programs (Stack/Queue/Linked List style).

## Folder contents

- `test.sh` - automated test runner with PASS/FAIL per case
- `sample_expected_outputs.txt` - sample expected outputs/messages
- `.tmp/` - runtime output artifacts (auto-created)

## Prerequisites

- Linux or macOS terminal
- `gcc`
- `rg` (ripgrep, typically available in dev environments)

## Target assumptions

The default script flow assumes:

- Source file: `program.c`
- Binary: `program`
- Menu pattern:
  - `1` Insert
  - `2` Delete
  - `3` Display
  - `4` Exit

If your menu numbers differ, edit the input blocks in `test.sh`.

## Run

From the project directory containing `program.c`:

```bash
gcc program.c -o program
cd dd/ds_test_suite
chmod +x test.sh
../ds_test_suite/test.sh
```

Or run directly with variables:

```bash
cd dd/ds_test_suite
SRC=../../c/your_ds_program.c BIN=your_ds_program PROGRAM=./your_ds_program ./test.sh
```

## Example test report

```text
[PASS] insert_and_display
[PASS] multiple_inserts
[FAIL] underflow_on_empty_delete
  Expected (contains): underflow
  Actual: ... your output ...
  Raw output file: ./ds_test_suite/.tmp/4_underflow_on_empty_delete.out

====================
Total: 7
PASS : 6
FAIL : 1
====================
```

## Notes

- The suite covers:
  - Insert
  - Delete
  - Display
  - Edge cases (empty, underflow, overflow)
  - Stress test (many inserts/deletes)
- Overflow check mainly applies to fixed-capacity array implementations.
