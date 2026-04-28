# Production-Grade Menu-Driven Data Structures in C

A production-style, menu-driven CLI mini product built in pure C.  
This project implements a **Task Manager** using classic data structures with robust user input handling, structured logging, automated build targets, and test/debug workflows.

## Why This Project?

This repository demonstrates how to evolve data structures practice into an engineering-grade deliverable:

- Linked list-backed task database
- Queue-backed execution pipeline (FIFO)
- Strong CLI UX with input validation loops
- Persistent storage for save/load flows
- File-based operational logging for observability
- Automated build, test, and debug tooling

---

## Features

- **Menu-Driven UX**
  - Add, list, update, delete tasks
  - Filter tasks by assignee
  - Enqueue/dequeue tasks for execution
  - Save/load task data from disk
- **Data Structures**
  - Singly linked list for dynamic task storage
  - Queue for task execution scheduling
- **Validation & Safety**
  - Input sanitization with retry loops
  - Constrained value checks (priority, status, IDs)
  - Defensive string handling
- **Engineering Tooling**
  - Release and debug build targets
  - AddressSanitizer + UBSan enabled debug/test runs
  - Unit tests for linked list and queue operations
  - Structured timestamped logging

---

## Project Structure

```text
ds_task_manager/
├── include/
│   ├── cli.h
│   ├── logger.h
│   └── task_manager.h
├── src/
│   ├── cli.c
│   ├── ds.c
│   ├── logger.c
│   ├── main.c
│   └── storage.c
├── tests/
│   └── test_ds.c
├── scripts/
│   └── run_debug.sh
├── Makefile
└── README.md
```

---

## Build & Run

### 1) Build release binary

```bash
make build
```

### 2) Run CLI

```bash
make run
```

Optional custom data file:

```bash
./bin/task_manager data/my_tasks.db
```

### 3) Run tests

```bash
make test
```

### 4) Debug build with sanitizers

```bash
make debug
./bin/task_manager
```

Or:

```bash
./scripts/run_debug.sh
```

---

## CLI Workflow Example

1. Add tasks with title, assignee, and priority
2. Mark task status (`TODO`, `IN_PROGRESS`, `DONE`)
3. Enqueue tasks for execution
4. Dequeue in FIFO order
5. Save to `data/tasks.db`
6. Reload later from disk

---

## Logging

Runtime logs are stored in:

```text
logs/task_manager.log
```

Each event is timestamped and categorized with level (`INFO`, `WARN`, `ERROR`), useful for debugging and operational traceability.

---

## Engineering Notes

- **Compiler**: GCC with `-Wall -Wextra -Wpedantic`
- **Standards**: C11
- **Debug Safety**: `-fsanitize=address,undefined`
- **Memory**: Explicit allocation/free lifecycle for list/queue nodes

---

## Future Extensions

- Priority queue scheduling
- Search/sort modules
- CSV/JSON export
- Authentication for multi-user CLI sessions
- ncurses-based interactive terminal UI

---

## License

For educational and internal project use.
