# Level 07 — Concepts

Background theory behind this level. The overflow here is not a linear buffer overflow but an
**arbitrary write via an unchecked array index**, used to place a ret2libc frame.

## Arbitrary write through an unbounded index

`store_number` does `tab[index] = value` with **no bounds check** on `index`, and `tab` is an
array on the **stack**. So `tab[index]` is really `*(tab + index*4)` — an attacker-chosen
address. Pick the right index and you write any 4-byte value anywhere on the stack, including
`main`'s saved return address. This is a *write-what-where* primitive, more surgical than a
linear overflow: you don't smash everything in between, you poke exactly the slots you want.

## Turning an index into the return address

The saved return address is at a fixed distance from `tab`. Compute it:

```
index = (eip_address - table_address) / 4
```

Here that's 114. Storing at index 114 overwrites EIP's slot; indices 115 and 116 are the next
two dwords, which become the ret2libc frame's return address and argument.

## The guard and unsigned integer overflow

The program blocks `index % 3 == 0` (index 114 is blocked) and numbers whose top byte is 183.
The escape uses the fact that `index` is **unsigned 32-bit** and the access is `index * 4`.
Adding `2^32 / 4` to the index changes its value (so `% 3` differs) but, after multiplying by 4
and wrapping modulo `2^32`, lands on the **same address**:

```
(2^32 / 4) + 114 = 1073741938,   1073741938 % 3 == 1   // passes the guard, same slot
```

This is a classic **integer-overflow bypass**: the arithmetic wraps, so two very different
"indices" resolve to the same memory.

## Same ret2libc as before

Once you can write those three slots, the payload is the familiar ret2libc frame —
`system`, `exit`, `"/bin/sh"`, entered as decimals because the program reads numbers. argv and
env are wiped by `main`, so shellcode has nowhere to live; reusing libc is the way. (See
level01 concepts for the frame layout and little-endian details.)

## Takeaway

An unchecked array index is a write-what-where primitive: compute the index that maps onto the
saved return address, and use unsigned-integer wraparound to dodge an index guard while hitting
the same slot. Then it's ret2libc again.
