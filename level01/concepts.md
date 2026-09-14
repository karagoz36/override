# Level 01 — Concepts

Background theory behind this level. This is the big one: it introduces the whole
overflow → control EIP → ret2libc chain that later levels build on.

## 1. Reading a binary without source

The binary has **no debugging symbols** (no DWARF), so gdb can't show variable names, types,
or line numbers. But the **function symbol table** survives (the binary isn't stripped), so
function *names and addresses* are still visible.

- `info functions` — lists the program's own functions (plus `@plt` libc stubs). This is how
  you discover `verify_user_name` / `verify_user_pass`.
- `disassemble <func>` — shows a function's assembly. In `main`, `call ... <name>` lines also
  reveal the function names.
- `x/s <addr>` — read a string at an address (e.g. the `"dat_wil"` / `"admin"` constants).

**Variable names are yours to infer.** The binary only has addresses; you name a buffer by
what it's used for (the thing `fgets` fills and gets compared to `"dat_wil"` → "username").

## 2. The stack, registers, and `ret`

Each function call gets a **stack frame**: arguments, the **saved return address**, the saved
`EBP`, then local variables. The stack grows *downward* (toward lower addresses).

Key registers (32-bit):

| Register | Meaning |
| --- | --- |
| **EIP** | address of the next instruction — control this and you control the program |
| **ESP** | top of the stack |
| **EBP** | base of the current frame (locals are addressed relative to it) |
| **EAX** | function return value / scratch |

`LEA` (Load Effective Address) computes an address without dereferencing it:
`lea 0x1c(%esp),%eax` puts *the address* `esp+0x1c` into `eax` (a buffer pointer), not the
value there.

The crucial instruction is **`ret`**: it pops 4 bytes off the top of the stack into `EIP` and
jumps there. Normally that popped value is the real return address left by the caller.

## 3. Stack buffer overflow

`fgets(password, 100, stdin)` reads up to 100 bytes into a **64-byte** buffer. The extra bytes
run past the buffer and overwrite whatever sits above it on the stack — eventually the **saved
return address**. When `main` executes `ret`, it loads *our bytes* into `EIP`. We now decide
where the program jumps.

(Note the password check is a tautology in this binary, so it never actually blocks us — the
only thing that matters is that `fgets` overflowed.)

## 4. Finding the EIP offset (cyclic pattern)

We need to know exactly how many bytes of input land on the return-address slot. Feed a
**cyclic / De Bruijn pattern** (`Aa0Aa1Aa2...`, every 4-byte window unique) as the password.
At the crash, `EIP` holds the 4 pattern bytes that overwrote the return address
(e.g. `0x37634136` = `"6Ac7"` little-endian). Find that substring's position in the pattern →
**offset 80**. So 80 bytes of padding, then the next 4 bytes are `EIP`.

## 5. NX — why we can't just run shellcode

In RainFall you wrote shellcode into a buffer and jumped to it. Here the **NX (No-eXecute)**
protection marks the stack non-executable: the CPU refuses to run instructions located on the
stack. Jumping to stack shellcode just faults. So we can't run *our* code — we must reuse code
that already exists in an executable region.

## 6. ret2libc

Every process maps **libc**, which already contains executable functions like `system` and
`exit`, and even the string `"/bin/sh"`. Instead of injecting code, we overwrite the return
address with the address of `system` and hand it `"/bin/sh"`.

**Calling convention (cdecl, 32-bit):** when a function starts, the stack holds
`[return address][arg1][arg2]...`. So we forge a fake frame after the padding:

```
[ "A" * 80 ]
[ &system    ]  <- ret jumps here; system starts running
[ &exit      ]  <- the "return address" system sees (runs when the shell exits)
[ &"/bin/sh" ]  <- system's arg1
```

- Addresses are written **little-endian** (least-significant byte first): `system` at
  `0xf7e6aed0` → `\xd0\xae\xe6\xf7`.
- `find &system, +9999999, "/bin/sh"` locates the ready-made string inside libc, so we don't
  inject it.
- **`exit` is optional** — it just makes the program terminate cleanly instead of segfaulting
  when the shell exits. You can replace it with any 4-byte placeholder (e.g. `BBBB`), but you
  can't *delete* it, or `"/bin/sh"` would shift out of the argument slot.

## Takeaway

Overflow to overwrite the saved return address, measure the offset with a cyclic pattern, then
(because NX blocks stack shellcode) redirect execution into libc's `system("/bin/sh")` via a
forged stack frame. Every technique here recurs in later levels.
