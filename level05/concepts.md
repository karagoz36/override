# Level 05 — Concepts

> 💡 **Core idea:** stash shellcode in an env var (where it *can* run), then use a format-string
> `%hn` write to point `exit@GOT` at it — so the program's own `exit(0)` jumps to our shellcode.
> 🧩 **New here:** env-var shellcode, NOP sled, **PLT/GOT**, `%n`/`%hn` writes, short writes.
> 🔗 **Builds on:** [level02](../level02/concepts.md) (format strings),
> [level01](../level01/concepts.md) (NX).

This is the most machinery-heavy level — take it in pieces.

---

## 🛡️ NX recap → put the shellcode in the environment

The stack is non-executable, so stack shellcode won't run. But **environment variables** are
copied into the new process in a region that (on this VM) is still executable. So we hide
`execve("/bin/sh")` shellcode in an env var and aim execution there.

### 🛬 NOP sled

Env addresses shift a little with the environment, so we can't hit the shellcode's first byte
exactly. Prefix it with a long run of `\x90` (**NOP** = "do nothing"):

```
[ \x90 \x90 \x90 ... \x90 ][ real shellcode ]
        NOP sled              land anywhere in here and slide right ▶
```

It turns a precise jump into an approximate one.

---

## 🔗 PLT and GOT

External functions live in libc, whose address isn't known at link time, so calls go through
two tables:

- **PLT** (Procedure Linkage Table): a small stub per function that *your* code calls.
- **GOT** (Global Offset Table): the table of *resolved* real addresses. The PLT stub jumps to
  `*GOT[func]`.

```
x/i 0x8048370        ; exit@plt
=>  jmp  *0x80497e0   ; 0x080497e0 = exit@GOT   <-- writable!
```

> 🎯 Overwrite `exit@GOT` with our shellcode's address, and the final `exit(0)` jumps to the
> shellcode instead of libc's `exit`.

---

## ✍️ Format-string write with `%n`

`%n` writes "characters printed so far" to the address given as that argument. Our input buffer
*is* on the stack (and is itself an argument), so:

1. put the target address(es) at the **start** of the buffer;
2. use `%N$n`, where `N` is the buffer's argument index (found by leaking with `%x`; here **10**).

### 🪓 Short writes (`%hn`)

Writing a full 32-bit address with one `%n` would mean printing billions of chars. `%hn` writes
only **2 bytes**. So split the address into two halves and write twice:

```
target 0xffffd832  ->  low 0xd832 = 55346 , high 0xffff = 65535
%hn write #1 -> exit@GOT      (low half)
%hn write #2 -> exit@GOT + 2  (high half)
```

The width fields (`%55338d`, `%10189d`) pad the output so the running count equals each half —
subtracting the bytes already printed (the 8 front-loaded address bytes, then the first half).

---

## 🔡 The tolower quirk

The program lowercases uppercase letters (`0x41`–`0x5a`) before printing. Our address bytes and
format specifiers are never uppercase ASCII, so they survive untouched.

---

## 🔑 Takeaway

Put the code where it *can* execute (env var + NOP sled), then use the format-string `%hn`
primitive to overwrite a writable **GOT** entry (`exit`), so the program's own `exit(0)` hands
control to your shellcode. PLT/GOT and short writes are standard tools from here on.
