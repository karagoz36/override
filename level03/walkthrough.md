# Level 03 — Practical walkthrough

> 🔓 **Flaw:** weak XOR "encryption" of the password check
> 🎯 **Target:** the key that decrypts the ciphertext to `"Congratulations"`
> 🛠️ **Technique:** recover the XOR key from one known plaintext byte, then invert the arithmetic to get the input.

All commands are run as user `level03`.

## 1. Observe the program

🟦 **Goal:** confirm the SUID bit and behaviour.

🟨 **Terminal:**

```bash
ls -l ./level03
./level03
```

⬜ **Output:**

```text
-rwsr-s---+ 1 level04 users 7677 Sep 10  2016 level03
***********************************
*		level03		**
***********************************
Password:hello

Invalid Password
```

Owned by `level04` and SUID. It reads a password and rejects it.

## 2. Understand the logic

🟦 **Goal:** trace what happens to our input.

🟨 **Terminal:**

```bash
gdb ./level03
```

🟨 **GDB:**

```gdb
info functions
disassemble main
disassemble test
disassemble decrypt
```

`info functions` lists the program's own functions (ignore the `@plt` libc stubs) — that is
how we discover `test` and `decrypt` and know to disassemble them. Their names also appear on
the `call ... <name>` lines inside `main`.

Reconstructing the flow:

- `main` reads an integer `input` and calls `test(input, 322424845)`.
- `test` derives `key = 322424845 - input`. Only a small set of keys is accepted (a `switch`,
  covering 1–9 and 16–21); any other key decrypts with a **random** byte — a dead end.
- `decrypt(key)` XORs the fixed ciphertext `"Q}|u`sfg~sf{}|a3"` byte-by-byte with `key`,
  and if the result equals `"Congratulations"` it calls `system("/bin/sh")`.

So we need the single-byte `key` (0–21) such that `ciphertext ^ key == "Congratulations"`.

## 3. Recover the key

🟦 **Goal:** XOR is reversible with a known plaintext byte. The first cipher byte is `'Q'`
and the first plaintext byte must be `'C'`, so `key = 'Q' ^ 'C'`.

🟨 **Terminal:**

```bash
python -c "print(ord('Q') ^ ord('C'))"
```

⬜ **Output:**

```text
18
```

Sanity-check the whole string against this key:

🟨 **Terminal:**

```bash
python -c "print(''.join(chr(ord(c) ^ 18) for c in 'Q}|u\x60sfg~sf{}|a3'))"
```

⬜ **Output:**

```text
Congratulations!
```

`key = 18`. (18 is in the accepted 16–21 range, so `test` uses our key, not the random path.)

## 4. Invert the arithmetic

🟦 **Goal:** turn the key back into the input to type. `key = 322424845 - input`, so
`input = 322424845 - 18`.

🟨 **Terminal:**

```bash
python -c "print(322424845 - 18)"
```

⬜ **Output:**

```text
322424827
```

## 5. Run the exploit

🟦 **Goal:** feed the input and keep stdin open so the shell stays interactive.

🟨 **Terminal:**

```bash
(echo 322424827; cat -) | ./level03
```

🟨 **In the shell:**

```bash
whoami
cat /home/users/level04/.pass
```

⬜ **Output:**

```text
level04
kgv3tkEb9h2mLkRsPkXRfc2mHbjMxQzvb2FrgKkf
```

## 6. Move to the next level

🟨 **Terminal:**

```bash
su level04
```

(Enter the password above.) Then:

🟨 **In the shell:**

```bash
whoami
```

⬜ **Output:**

```text
level04
```
