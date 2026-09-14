# Level 04 — Concepts

> 💡 **Core idea:** a `gets()` overflow guarded by a `ptrace` watchdog that kills `execve` — but
> `system` runs the exec in an untraced *grandchild*, so ret2libc slips through.
> 🧩 **New here:** `fork`, `ptrace` anti-exec, syscall numbers, follow-fork-mode.
> 🔗 **Builds on:** [level01](../level01/concepts.md) (overflow, offset, ret2libc).

---

## 🍴 fork(): parent and child

`fork()` duplicates the process. It returns:

```
0            in the CHILD
child's PID  in the PARENT
```

Here the **child** runs the vulnerable `gets()`; the **parent** stays behind to police it.

---

## 💥 gets() — the overflow

`gets(buf)` reads a line with **no size limit whatsoever** (there is no safe way to call it).
Into a 32-byte buffer, it's the same overflow as level01 — smash past the buffer, overwrite the
saved return address, control EIP.

> 📏 The offset here is **156**. (See [level01](../level01/concepts.md) for the
> overflow/offset/EIP mechanics.)

---

## 👁️ ptrace as an anti-exec guard

`ptrace` is the syscall debuggers use to trace another process. This binary weaponises it:

```
child:   ptrace(PTRACE_TRACEME, ...)          "trace me"
parent:  ptrace(PTRACE_PEEKUSR, child, 44, 0)  read child's ORIG_EAX (attempted syscall #)
         if that number == 11 (execve) -> "no exec() for you", kill child
```

> 📖 **Offset 44** in the child's user area is **`ORIG_EAX`** — the number of the syscall it just
> attempted. **Syscall 11 = `execve`.**

So any shellcode that calls `execve("/bin/sh")` is caught and killed. Injecting exec shellcode
is a dead end (and NX would block stack shellcode anyway).

---

## 🧩 Why ret2libc `system` still works

```
our traced child  ── calls ──▶  system("/bin/sh")
                                    │  system() does fork()
                                    ▼
                              grandchild  ── execve ──▶  /bin/sh
                              (NOT traced by the parent)
```

The parent only watches our **direct** child, not the grandchild `system` spawns. The forbidden
`execve` happens one process further down, unseen.

> 🧠 **The lesson:** a *per-process* syscall filter is defeated by making the forbidden syscall
> happen in a *different* process.

---

## 🔍 follow-fork-mode in gdb

The bug is in the child, but gdb keeps debugging the parent by default:

```
set follow-fork-mode child
```

Now gdb follows the child across the `fork` so you can crash it and read the offset.

---

## 🔑 Takeaway

A `ptrace` guard that kills the child on `execve` doesn't stop ret2libc: `system` spawns an
untraced grandchild to do the exec. Move the forbidden action to another process and the filter
never sees it.
