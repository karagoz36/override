# Level 04 — Concepts

Background theory behind this level. It's a `gets()` overflow (like level01's overflow) wrapped
in a **ptrace anti-exec guard**, so the new ideas are process tracing and why ret2libc slips
past it.

## fork(): parent and child

`fork()` duplicates the process. It returns `0` in the **child** and the child's PID in the
**parent**. Here the child runs the vulnerable `gets()`; the parent stays behind to police the
child.

## gets() — the overflow

`gets(buf)` reads a line with **no size limit** at all (there is no way to make it safe). Into
a 32-byte buffer, it's a textbook stack overflow — same mechanism as level01: overflow past
the buffer, overwrite the saved return address, control EIP. (See level01 concepts for the
overflow/offset/EIP details; the offset here is 156.)

## ptrace as an anti-debug / anti-exec guard

`ptrace` is the syscall debuggers use to trace another process. This binary weaponises it:

- The child calls `ptrace(PTRACE_TRACEME, ...)` to let the parent trace it.
- The parent loops on `ptrace(PTRACE_PEEKUSR, child, 44, 0)`. Offset **44** in the child's
  user area is **`ORIG_EAX`** — the number of the syscall the child just attempted. Syscall
  **11** is `execve`. If the parent ever sees 11, it prints `no exec() for you` and kills the
  child.

So any shellcode that calls `execve("/bin/sh")` is detected and killed. Injecting exec
shellcode is a dead end here (and NX would block stack shellcode anyway).

## Why ret2libc `system` still works

`system("/bin/sh")` doesn't call `execve` in *our* traced child. Internally `system` does
`fork()` and the **grandchild** performs the `execve`. The parent only traces our direct child,
not the grandchild — so the exec is never seen. This is the key insight: bypass a per-process
syscall filter by making the forbidden syscall happen in a *different* process.

Everything else (finding `system`/`exit`/`"/bin/sh"`, the little-endian fake frame) is
identical to level01.

## follow-fork-mode in gdb

Because the bug is in the child, plain gdb would keep debugging the parent. `set
follow-fork-mode child` tells gdb to follow the child across the `fork`, so you can crash it
and read the offset.

## Takeaway

A `ptrace` guard that kills the child on `execve` doesn't stop ret2libc, because `system`
spawns an untraced grandchild to do the exec. Per-process syscall policing is defeated by
moving the syscall to another process.
