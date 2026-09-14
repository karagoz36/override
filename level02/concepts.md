# Level 02 — Concepts

> 💡 **Core idea:** `printf(user_input)` lets us aim `%`-specifiers at the process's own memory
> and *read* the password the program loaded onto the stack.
> 🧩 **New here:** the **format-string** bug, `%p`/`%N$p` reads, 64-bit differences, endian
> decoding.
> 🔗 **Builds on:** [level00](../level00/concepts.md)–[level01](../level01/concepts.md)
> (gdb, the stack).

---

## 🧠 What is a format-string bug?

`printf` doesn't know how many arguments it really received — it **trusts the format string** to
say. If *we* control the format string, every extra `%`-specifier makes `printf` fetch another
"argument" that was never passed, walking registers and the stack.

```c
printf("%s", name);   // ✅ safe — name is data
printf(name);         // ❌ bug — if name is "%p %p %p", printf leaks memory
```

---

## 🖥️ Why 64-bit changes the details

This binary is 64-bit (`rax`/`rsp`/`rbp`), so:

- each `%p` prints an **8-byte** pointer;
- the first arguments come from **registers** (`rdi, rsi, rdx, rcx, r8, r9`), then the stack.

That shifts *which* positional index lands on your buffer/the password — hence the specific
indices `%22$p … %26$p`.

---

## 🔍 Reading the stack

| Specifier | Effect |
| --- | --- |
| `%x` / `%p` | print the *next* argument (hex int / pointer) |
| `%N$p` | print the **N-th** argument directly, skipping 1…N-1 |

Feed `"AAAA %p %p %p ..."` and watch for your own `0x41414141` — that reveals where you are in
the argument list. Then jump straight to the password words with positional access.

> 🧩 The program `fread`s level03's password **onto the stack** before printing, so the secret is
> just sitting in those stack words, ready to leak.

---

## 🔡 Decoding a leak (endianness)

A leaked word like `0x48336750664b394d` is 8 bytes stored **little-endian**. Turn it back into
text:

```
hex  --xxd -r -p-->  raw bytes  --rev-->  readable
```

Printing the words high-index-first (`%26$p … %22$p`) lines the fragments up so the
reconstruction reads in order.

---

## ✍️ `%n` — reading vs writing

`%p`/`%x` only **read**. The same bug can **write**: `%n` stores "characters printed so far"
into an address argument.

> 🔗 We don't need writing here (reading the password is enough) — but remember `%n`: **level05**
> uses it to overwrite a GOT entry.

---

## 🔑 Takeaway

An uncontrolled format string turns `printf` into a memory **read** primitive (and, with `%n`, a
**write** primitive). Here: positional `%N$p` reads leak a password the program left on the
stack; decode the little-endian words back to text.
