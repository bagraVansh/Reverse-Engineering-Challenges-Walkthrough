# Simple Encryptor — HTB Reversing Challenge Writeup

## Challenge Overview

The challenge provided a `Simple_Encryptor.zip` archive which, upon extraction, yields two files:

- `encrypt` — the binary used to encrypt the flag; our target for reverse engineering.
- `flag.enc` — the flag in its encrypted form.

---

## Binary Analysis

### Initial Triage

Running the binary directly results in a segmentation fault, which tells us it expects a file named `flag` to be present in the working directory:

```console
~/CTF/Reversing/Challenge1/rev_simpleencryptor$ ./encrypt
Segmentation fault (core dumped)
```

Checking the file type confirms it is a standard 64-bit ELF executable:

```console
~/CTF/Reversing/Challenge1/rev_simpleencryptor$ file encrypt
encrypt: ELF 64-bit LSB pie executable, x86-64, version 1 (SYSV),
dynamically linked, interpreter /lib64/ld-linux-x86-64.so.2,
BuildID[sha1]=0bddc0a794eca6f6e2e9dac0b6190b62f07c4c75, for GNU/Linux 3.2.0,
not stripped
```

The binary is **not stripped**, which means symbol names are preserved — a significant advantage when reverse engineering.

### Reverse Engineering with Ghidra

Loading the binary into Ghidra and renaming variables for clarity, the decompiled `main` function is as follows:

```c
undefined8 main(void)
{
  int rand_val1;
  time_t curr_time;
  long in_FS_OFFSET;
  uint seed_value;
  uint rand_val2;
  long i;
  FILE *file_ptr1;
  size_t fileLenInBytes;
  void *data_memory;
  FILE *file_ptr2;
  long local_10;

  local_10 = *(long *)(in_FS_OFFSET + 40);
  file_ptr1 = fopen("flag", "rb");
  fseek(file_ptr1, 0, 2);
  fileLenInBytes = ftell(file_ptr1);
  fseek(file_ptr1, 0, 0);
  data_memory = malloc(fileLenInBytes);
  fread(data_memory, fileLenInBytes, 1, file_ptr1);
  fclose(file_ptr1);

  curr_time = time((time_t *)0x0);
  seed_value = (uint)curr_time;
  srand(seed_value);

  for (i = 0; i < (long)fileLenInBytes; i = i + 1) {
    rand_val1 = rand();
    *(byte *)((long)data_memory + i) =
        *(byte *)((long)data_memory + i) ^ (byte)rand_val1;

    rand_val2 = rand();
    rand_val2 = rand_val2 & 7;
    *(byte *)((long)data_memory + i) =
        *(byte *)((long)data_memory + i) << (sbyte)rand_val2 |
        *(byte *)((long)data_memory + i) >> 8 - (sbyte)rand_val2;
  }

  file_ptr2 = fopen("flag.enc", "wb");
  fwrite(&seed_value, 1, 4, file_ptr2);
  fwrite(data_memory, 1, fileLenInBytes, file_ptr2);
  fclose(file_ptr2);

  if (local_10 != *(long *)(in_FS_OFFSET + 40)) {
    __stack_chk_fail();
  }
  return 0;
}
```

### Understanding the Encryption Logic

The encryption applies two operations to each byte of the flag in sequence:

**Step 1 — XOR:** Each byte is XORed with the least significant byte of `rand()`.

```c
byte[i] ^= (byte) rand();
```

**Step 2 — Bit Rotation:** A second call to `rand()` produces a rotation amount (masked to 3 bits, so the range is 0–7). The byte is then left-rotated by that amount.

```c
rot = rand() & 7;
byte[i] = (byte[i] << rot) | (byte[i] >> (8 - rot));
```

Crucially, the PRNG seed is derived from the Unix timestamp at the time of encryption (`time(NULL)`), and this seed is **saved as the first 4 bytes of `flag.enc`**. This is the critical detail that makes decryption possible — we don't need to brute-force the seed.

### Structure of `flag.enc`

| Bytes | Content |
|---|---|
| `[0..3]` | 32-bit PRNG seed (little-endian) |
| `[4..]` | Encrypted flag bytes |

---

## Solution

Since the seed is embedded in `flag.enc`, we can reconstruct the exact same PRNG sequence used during encryption. Decryption simply means reversing the two operations in reverse order:

1. **Reverse the bit rotation** by right-rotating each byte by the same amount.
2. **Reverse the XOR** by XORing again with the same `rand()` value (XOR is its own inverse).

### Decryption Script (C)

```c
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main() {
    const char *file_name = "flag.enc";
    FILE *f = fopen(file_name, "rb");

    // Read the seed from the first 4 bytes
    uint32_t seed;
    fread(&seed, 1, 4, f);
    srand(seed);

    // Determine the size of the encrypted flag data
    fseek(f, 0, SEEK_END);
    const long size = ftell(f) - 4;
    fseek(f, 4, SEEK_SET);

    uint8_t *flag = malloc(size);
    fread(flag, 1, size, f);
    fclose(f);

    for (long i = 0; i < size; i++) {
        const int rand_1 = rand();
        int rand_2 = rand();
        rand_2 = rand_2 & 7;

        // Reverse Step 2: undo left-rotation by right-rotating
        if (rand_2 != 0)
            flag[i] = flag[i] >> rand_2 | flag[i] << (8 - rand_2);

        // Reverse Step 1: undo XOR
        flag[i] ^= (uint8_t)rand_1;
    }

    fwrite(flag, 1, size, stdout);
    putchar('\n');
    free(flag);
    return 0;
}
```

### How It Works

The script mirrors the encryption process exactly, but in reverse order. Since the PRNG state is deterministic given the same seed, calling `rand()` in the same sequence produces identical values. The two operations are then undone as follows:

- A left-rotation by `n` bits is reversed by a right-rotation by `n` bits.
- XOR is a self-inverse operation: `x ^ k ^ k == x`.

Compiling and running the script against `flag.enc` recovers the original flag.

---

## Key Takeaway

The vulnerability in this encryption scheme is the storage of the PRNG seed alongside the ciphertext. A time-based seed is already weak due to its limited entropy, but even if it were stronger, embedding it in the output file entirely eliminates any security it might have provided.