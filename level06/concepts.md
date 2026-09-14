# Level 06 — Concepts

Background theory behind this level. Like level03 it's a **deterministic check you
reimplement**, plus a stack canary and a ptrace anti-debug trick.

## The check is a pure function of the login

`auth()` computes a hash from the login and requires `serial == hash`. Nothing random is
involved (unlike level03's wrong-key branch), so the serial is a **deterministic function** of
the login. That means we can compute it ourselves offline — no runtime tricks needed.

## Reimplementing the algorithm

Read the disassembly, translate it to C, and run it. The algorithm here:

```
hash = (login[3] ^ 0x1337) + 0x5eeded         // seed
for each byte c in login (must be > 31):
    hash += (hash ^ c) % 1337                  // fold each byte in
```

Feed it a login of your choice and it prints the matching serial. This "re-derive the check in
your own code" approach is a core RE skill and reappears whenever a program validates input
with a self-contained computation.

## Magic-number division (again)

The `% 1337` isn't emitted as a division. The compiler uses reciprocal multiplication:
multiply by `0x88233b2b`, shift, then `imul 0x539` (`0x539 = 1337`). Recognise it as modulo
1337 rather than tracing every instruction. (Same trick as level03.)

## ptrace anti-debug

`ptrace(PTRACE_TRACEME, ...)` returns **-1 if the process is already being traced** (e.g. under
gdb). The code uses this to detect debuggers and print `TAMPERING DETECTED`. Two ways around
it: patch the return in gdb (`set $eax=0` at the check), or — simpler — never attach a debugger
at all and just compute the serial offline.

## Stack canary (why not overflow?)

The disassembly shows a **stack canary**: `mov %gs:0x14,%eax` stored near the saved return
address, checked before `ret` with `__stack_chk_fail`. A canary is a secret value placed
between the locals and the return address; if an overflow overwrites it, the mismatch aborts
the program. That's why the intended route here is *not* a buffer overflow but computing a
valid serial.

## Takeaway

When a program validates input with a deterministic, self-contained computation, the cleanest
attack is to reimplement that computation and produce a valid input — sidestepping anti-debug
guards and stack canaries entirely.
