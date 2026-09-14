# Level 06 — Concepts

> 💡 **Core idea:** the "serial" is a deterministic function of the login — reimplement it and
> compute a valid serial yourself, sidestepping the anti-debug guard and the stack canary.
> 🧩 **New here:** reimplementing an algorithm offline, `ptrace` anti-debug, stack canaries.
> 🔗 **Builds on:** [level03](../level03/concepts.md) (deterministic reversing, magic-number
> division), [level04](../level04/concepts.md) (ptrace).

---

## 🎯 The check is a pure function of the login

`auth()` hashes the login and requires `serial == hash`. Nothing random is involved, so the
serial is a **deterministic function** of the login — computable offline. No runtime tricks
needed.

---

## 🧮 Reimplementing the algorithm

Read the disassembly, translate to C, run it:

```c
hash = (login[3] ^ 0x1337) + 0x5eeded;      // seed
for each byte c in login (must be > 31):
    hash += (hash ^ c) % 1337;              // fold each byte in
```

Feed it a login of your choice → it prints the matching serial.

> 🧠 **This "re-derive the check in your own code" skill** applies whenever a program validates
> input with a self-contained computation.

---

## ⚙️ Magic-number division (again)

The `% 1337` is emitted as reciprocal multiplication: multiply by `0x88233b2b`, shift, then
`imul 0x539` (`0x539 = 1337`). Recognise it as modulo 1337 rather than tracing each step. (Same
trick as [level03](../level03/concepts.md).)

---

## 👁️ ptrace anti-debug

```
ptrace(PTRACE_TRACEME, ...)  ->  returns -1 if already traced (e.g. under gdb)
```

The code uses this to print `TAMPERING DETECTED` when a debugger is attached. Two ways around
it:

- patch the return in gdb (`set $eax = 0` at the check), **or**
- 🟢 simpler: never attach a debugger — compute the serial offline.

---

## 🐤 Stack canary — why not overflow?

The disassembly shows a **stack canary**:

```
mov  %gs:0x14, %eax     ; load secret canary
... store near the saved return address ...
xor  %gs:0x14, %edx     ; check it before ret
call __stack_chk_fail   ; abort if it changed
```

> 📖 A **canary** is a secret value placed between the locals and the saved return address. An
> overflow that reaches the return address also clobbers the canary; the mismatch aborts the
> program. That's why the intended route here is computing a serial, **not** a buffer overflow.

---

## 🔑 Takeaway

When a program validates input with a deterministic, self-contained computation, reimplement
that computation and produce a valid input — bypassing anti-debug guards and stack canaries
entirely.
