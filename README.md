# 🔍 Reverse Engineering Challenges — Walkthrough

> Detailed walkthroughs and solution scripts for reverse engineering CTF challenges.

---

## 📖 About This Repository

This repository documents my journey through reverse engineering CTF challenges. Each writeup covers the full thought process — from initial binary triage to the final flag — not just the answer. The goal is to build a reference that is useful both for my own learning and for anyone working through similar challenges.

Writeups are written to be beginner-accessible where possible, but they don't skip the technical detail that makes them actually useful.

---

## 🗂️ Repository Structure

```
Reverse Engineering/
├── Challenge1/
│   ├── challenge-file
│   └── README.md
├── Challenge2/
│   ├── challenge-file
│   └── README.md
└── ...
```
Each challenge directory contains:
- A `README.md` with the full walkthrough
- The relevant challenge file(s)
- A solution script (C/C++, Python, or Bash depending on the challenge)

---

## 🧰 Tools Used

| Tool | Purpose |
|---|---|
| [Ghidra](https://ghidra-sre.org/) | Static analysis and decompilation |
| [IDA Pro / IDA Free](https://hex-rays.com/) | Static analysis and disassembly |
| [GDB](https://www.gnu.org/software/gdb/) + [pwndbg](https://github.com/pwndbg/pwndbg) | Dynamic analysis and runtime debugging |
| Python 3 | Scripting, decryption solvers, automation |
| C / C++ | Low-level solution scripts where performance or syscall-level access matters |
| Bash | Utility scripting and quick one-liners |

---

## 📚 Writeup Index

| Challenge | Topics | Writeup |
|---|---|---|
| Simple Encryptor | PRNG, XOR, Bit Rotation | [View](./Challenge1/README.md) |

> The index is updated as new writeups are added.

---

## 🔎 Topics Covered

Challenges in this repository span a range of reverse engineering concepts including, but not limited to:

- Static and dynamic binary analysis
- PRNG-based encryption schemes and seed recovery
- XOR ciphers and bitwise operations
- Custom encryption and encoding algorithms
- Anti-debugging and obfuscation techniques
- ELF binary structure and format analysis
- Keygen and license validation reversing
- Packed and obfuscated binaries

---

## ⚙️ Running the Solution Scripts

### Python

```bash
python3 solve.py
```

### C / C++

```bash
gcc -o solve solve.c && ./solve
```

### Bash

```bash
chmod +x solve.sh && ./solve.sh
```

Dependencies, if any, are noted at the top of each script.

---

## ⚠️ Disclaimer

All challenges documented here are sourced from legal, intentional CTF platforms. Writeups for active or ongoing competitions are not published until the challenge has been retired or the event has concluded, in compliance with each platform's rules.

This repository is strictly for educational purposes.

---

## 📬 Contact

If you spot an error in a writeup, have a better approach, or just want to discuss a challenge — open an issue or reach out directly.

---

*Happy reversing.*
