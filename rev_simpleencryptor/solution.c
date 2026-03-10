#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main() {
    const char* file_name = "flag.enc";
    FILE *f = fopen(file_name, "rb");
    uint32_t seed;
    fread(&seed, 1, 4, f);
    srand(seed);

    fseek(f,0,SEEK_END);
    const long size = ftell(f) - 4;
    fseek(f, 4, SEEK_SET);

    uint8_t *flag = malloc(size);
    fread(flag, 1, size, f);

    for (long i = 0; i<size; i++) {
        const int rand_1 = rand();
        int rand_2 = rand();
        rand_2 = rand_2 & 7;

        if (rand_2 != 0)
            flag[i] = flag[i] >> rand_2 | flag[i] << (8 - rand_2);

        flag[i] ^= (uint8_t)rand_1;
    }
    fwrite(flag, 1, size, stdout);
    putchar('\n');
    free(flag);
    return 0;

}
