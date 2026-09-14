# Level 03 — Concepts

Background theory behind this level. No memory corruption — this is a **reverse-engineering /
crypto** level: understand the check, then invert it.

## XOR and reversibility

XOR (`^`) is its own inverse: `a ^ k ^ k == a`. So a byte "encrypted" as `c = p ^ k` is
decrypted by `p = c ^ k`. The program stores a fixed ciphertext and XORs every byte with a
single-byte `key`; if the result equals `"Congratulations"`, you win.

## Known-plaintext attack

Because we know both the ciphertext (`"Q}|u..."`) and what the plaintext must be
(`"Congratulations"`), we can recover the key from **one** pair of bytes:

```
key = cipher[0] ^ plaintext[0] = 'Q' ^ 'C' = 0x12 = 18
```

Verify by XORing the whole string with 18 → `"Congratulations!"`. This is the essence of a
known-plaintext attack: one matching byte leaks the key.

## Working backwards through the arithmetic

The key isn't the input. `test()` computes `key = 322424845 - input`, so once we know
`key = 18` we invert: `input = 322424845 - 18 = 322424827`.

## Switch / jump tables

The binary doesn't use a simple `if (key < 21)`. The compiler built a **switch** that only
accepts specific keys (1–9 and 16–21); anything else decrypts with a *random* byte
(`rand()`), a dead end. Our key 18 falls in the accepted range, so the real key is used. When
you disassemble, a switch usually appears as a bounds check plus an indirect jump through a
table of addresses.

## Magic-number division

The "modulo" the compiler emits often looks nothing like `%`: it's a multiply by a magic
constant plus shifts (reciprocal multiplication). Don't let that scare you — recognise the
pattern and treat it as the modulo it implements. (This exact trick reappears in level06.)

## rand / srand

`srand(time(0))` seeds the PRNG from the clock, and the wrong-key branch calls `rand()`. That
path is deliberately unpredictable so brute-forcing the *decryption* is pointless — the
intended solution is to derive the one correct key deterministically, as above.

## Takeaway

When a program "encrypts" a check with XOR, it isn't security — XOR is reversible and a single
known plaintext byte hands you the key. Recover the key, invert any surrounding arithmetic,
and compute the exact input.
