# Level 09 - Concepts

> 💡 **Core idea:** a single off-by-one byte, positioned by struct layout onto a length field,
> escalates into a full overflow of a *later* copy - redirect the 64-bit return to a hidden
> `system`-calling function.
> 🧩 **New here:** off-by-one, struct field adjacency, 64-bit return overwrite, hidden "win"
> function, PIE/ASLR.
> 🔗 **Builds on:** [level01](../level01/concepts.md) (overflow, offset). *(bonus level)*

---

## ➕ Off-by-one errors

A loop bound of `i <= 40` instead of `i < 40` copies **41** bytes into a 40-byte field - one
byte too many. That single extra byte is the whole vulnerability.

---

## 🧱 Struct layout / field adjacency

Struct fields sit back-to-back in memory, in order:

```
struct { char message_body[140]; char sender[40]; int body_len; };
          offset 0                offset 140       offset 180
                                        └── sender[40] == first byte of body_len ──┘
```

So the off-by-one `sender[40]` lands on the **first byte of `body_len`**. Writing `0xff` there
inflates `body_len` from 140 to a large value.

> 🧠 **Why layout matters:** adjacent fields mean an overflow of one bleeds directly into the
> next.

---

## 🪤 The length field controls a later copy

```
set_msg:  strncpy(message_body, input, body_len);   // body_len now huge
```

Because we corrupted `body_len`, this copy runs far past `message_body[140]` and overflows into
the saved return address.

> 🧠 A 1-byte off-by-one on the *username* turns the *message* copy into a full stack overflow.

---

## 🖥️ 64-bit return address (RSP, not EIP)

Same idea as level01 (overwrite the saved return address), but 64-bit:

- the instruction pointer is `RIP`; the return slot is read via `RSP`;
- addresses are 8 bytes;
- find the offset (**200**) the same way - cyclic pattern, then `x/s $rsp` at the crash.

---

## 🚪 Hidden "win" function

```
info functions   ->   secret_backdoor   (defined but never called)
```

`secret_backdoor` reads a line and runs it through `system()`. So no ret2libc or shellcode
needed - overwrite the return address with its address, and when it runs, type `/bin/sh`.

> 🧠 **Always scan for functions the program *defines but never calls*** - they're often the
> intended target.

---

## 🎲 PIE / ASLR note

The binary is **PIE** (addresses look like `0x5555...`). On the VM ASLR is effectively off, so
the address is stable - read it once with `print secret_backdoor` and reuse it.

> ⚠️ Under real ASLR you'd need an info leak first to defeat randomisation.

---

## 🔑 Takeaway

One off-by-one byte, placed by struct layout onto a length field, escalates into a full overflow
of a later copy. Redirect the 64-bit return address to a hidden `system`-calling function and
feed it a shell command.
