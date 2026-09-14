# Level 08 — Concepts

Background theory behind this level. No memory corruption — this is a **logic / filesystem
privilege** bug exploited with a symlink.

## SUID + file access = privilege to read anything the owner can

The binary is SUID `level09`, so when it calls `fopen(argv[1], "r")` the open happens with
**`level09`'s rights**. The program will read any file `level09` can read — including
`/home/users/level09/.pass` — and then copy the bytes somewhere we control. The vulnerability
isn't in memory; it's that a privileged program reads an attacker-named file and hands us the
result.

## Why the direct path fails

The destination is built as `"./backups/" + argv[1]`. Passing an absolute path gives
`./backups//home/users/level09/.pass`, whose directories don't exist, so `open` fails. The tool
effectively only writes to a plain filename inside `./backups/`.

## Symlinks

A **symbolic link** is a file whose content is a path to another file; opening the link opens
its target (following the link). We create a link whose *name* is a simple filename but whose
*target* is the protected `.pass`:

```
ln -s /home/users/level09/.pass password
```

Now `fopen("password", "r")` follows the link and reads the real `.pass` (as `level09`), while
the destination stays a tidy `./backups/password` that we own and can read. The mismatch
between "the name the program uses for the destination" and "what that name actually points to
for reading" is the whole trick.

## Secondary bug: format string in the log

`log_wrapper` passes the filename straight to `snprintf` as a **format string** (same class of
bug as level02/05). It's exploitable, but the symlink route is far simpler, so we don't need
it. Worth noting because recognising the bug class matters even when you don't use it.

## Takeaway

A privileged program that opens an attacker-named file can be aimed, via a symlink, at a file
you're not allowed to read — it does the reading with its own rights and deposits the result
somewhere you can reach. Watch for SUID + attacker-controlled paths.
