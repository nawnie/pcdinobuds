#include "guest_memory.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); exit(1); } } while (0)

int main(void) {
    unsigned char backing[0x2000], second[16], text[] = {1,2,3}, ro[] = {4,5}, data[] = {6,7};
    YoshiMapping slots[2]; YoshiAddressSpace space = {slots, 0, 2};
    CHECK(yoshi_memory_require_identity(&space) == YOSHI_MEMORY_INVALID);
    YoshiMapping identity_slot; YoshiAddressSpace identity = {&identity_slot, 0, 1};
    CHECK(yoshi_memory_map(&identity, (YoshiMapping){(uint64_t)(uintptr_t)second, sizeof second, second, 3}) == YOSHI_MEMORY_OK);
    CHECK(yoshi_memory_require_identity(&identity) == YOSHI_MEMORY_OK);
    CHECK(yoshi_memory_map(&space, (YoshiMapping){0x100000, sizeof backing, backing, 3}) == YOSHI_MEMORY_OK);
    CHECK(yoshi_memory_map(&space, (YoshiMapping){0x101fff, sizeof second, second, 3}) == YOSHI_MEMORY_OVERLAP);
    CHECK(yoshi_memory_map(&space, (YoshiMapping){UINT64_MAX - 2, 16, second, 3}) == YOSHI_MEMORY_INVALID);
    CHECK(yoshi_memory_map(&space, (YoshiMapping){0x102000, sizeof second, second, YOSHI_MEM_READ}) == YOSHI_MEMORY_OK);
    CHECK(yoshi_memory_require_identity(&space) == YOSHI_MEMORY_NOT_IDENTITY);
    void *p = NULL;
    CHECK(yoshi_memory_resolve(&space, 0x101fff, 1, YOSHI_MEM_READ, &p) == YOSHI_MEMORY_OK && p == backing + 0x1fff);
    CHECK(yoshi_memory_resolve(&space, 0x101fff, 2, YOSHI_MEM_READ, &p) == YOSHI_MEMORY_UNMAPPED);
    CHECK(yoshi_memory_resolve(&space, 0x102000, 1, YOSHI_MEM_WRITE, &p) == YOSHI_MEMORY_PERMISSION);
    CHECK(yoshi_memory_resolve(&space, UINT64_MAX, 8, YOSHI_MEM_READ, &p) == YOSHI_MEMORY_UNMAPPED);
    YoshiModuleImage image = {0x100000, sizeof backing,
        {0, 16, text, sizeof text}, {16, 16, ro, sizeof ro}, {0x1000, 16, data, sizeof data}};
    memset(backing, 0xcc, sizeof backing);
    CHECK(yoshi_module_load(&space, &image) == YOSHI_MEMORY_OK);
    CHECK(backing[0] == 1 && backing[2] == 3 && backing[3] == 0 && backing[16] == 4 &&
          backing[0x1000] == 6 && backing[0x1002] == 0 && backing[0x1fff] == 0);
    YoshiModuleImage bad = image; bad.rodata.offset = 15;
    backing[0] = 0xaa;
    CHECK(yoshi_module_load(&space, &bad) == YOSHI_MEMORY_INVALID && backing[0] == 0xaa);
    bad = image; bad.data.bytes = backing; bad.data.bytes_size = 2;
    CHECK(yoshi_module_load(&space, &bad) == YOSHI_MEMORY_INVALID && backing[0] == 0xaa);
    bad = image; bad.data.offset = 0x1001;
    CHECK(yoshi_module_load(&space, &bad) == YOSHI_MEMORY_INVALID);
    CHECK(yoshi_module_relocate_relative(&space, &image, 0xfff, 0) == YOSHI_MEMORY_INVALID);
    CHECK(yoshi_module_relocate_relative(&space, &image, 0x1ff9, 0) == YOSHI_MEMORY_INVALID);
    CHECK(yoshi_module_relocate_relative(&space, &image, 0x1ff8, -1) == YOSHI_MEMORY_OK);
    const unsigned char expected[] = {0xff,0xff,0x0f,0,0,0,0,0};
    CHECK(memcmp(backing + 0x1ff8, expected, sizeof expected) == 0);
    puts("guest memory tests passed");
}
