# Level 07 — Concepts

> 💡 **Core idea:** an unchecked array index is a *write-what-where* primitive — poke a ret2libc
> frame directly onto the saved return address, using unsigned overflow to dodge the index guard.
> 🧩 **New here:** write-what-where via array indexing, unsigned integer-overflow bypass.
> 🔗 **Builds on:** [level01](../level01/concepts.md) (ret2libc).

---

## 🎯 Arbitrary write through an unbounded index

`store_number` does `tab[index] = value` with **no bounds check**, and `tab` is on the stack:

```
tab[index]  ==  *(tab + index*4)     // index is attacker-chosen
```

Pick the right index and you write any 4-byte value **anywhere** on the stack — including
`main`'s saved return address.

> 🧠 This is a **write-what-where** primitive: more surgical than a linear overflow. You don't
> smash everything in between; you poke exactly the slots you want.

---

## 📐 Turning an index into the return address

The saved return address sits at a fixed distance from `tab`:

```
index = (eip_address - table_address) / 4  =  114
```

- index **114** → the EIP slot
- index **115, 116** → the next two dwords (ret2libc frame's return address and argument)

---

## 🔄 The guard and unsigned integer overflow

The program blocks `index % 3 == 0` (so 114 is blocked) and numbers whose top byte is 183. The
escape abuses that `index` is **unsigned 32-bit** and the access multiplies by 4:

```
(2^32 / 4) + 114 = 1073741938
1073741938 % 3   = 1            ✅ passes the guard
1073741938 * 4   ≡ 114*4  (mod 2^32)   ✅ same address
```

> 🧠 **Integer-overflow bypass:** the arithmetic wraps modulo 2³², so two very different
> "indices" resolve to the *same* memory. One passes the `% 3` check, the other doesn't — but
> they hit the same slot.

---

## 🧩 Same ret2libc as before

Once you can write those three slots, it's the familiar frame — `system`, `exit`, `"/bin/sh"` —
entered as **decimals** (the program reads numbers). `argv`/`env` are wiped by `main`, so
shellcode has nowhere to live; reusing libc is the way. (Frame layout & little-endian:
[level01](../level01/concepts.md).)

---

## 🔑 Takeaway

An unchecked array index is a write-what-where primitive: compute the index that maps onto the
saved return address, and use unsigned wraparound to slip past an index guard while hitting the
same slot. Then it's ret2libc.
