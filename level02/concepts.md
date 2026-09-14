# Level 02 — Concepts

Background theory behind this level. Level02 is 64-bit and introduces the **format-string**
vulnerability used to *read* memory.

## Why 64-bit matters here

Unlike level00/01 (32-bit), this binary is 64-bit: registers are `rax`/`rsp`/`rbp`, and each
`%p` prints an 8-byte pointer. It also changes how function arguments are passed (first args go
in registers `rdi, rsi, rdx, rcx, r8, r9`, then the stack), which affects *which* format
argument index reaches your buffer.

## The format-string bug

`printf(user_string)` with **no format specifier** is the bug. `printf` doesn't know how many
arguments it was really given — it trusts the format string. If the format string is
attacker-controlled, every `%`-specifier makes `printf` fetch and act on another "argument"
that was never passed — i.e. it walks memory (registers then the stack).

Contrast:
- `printf("%s", name)` — safe, `name` is data.
- `printf(name)` — if `name` is `"%p %p %p"`, printf leaks three stack/register words.

## Reading the stack with `%p` / `%x`

- `%p` prints the next argument as a pointer (hex). `%x` as a hex int.
- Feeding `"AAAA %p %p %p ..."` dumps successive words; you'll see your own `AAAA`
  (`0x41414141`) appear once you reach the buffer, which tells you *where you are* in the arg
  list.
- **Positional access**: `%N$p` prints the N-th argument directly, without printing 1..N-1.
  That lets us jump straight to the words holding the password (`%22$p` … `%26$p`).

Here the program `fread`s level03's password onto the **stack** before printing, so the secret
is sitting in those stack words waiting to be leaked.

## Decoding a leak (endianness)

A leaked word like `0x48336750664b394d` is 8 bytes stored **little-endian**. To turn it back
into text: write the hex, convert to raw bytes, and reverse. `xxd -r -p` does hex→bytes; `rev`
reverses. Printing arguments high-index-first (`%26$p%25$p…%22$p`) lines the fragments up so
the reconstructed string reads correctly.

## `%n` — reading vs writing

`%p`/`%x` only *read*. The same bug can also *write*: `%n` stores "the number of chars printed
so far" into an address argument. We don't need writing in level02 (reading the password is
enough), but keep `%n` in mind — level05 uses it to overwrite a GOT entry.

## Takeaway

An uncontrolled format string turns `printf` into a memory-reading (and, with `%n`, memory-
writing) primitive. Here we use positional `%N$p` reads to leak a password the program had
loaded onto the stack, then decode the little-endian words back to text.
