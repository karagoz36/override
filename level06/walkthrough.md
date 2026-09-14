# Level 06 - Practical walkthrough

> 🔓 **Flaw:** deterministic, reversible hash used as a "serial" check
> 🎯 **Target:** the `serial == hash(login)` test inside `auth()`
> 🛠️ **Technique:** reimplement the hash offline and compute a valid serial for a chosen login (bypasses the `ptrace` anti-debug guard).

All commands are run as user `level06`. This binary is **32-bit**.

## 1. Observe the program

🟦 **Goal:** confirm the SUID bit and the login/serial flow.

🟨 **Terminal:**

```bash
ls -l ./level06
./level06
```

⬜ **Output:**

```text
-rwsr-s---+ 1 level07 users 7907 Sep 10  2016 level06
***********************************
*		level06		  *
***********************************
-> Enter Login: Me!
***** NEW ACCOUNT DETECTED ********
-> Enter Serial: 1234
```

Owned by `level07` and SUID. It wants a *Login* and a matching *Serial*.

## 2. Understand `auth()`

🟦 **Goal:** read the hashing algorithm and the anti-debug guard.

🟨 **Terminal:**

```bash
gdb ./level06
```

🟨 **GDB:**

```gdb
info functions
disassemble main
disassemble auth
```

`info functions` lists the program's own functions (ignore the `@plt` libc stubs) - that is
how we discover `auth` and know to disassemble it. Its name also appears on the
`call ... <auth>` line inside `main`.

Reconstructing `auth(login, serial)`:

- Requires `strnlen(login, 32) > 5` (login at least 6 chars) and each byte printable (> 31).
- Calls `ptrace(PTRACE_TRACEME, 0, 1, 0)`; if a debugger is attached this returns `-1` and
  the program prints **TAMPERING DETECTED** and bails. This blocks naive gdb inspection.
- Seeds a hash: `hash = (login[3] ^ 0x1337) + 0x5eeded` (i.e. `(login[3] ^ 4919) + 6221293`).
- For each byte: `hash += (hash ^ login[i]) % 1337`. The binary implements `% 1337` with a
  magic-number multiplication (`0x88233b2b`, shifts, `imul 0x539`), but it is exactly a
  modulo 1337.
- Succeeds only if `serial == hash`, then `main` runs `system("/bin/sh")`.

The hash depends only on the login, so we compute it ourselves - no runtime tampering needed.

## 3. Compute a valid serial

🟦 **Goal:** reproduce the algorithm and get the serial for a login (here `username`). Use the
`Ressources/serial.c` helper.

🟨 **Terminal:**

```bash
gcc Ressources/serial.c -o /tmp/serial
/tmp/serial username
```

⬜ **Output:**

```text
serial for 'username' = 6234463
```

(As a one-liner, without the helper:)

🟨 **Terminal:**

```bash
python3 -c "
login='username'
h=(ord(login[3])^0x1337)+0x5eeded
for c in login: h+=(h^ord(c))%1337
print(h & 0xffffffff)
"
```

⬜ **Output:**

```text
6234463
```

## 4. Run the exploit

🟦 **Goal:** enter the login and its serial to reach `system("/bin/sh")`.

🟨 **Terminal:**

```bash
./level06
```

🟨 **In the program**, type only the value at each prompt (the `-> Enter ...:` text is printed):

- `-> Enter Login:` → `username`
- `-> Enter Serial:` → `6234463`

⬜ **Output:**

```text
Authenticated!
```

🟨 **In the shell:**

```bash
whoami
cat /home/users/level07/.pass
```

⬜ **Output:**

```text
level07
GbcPDRgsFK77LNnnuh7QyFYA2942Gp8yKj9KrWD8
```

> Alternative (gdb) route: set a breakpoint at the `ptrace` return and force `set $eax=0` to
> pass the guard, then break at the `serial` compare and read the computed hash. Computing it
> offline is cleaner and needs no debugger.

## 5. Move to the next level

🟨 **Terminal:**

```bash
su level07
```

(Enter the password above.) Then:

🟨 **In the shell:**

```bash
whoami
```

⬜ **Output:**

```text
level07
```
