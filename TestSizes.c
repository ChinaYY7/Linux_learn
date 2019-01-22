#include "apue.h"
#include <stdint.h>

int main(void)
{
    printf("         CHAR_BIT: %d\n\n", CHAR_BIT);

    printf("     sizeof(char): %ld\n", sizeof(char));
    printf("    sizeof(short): %ld\n", sizeof(short));
    printf("      sizeof(int): %ld\n", sizeof(int));
    printf("     sizeof(long): %ld\n", sizeof(long));
    printf("sizeof(long long): %ld\n\n", sizeof(long long));

    printf("   sizeof(int8_t): %ld\n", sizeof(int8_t));
    printf("  sizeof(int16_t): %ld\n", sizeof(int16_t));
    printf("  sizeof(int32_t): %ld\n", sizeof(int32_t));
    printf("  sizeof(int64_t): %ld\n\n", sizeof(int64_t));

    printf("  sizeof(uint8_t): %ld\n", sizeof(uint8_t));
    printf(" sizeof(uint16_t): %ld\n", sizeof(uint16_t));
    printf(" sizeof(uint32_t): %ld\n", sizeof(uint32_t));
    printf(" sizeof(uint64_t): %ld\n\n", sizeof(uint64_t));

    printf("    sizeof(float): %ld\n", sizeof(float));
    printf("   sizeof(double): %ld\n", sizeof(double));
}