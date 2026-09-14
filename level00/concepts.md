# Level 00 — Concepts

> 💡 **Core idea:** an SUID binary runs with its *owner's* rights — make it hand you a shell.
> 🧩 **New here:** SUID, reading one branch in the disassembly, hex ↔ decimal.
> 🎯 **Goal of the level:** pass a hard-coded integer check to reach `system("/bin/sh")`.

The walkthrough shows *what to do*; this file explains *why it works*.

---

## 📖 What is SUID?

A file can carry the **SUID bit** — shown as an `s` where the owner's `x` would be in
`ls -l`:

```
-rwsr-s---+ 1 level01 users 7280 ... level00
   ^
   this 's' = SUID
```

When you run an SUID binary, it executes with the **owner's** privileges, not yours. `level00`
is owned by `level01`, so any shell it spawns is a `level01` shell.

> 🧠 **Why this matters:** this is the engine of the *whole* project — every level is "make an
> SUID binary owned by the next user spawn a shell, then read their `.pass`."

---

## ⚙️ How the check works

The program reads an integer and compares it to a fixed value. No memory corruption — pure
logic:

```c
scanf("%d", &pin);       // read a decimal integer
if (pin != 5276) { ... reject ... }
else { system("/bin/sh"); }
```

---

## 🔍 Seeing it in gdb

```
disassemble main
```

The decisive lines:

```
cmp    eax, 0x149c      ; compare our input (in eax) to 0x149c
jne    <reject>         ; branch away if they differ
```

- `cmp` subtracts and sets CPU flags; `jne` jumps if "not equal".
- Assembly shows constants in **hex**. `0x149c` = `5276` decimal
  (`python -c "print(int('149c',16))"`). Since `scanf("%d")` wants decimal, you type `5276`.

---

## 🧠 Why a shell instead of "just print the pass"

`system("/bin/sh")` starts a shell, and because the process is still SUID `level01`, that
shell has `level01`'s rights. From it you `cat` `level01`'s `.pass` — a file your own account
can't read.

---

## 🔑 Takeaway

SUID → runs as owner → get a shell → read the next `.pass`. No exploitation yet, just
recognising the mechanism and reading a single `cmp`/`jne` branch.
