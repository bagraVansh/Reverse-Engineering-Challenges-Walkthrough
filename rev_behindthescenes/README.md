# Behind The Scenes – HackTheBox Write-up

## Overview

The challenge provides a compressed archive which, after extraction, contains a single executable binary named `behindthescenes`. The goal is to analyze this binary and recover the correct password that reveals the flag.

---

## Initial Execution

Running the binary without arguments:

```bash
./behindthescenes
```

Output:

```
./challenge <password>
```

So it expects a password as an argument. Goal is clear — find the correct password.

---

## Checking for Hardcoded Strings

First thing I tried was `strings` to see if the password is just sitting there in plaintext:

```bash
strings behindthescenes
```

Nothing useful came out. No obvious password. So the binary is doing something more interesting.

---

## Static Analysis — Ghidra

Loaded the binary into Ghidra and navigated to `main()`. The decompiled output looked like this:

```c
void main(void)
{
  code *pcVar1;
  long in_FS_OFFSET;
  sigaction local_a8;
  undefined8 local_10;

  local_10 = *(undefined8 *)(in_FS_OFFSET + 40);
  memset(&local_a8, 0, 152);
  sigemptyset(&local_a8.sa_mask);
  local_a8.__sigaction_handler.sa_handler = segill_sigaction;
  local_a8.sa_flags = 4;
  sigaction(4, &local_a8, (sigaction *)0x0);

  pcVar1 = (code *)invalidInstructionException();
  (*pcVar1)();
}
```

No password logic here. What caught my attention was `sigaction(4, ...)` — signal `4` is `SIGILL` (Illegal Instruction). The program is registering `segill_sigaction` as a custom handler for `SIGILL`, then intentionally calling `invalidInstructionException()`.

---

## Understanding the Trick — SIGILL + UD2

Looking at the assembly, `invalidInstructionException()` contains a `UD2` instruction — an x86 instruction that always triggers `SIGILL`.

Normally this would crash the program. But since a custom `SIGILL` handler (`segill_sigaction`) was registered beforehand, the OS calls that function instead of terminating the process.

Looking at `segill_sigaction()` in Ghidra:

```c
void segill_sigaction(int signal, siginfo_t *info, void *ucontext)
{
  ucontext_t *ctx = (ucontext_t *)ucontext;
  ctx->uc_mcontext.gregs[REG_RIP] += 2;
}
```

The handler just advances RIP by 2 bytes (the size of `UD2`), so execution continues right after it as if nothing happened. The program never actually crashes — the `UD2` is purely a trick to break the decompiler's control flow analysis and hide the real logic.

---

## Patching the Binary

To get a clean decompilation, I patched `UD2` (`0F 0B`) to `NOP NOP` (`90 90`) directly in Ghidra:

- Located `UD2` in the Listing view
- Right-click → **Patch Instruction** → replaced with `NOP`
- Re-ran auto-analysis

---

## Decompiled `main()` After Patching

```c
undefined8 main(int param_1, long param_2)
{
  int iVar1;
  undefined8 uVar2;
  size_t sVar3;
  long in_FS_OFFSET;
  sigaction local_a8;
  long local_10;

  local_10 = *(long *)(in_FS_OFFSET + 40);
  memset(&local_a8, 0, 152);
  sigemptyset(&local_a8.sa_mask);
  local_a8.__sigaction_handler.sa_handler = segill_sigaction;
  local_a8.sa_flags = 4;
  sigaction(4, &local_a8, (sigaction *)0x0);

  if (param_1 == 2) {

    sVar3 = strlen(*(char **)(param_2 + 8));

    if (sVar3 == 0xc) {

      iVar1 = strncmp(*(char **)(param_2 + 8), "Itz", 3);
      if (iVar1 == 0) {

        iVar1 = strncmp((char *)(*(long *)(param_2 + 8) + 3), "_0n", 3);
        if (iVar1 == 0) {

          iVar1 = strncmp((char *)(*(long *)(param_2 + 8) + 6), "Ly_", 3);
          if (iVar1 == 0) {

            iVar1 = strncmp((char *)(*(long *)(param_2 + 8) + 9), "UD2", 3);
            if (iVar1 == 0) {
              printf("> HTB{%s}\n", *(undefined8 *)(param_2 + 8));
              uVar2 = 0;
            }
          }
        }
      }
    }
  }
  else {
    puts("./challenge <password>");
    uVar2 = 1;
  }

  if (local_10 != *(long *)(in_FS_OFFSET + 0x28)) {
    __stack_chk_fail();
  }

  return uVar2;
}
```

---

## Recovering the Password

The logic checks:

1. Exactly one argument passed (`argc == 2`)
2. Password length is `0xc` = **12 characters**
3. Four `strncmp` checks on 3-character chunks:

|Offset|Expected|
|---|---|
|0|`Itz`|
|3|`_0n`|
|6|`Ly_`|
|9|`UD2`|

Concatenating them gives: `Itz_0nLy_UD2`

---

## Flag

```bash
./behindthescenes Itz_0nLy_UD2
```

```
> HTB{Itz_0nLy_UD2}
```

**Flag: `HTB{Itz_0nLy_UD2}`**
