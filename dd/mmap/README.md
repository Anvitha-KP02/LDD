## Goal
Learn what real `mmap()` does by contrasting it with a user-space, mmap-like helper that uses `malloc()` + `read()`.

## Build
```bash
make
```

## Run
```bash
./demo_my_mmap ./demo_my_mmap.c
./demo_my_mmap ./demo_my_mmap.c 64
```

## What to notice
- `my_mmap()` copies bytes into a heap buffer (immediate I/O + memory copy).
- Real `mmap()` creates a *virtual memory mapping* and typically loads pages lazily via page faults (no upfront copy of the entire file).
