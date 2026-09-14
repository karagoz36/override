# Level 03 - Concepts

> 💡 **Core idea:** the "encryption" guarding the check is reversible XOR - one known byte hands
> you the key, then you invert the arithmetic to get the input.
> 🧩 **New here:** XOR reversibility, known-plaintext attacks, switch/jump tables,
> magic-number division.
> 🎯 **Goal:** no memory corruption - pure reverse engineering.

---

## 🔁 XOR is reversible

XOR is its own inverse:

```
a ^ k ^ k == a
encrypt:  c = p ^ k
decrypt:  p = c ^ k     (same key, same operation)
```

The program XORs every byte of a fixed ciphertext with a single-byte `key`; if the result is
`"Congratulations"`, you win.

---

## 🕵️ Known-plaintext attack

We know both the ciphertext (`"Q}|u..."`) **and** what the plaintext must be
(`"Congratulations"`). One matching pair leaks the key:

```
key = cipher[0] ^ plaintext[0] = 'Q' ^ 'C' = 0x12 = 18
```

Check by XORing the whole string with 18 → `"Congratulations!"`. ✅

> 🧠 **Intuition:** if you know any input/output pair of a XOR, the key is just their XOR. That's
> why XOR alone is never security.

---

## ➗ Working backwards through the arithmetic

The key isn't the input. `test()` computes `key = 322424845 - input`, so:

```
input = 322424845 - key = 322424845 - 18 = 322424827
```

---

## 🧩 Switch / jump tables

The real control flow is a **switch**, not a simple `if`. Only specific keys are accepted (1-9
and 16-21); any other key decrypts with `rand()` - a dead end. Our key 18 is in range.

> 🔍 **In the disassembly** a switch usually looks like a bounds check plus an indirect jump
> through a table of addresses.

---

## ⚙️ Magic-number division

Compilers rarely emit a real `div` for `%` or `/`. Instead you'll see a multiply by a weird
constant plus shifts (**reciprocal multiplication**).

> ⚠️ Don't trace every instruction - recognise the pattern and read it as the modulo/division it
> implements. (This same trick returns in [level06](../level06/concepts.md).)

---

## 🎲 rand / srand

`srand(time(0))` seeds the PRNG from the clock; the wrong-key branch calls `rand()`. That path
is deliberately unpredictable, so brute-forcing decryption is pointless - derive the one correct
key instead.

---

## 🔑 Takeaway

"Encrypting" a check with XOR isn't security: XOR is reversible and a single known plaintext
byte gives you the key. Recover the key, invert the surrounding math, compute the exact input.
