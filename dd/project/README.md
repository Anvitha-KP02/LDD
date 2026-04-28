# Data Structures CLI (Production-Style C Project)

A menu-driven command-line application in C that implements core data structures with clean architecture, defensive input handling, logging, and automated tests.

## Description

This project is designed like a real-world GitHub repository rather than a single-file demo.  
It provides modular implementations of:

- Stack (array-based)
- Queue (circular array-based)
- Linked List (singly linked)

The CLI provides insert/delete/display operations for each structure with robust input validation and clear runtime messages.

## Features

- Menu-driven interactive CLI with nested menus
- Stack / Queue / Linked List operations:
  - Insert
  - Delete
  - Display
- Defensive input parsing (`strtol`-based validation)
- Underflow and overflow error handling
- Basic file logging (`app.log`)
- Optional colorized terminal output via ANSI escape codes
- Debug mode support using compile-time macro (`DEBUG`)

## Folder Structure

```text
project/
├── src/
│   ├── main.c
│   ├── stack.c
│   ├── queue.c
│   ├── list.c
│   └── utils.c
├── include/
│   ├── stack.h
│   ├── queue.h
│   ├── list.h
│   └── utils.h
├── tests/
│   └── test.sh
├── Makefile
└── README.md
```

## Build Instructions

```bash
make build
```

Compiler defaults:

- `gcc`
- `-Wall -Wextra -Werror -std=c11`

## Run Instructions

```bash
make run
```

Or run directly:

```bash
./ds_cli
```

## Debug Mode

Build with debug tracing enabled:

```bash
make debug
```

In debug mode, internal state updates are printed using `DEBUG_PRINT(...)`.

## Sample Output

```text
============================================
 Data Structures CLI (Stack/Queue/List)
============================================

Main Menu
1. Stack
2. Queue
3. Linked List
4. Exit
Choose an option [1-4]: 1

Stack Menu
1. Insert
2. Delete
3. Display
4. Back
Choose an option [1-4]: 1
Enter value to insert: 42
[OK] Value inserted into stack.
```

## Testing Instructions

Run all tests:

```bash
make test
```

The `tests/test.sh` script validates:

- Normal behavior (e.g., insert + display)
- Edge behavior (invalid input, delete from empty structure)
- Stress behavior (stack overflow scenario)

Test output includes PASS/FAIL per scenario and an aggregated summary.

## Screenshots (Text-Based)

```text
[Main Menu Screenshot]
1. Stack
2. Queue
3. Linked List
4. Exit
```

```text
[Error Handling Screenshot]
[ERROR] Invalid input. Enter a valid menu option.
[ERROR] Stack underflow. Nothing to delete.
```

## Logging

Operations are logged to:

```text
app.log
```

Examples:

- application start/exit
- operation success/failure
- invalid input warnings

## Future Enhancements

- Persistent storage (save/load DS state from disk)
- Generic data type support (`void *` based structures)
- Unit tests with a C test framework (e.g., CMocka)
- Benchmark mode for operation performance
- Optional ncurses-based TUI for richer interaction

## License

This project is suitable for educational and portfolio use.  
You can add an OSS license (MIT/Apache-2.0) based on your publishing needs.
