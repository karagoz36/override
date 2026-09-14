# Level 00 — Concepts

Background theory behind this level. The walkthrough shows *what to do*; this file explains
*why it works*.

## SUID binaries

A file can carry the **SUID bit** (`-rwsr-xr-x`, the `s` where `x` would be). When such a
binary is executed, it runs with the **owner's** privileges, not the caller's. Here `level00`
is owned by `level01`, so any shell it spawns runs as `level01`. That is the whole point of
the game: make an SUID binary spawn a shell so we inherit the next user's rights and read
their `.pass`.

Spot it with `ls -l`: the `s` in the owner's execute position is the SUID bit.

## scanf and integer comparison

`scanf("%d", &x)` reads a signed decimal integer from stdin into `x`. The program then does a
plain `if (x != 5276)`. There is no memory corruption here — it is pure logic. If you supply
the exact value, you pass.

## Reading the check in gdb

- `disassemble main` prints the assembly.
- The decisive instruction is `cmp eax, 0x149c` followed by a conditional jump (`jne`). `cmp`
  subtracts and sets flags; `jne` branches if the two values differ.
- `eax` holds the integer we typed. So the program compares our input to `0x149c`.

## Hex vs decimal

Assembly shows constants in hexadecimal. `0x149c` = `5276` in decimal. Convert with
`python -c "print(int('149c',16))"`. Since `scanf("%d")` expects decimal, we type `5276`.

## Why a shell, not just "print the pass"

The binary calls `system("/bin/sh")`. `system` runs a command through a shell; because the
process is still SUID `level01`, that shell has `level01`'s rights. From it we can `cat`
`level01`'s `.pass`, which our own account could not read.

## Takeaway

Level00 teaches the *setup* for every later level: SUID → run as owner → get a shell → read
the next `.pass`. No exploitation yet, just recognising the mechanism and reading one branch
in the disassembly.
