# Level 01 — Concepts

> 💡 **Core idea:** overflow a buffer to overwrite the saved return address, then (because NX
> blocks stack shellcode) redirect execution into libc's `system("/bin/sh")`.
> 🧩 **New here:** reversing without symbols, the stack & registers, buffer overflow, EIP
> offset, NX, **ret2libc**.
> 🔗 **Builds on:** [level00](../level00/concepts.md) (SUID).

This is the big one — the chain here recurs in almost every later level.

---

## 🔍 1. Reading a binary without source

The binary has **no debugging symbols** (no variable names, types, or line numbers). But the
**function symbol table** survives, so function *names and addresses* are still visible.

| Command | What it gives you |
| --- | --- |
| `info functions` | the program's own functions (+ `@plt` libc stubs) |
| `disassemble <fn>` | a function's assembly |
| `x/s <addr>` | the string at an address (e.g. `"dat_wil"`, `"admin"`) |

> ⚠️ **Variable names are yours to infer.** The binary only has addresses. You name a buffer by
> what it *does* — the thing `fgets` fills and that gets compared to `"dat_wil"` is "username".

---

## 🧱 2. The stack, registers, and `ret`

Each function call gets a **stack frame** — laid out (high → low address) as:

```
   arguments
   saved return address   <-- our real target
   saved EBP
   local variables (buffers)   <-- fgets writes here, grows toward the return address
```

Registers (32-bit):

| Reg | Meaning |
| --- | --- |
| **EIP** | address of the next instruction — *control this = control the program* |
| **ESP** | top of the stack |
| **EBP** | base of the frame (locals addressed from it) |
| **EAX** | return value / scratch |

> 📖 **`LEA`** loads an *address*, not a value: `lea 0x1c(%esp),%eax` puts the address
> `esp+0x1c` into `eax` (a buffer pointer).
>
> 📖 **`ret`** pops 4 bytes off the top of the stack into `EIP` and jumps there. Normally that's
> the real return address — but we're about to replace it.

---

## 💥 3. The buffer overflow

```
fgets(password, 100, stdin);   // reads up to 100 bytes...
char password[64];             // ...into a 64-byte buffer
```

The extra bytes spill past the buffer and climb the stack until they overwrite the **saved
return address**. When `main` runs `ret`, it loads *our* bytes into `EIP`.

> ⚠️ The password check is a tautology in this binary (`==0 || !=0`), so it never blocks us. The
> only thing that matters is that `fgets` overflowed.

---

## 📏 4. Finding the EIP offset (cyclic pattern)

We need the exact number of bytes before the return-address slot. Feed a **cyclic pattern**
(`Aa0Aa1Aa2...`, every 4-byte window unique) as the password:

```
crash:  EIP = 0x37634136   ==  "6Ac7"  (little-endian)
find "6Ac7" in the pattern ->  position 80
```

So **80 bytes** of padding, then the next 4 bytes are `EIP`.

---

## 🛡️ 5. NX — why we can't run our own shellcode

> In RainFall you wrote shellcode into a buffer and jumped to it.

Here the **NX (No-eXecute)** bit marks the stack non-executable: the CPU refuses to run
instructions located on the stack. Stack shellcode just faults. So we can't run *our* code — we
must reuse code that already lives in an executable region.

---

## 🧩 6. ret2libc

Every process maps **libc**, which already contains `system`, `exit`, and even the string
`"/bin/sh"`. Instead of injecting code, we point the return address at `system` and forge its
call frame.

> 📖 **Calling convention (cdecl, 32-bit):** when a function starts, the stack holds
> `[return address][arg1][arg2]...`.

So after the padding we lay down a fake frame:

```
[ "A" * 80 ]
[ &system    ]  <- ret jumps here; system starts
[ &exit      ]  <- the "return address" system sees (runs when the shell exits)
[ &"/bin/sh" ]  <- system's arg1
```

- Addresses are **little-endian**: `system` at `0xf7e6aed0` → `\xd0\xae\xe6\xf7`.
- `find &system, +9999999, "/bin/sh"` locates the ready-made string *inside* libc — no need to
  inject it.

> ⚠️ **`exit` is optional but its slot isn't.** `exit` just makes the program end cleanly instead
> of segfaulting when the shell exits. You may replace it with any 4-byte placeholder (`BBBB`),
> but you can't *delete* it — that would shift `"/bin/sh"` out of the argument position.

---

## 🔑 Takeaway

Overflow → overwrite the saved return address → measure the offset with a cyclic pattern → and
because NX blocks stack shellcode, jump into libc's `system("/bin/sh")` via a forged frame.
Everything after this reuses these ideas.
