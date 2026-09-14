# Level 05 — Concepts

Background theory behind this level. It combines a **format-string write** (`%n`) with a
**GOT overwrite** and **env-var shellcode**. This is the most machinery-heavy level.

## NX recap — why the shellcode goes in an env var

The stack is non-executable (NX), so shellcode on the stack won't run. But **environment
variables** are copied onto the stack of the new process in a region that (on this VM) is still
executable. So we stash `execve("/bin/sh")` shellcode in an env var and aim execution there.

### NOP sled

Env addresses shift slightly depending on the environment, so we can't hit the shellcode's
first byte exactly. We prefix it with a long run of `\x90` (**NOP** = "do nothing"). Landing
anywhere in this sled just slides the CPU forward until it reaches the real shellcode. It turns
a precise jump into an approximate one.

## PLT and GOT

External functions (`printf`, `exit`, …) live in libc, whose address isn't known at link time.
So the binary calls through two tables:

- **PLT** (Procedure Linkage Table): a small stub per function, called by your code.
- **GOT** (Global Offset Table): a table of *actual* resolved addresses. The PLT stub jumps to
  `*GOT[func]`.

`x/i 0x8048370` on `exit@plt` shows `jmp *0x80497e0` → `0x080497e0` is **`exit@GOT`**. Crucially
the GOT is **writable**. If we overwrite `exit@GOT` with our shellcode's address, then the
program's final `exit(0)` jumps into our shellcode instead of libc's `exit`.

## Format-string write with `%n`

`%n` writes "the number of characters printed so far" to the address given as that argument.
Combined with the fact that our input buffer *is* on the stack (and is itself a format
argument), we can:

1. Put the target address(es) at the start of the buffer.
2. Use `%N$n` to write to them, where `N` is the buffer's argument index (found by leaking with
   `%x`, here index 10).

### Short writes (`%hn`)

A full 32-bit address as one `%n` would require printing billions of characters. `%hn` writes
only **2 bytes** (a "short"). We split the target address into a low half and a high half and
do two writes: one to `exit@GOT`, one to `exit@GOT+2`. The width fields (`%55338d`, `%10189d`)
pad the output so the running character count equals each half's value. We subtract the bytes
already printed (the 8 bytes of the two front addresses, then the first half) so the counts
line up.

## The tolower quirk

The program lowercases uppercase letters (`0x41`–`0x5a` XOR `0x20`) before printing. Our
address bytes and format specifiers are never uppercase ASCII, so they pass through untouched —
which is why the raw address bytes survive.

## Takeaway

Put shellcode where it *can* execute (env var + NOP sled), then use the format-string `%hn`
primitive to overwrite a writable **GOT** entry (`exit`) so the program's own `exit(0)` hands
control to the shellcode. Learn PLT/GOT and short writes here — they're standard exploitation
tools.
