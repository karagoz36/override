# Level 09 — Concepts

Background theory behind this level (bonus). It chains an **off-by-one** into a length field, a
struct-layout abuse, and a **64-bit** return-address overwrite to a hidden function.

## Off-by-one errors

A loop bound of `i <= 40` instead of `i < 40` copies **41** bytes into a 40-byte field — one
byte too many. That single extra byte is enough here, because of what sits immediately after
the field in memory.

## Struct memory layout / field adjacency

The struct is laid out in order, fields back-to-back:

```
struct { char message_body[140]; char sender[40]; int body_len; };
          offset 0                offset 140       offset 180
```

`sender` is 40 bytes, so `sender[40]` (the off-by-one byte) lands on the **first byte of
`body_len`**. Overwriting it with `0xff` inflates `body_len` from 140 to a large value. This is
why struct layout matters: adjacent fields mean an overflow of one bleeds into the next.

## Length field controls a later copy

`set_msg` then does `strncpy(message_body, input, body_len)`. Because we corrupted `body_len`,
the copy runs far past `message_body[140]`, overflowing into the saved return address. So a
1-byte off-by-one on the username turns the *message* copy into a full stack overflow.

## 64-bit return address (RSP, not EIP)

This binary is 64-bit. Conceptually it's the same as level01 (overwrite the saved return
address, control the instruction pointer), but the register is `RIP`/the return slot is read
via `RSP`, addresses are 8 bytes, and the offset (200 here) is found the same way — a cyclic
pattern and checking where the return address points at the crash (`x/s $rsp`).

## Hidden "win" function

`info functions` reveals **`secret_backdoor`**, which is never called by the program. It reads
a line and runs it through `system()`. So we don't even need ret2libc or shellcode — just
overwrite the return address with `secret_backdoor`'s address, and when it runs, type
`/bin/sh`. Always scan for functions the program *defines but never calls* — they're often the
intended target.

## PIE / ASLR note

The binary is PIE and addresses look like `0x5555...`. On the VM ASLR is effectively off, so the
address is stable and you can read it once with `print secret_backdoor` and reuse it. Under real
ASLR you'd need an info leak first.

## Takeaway

One off-by-one byte, positioned by struct layout onto a length field, escalates into a full
overflow of a *later* copy. Redirect the 64-bit return address to a hidden `system`-calling
function and feed it a shell command.
