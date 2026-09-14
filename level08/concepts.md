# Level 08 — Concepts

> 💡 **Core idea:** a SUID backup tool reads any file you name with the *owner's* rights — point
> it (via a symlink) at the protected `.pass` and it copies the secret somewhere you can read.
> 🧩 **New here:** SUID + filesystem privilege abuse, symlinks.
> 🎯 **Goal:** no memory corruption — a pure logic/filesystem bug.

---

## 🔑 SUID + file access = read anything the owner can

The binary is SUID `level09`, so `fopen(argv[1], "r")` opens the file with **`level09`'s
rights**. It will read any file `level09` can — including `/home/users/level09/.pass` — and copy
the bytes somewhere we control.

> 🧠 The bug isn't in memory: a *privileged* program reads an *attacker-named* file and hands us
> the result.

---

## 🚧 Why the direct path fails

The destination is `"./backups/" + argv[1]`:

```
./level08 /home/users/level09/.pass
->  opens dest "./backups//home/users/level09/.pass"   (dirs don't exist -> fails)
```

So the tool effectively only writes to a **plain filename** inside `./backups/`.

---

## 🔗 Symlinks

A **symbolic link** is a file whose content is a path to another file; opening the link opens
its target. Make a link whose *name* is a simple filename but whose *target* is the protected
`.pass`:

```
ln -s /home/users/level09/.pass password
./level08 password
        │                        └─ dest = ./backups/password   (we own it)
        └─ fopen("password") follows the link -> reads real .pass as level09 ✅
cat backups/password
```

> 🧠 The trick is the mismatch: the program uses the name for a harmless destination, but that
> same name *reads* from a file we're not allowed to touch.

---

## 🧵 Secondary bug: format string in the log

`log_wrapper` passes the filename straight to `snprintf` as a **format string** (same class as
[level02](../level02/concepts.md)/[level05](../level05/concepts.md)). It's exploitable, but the
symlink route is far simpler.

> ⚠️ Worth recognising the bug class even when you don't use it — graders may ask.

---

## 🔑 Takeaway

A privileged program that opens an attacker-named file can be aimed, via a symlink, at a file
you can't read — it does the reading with its own rights and drops the result somewhere you can
reach. Watch for **SUID + attacker-controlled paths**.
