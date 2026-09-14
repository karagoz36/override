# OverRide

School 42 security project on the analysis and exploitation of 32-bit and
64-bit ELF binaries under Linux. It is the sequel to
[RainFall](../Rainfall).

Each level ships a SUID binary. The goal is to find its vulnerability, exploit
it to gain the next user's privileges, and read that user's password.

> This repository is for a controlled, educational environment.

## Levels

| Level | Arch | Main technique | Walkthrough |
| --- | --- | --- | --- |
| Level 00 | 32-bit | Hard-coded integer password | [View](level00/walkthrough.md) |
| Level 01 | 32-bit | Stack overflow → ret2libc | [View](level01/walkthrough.md) |
| Level 02 | 64-bit | Format-string stack leak | [View](level02/walkthrough.md) |
| Level 03 | 32-bit | Reversing a weak XOR check | [View](level03/walkthrough.md) |
| Level 04 | 32-bit | `gets` overflow → ret2libc (ptrace guard) | [View](level04/walkthrough.md) |
| Level 05 | 32-bit | Format string → overwrite `exit@GOT` (env shellcode) | [View](level05/walkthrough.md) |
| Level 06 | 32-bit | Reimplementing a serial/hash check | [View](level06/walkthrough.md) |
| Level 07 | 32-bit | Unbounded array write → ret2libc (index overflow) | [View](level07/walkthrough.md) |
| Level 08 | 64-bit | SUID backup + symlink to `.pass` | [View](level08/walkthrough.md) |
| Level 09 | 64-bit | Off-by-one → oversize `strncpy` → backdoor (bonus) | [View](level09/walkthrough.md) |

## Layout

Each level directory contains:

- `walkthrough.md` - the practical procedure, commands and explanations for evaluation;
- `concepts.md` - the background theory behind the level (the *why*, not expanded in the walkthrough);
- `source.c` - a reconstructed, commented pseudo-source of the binary;
- `flag` - the password obtained for the next user;
- `Ressources/` - a helper program, when one is genuinely useful (levels 05, 06).

No binaries are stored here (per the project rules); everything is proven live on the VM
during evaluation.

## Usage

The OverRide VM is reachable over SSH on port `4242`:

```bash
ssh level00@<vm-ip> -p 4242
```

Start at `level00` (password `level00`) and work up to `level09`, then `end`.
Becoming `root` is out of scope and counts as cheating.
