# Level 00 - Practical walkthrough

> 🔓 **Flaw:** SUID binary + hard-coded magic password
> 🎯 **Target:** the `password == 5276` check
> 🛠️ **Technique:** supply `5276` to keep level01's rights and open `/bin/sh` (no memory corruption).

All commands are run as user `level00`.

## 1. Observe the program

🟦 **Goal:** check the SUID bit (permission that runs the binary with the rights of its
owner) and see how the program behaves.

🟨 **Terminal:**

```bash
ls -l ./level00
./level00
```

⬜ **Output:**

```text
-rwsr-s---+ 1 level01 users 7280 Sep 10  2016 level00
***********************************
*            -Level00 -           *
***********************************
Password:test

Invalid Password!
```

The `s` in `-rwsr-s---` is the SUID bit: the binary is owned by `level01`, so it executes
with `level01`'s rights. It asks for a password and rejects anything wrong.

## 2. Find the expected value

🟦 **Goal:** disassemble `main` to identify the comparison performed on our input.

🟨 **Terminal:**

```bash
gdb ./level00
```

🟨 **GDB:**

```gdb
disassemble main
quit
```

⬜ **Output (relevant lines):**

```text
0x080484de <+74>:  call   0x80483d0 <__isoc99_scanf@plt>
0x080484e3 <+79>:  mov    eax,DWORD PTR [esp+0x1c]
0x080484e7 <+83>:  cmp    eax,0x149c
0x080484ec <+88>:  jne    0x804850d <main+121>
```

`scanf` stores our integer at `[esp+0x1c]`. That value is loaded into `eax` and compared
(`cmp`) to `0x149c`. If it does not match, `jne` jumps to the failure branch; otherwise the
program calls `system("/bin/sh")`. Convert the hex value to decimal:

🟨 **Terminal:**

```bash
python -c "print(int('149c', 16))"
```

⬜ **Output:**

```text
5276
```

So the expected password is `5276`.

## 3. Run the shell

🟦 **Goal:** pass the magic value to reach `system("/bin/sh")` and read the next password.

🟨 **Terminal:**

```bash
./level00
```

🟨 **In the program**, at the prompt type only the value (`Password:` is printed by the program):

- `Password:` → type `5276`

A shell with `level01`'s rights opens. Read the password:

🟨 **In the shell:**

```bash
whoami
cat /home/users/level01/.pass
```

⬜ **Output:**

```text
level01
uSq2ehEGT6c9S24zbshexZQBXUGrncxn5sD5QfGL
```

## 4. Move to the next level

🟨 **Terminal:**

```bash
su level01
```

(Enter the password above.) Then, in the shell:

🟨 **In the shell:**

```bash
whoami
```

⬜ **Output:**

```text
level01
```
