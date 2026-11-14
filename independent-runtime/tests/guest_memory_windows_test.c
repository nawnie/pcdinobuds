#include "guest_memory_windows.h"
#include <stdio.h>
#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x); exit(1); } } while (0)

int main(void) {
    SYSTEM_INFO info; GetSystemInfo(&info);
    size_t size = info.dwAllocationGranularity;
    void *probe = VirtualAlloc(NULL, size, MEM_RESERVE, PAGE_NOACCESS);
    CHECK(probe != NULL);
    uint64_t base = (uint64_t)(uintptr_t)probe;
    CHECK(VirtualFree(probe, 0, MEM_RELEASE));
    YoshiMapping slots[2]; YoshiAddressSpace space = {slots, 0, 2};
    YoshiOwnedRegion owner = {0}, duplicate = {0};
    CHECK(yoshi_region_reserve(&owner, &space, base, size) == YOSHI_REGION_OK);
    CHECK(yoshi_memory_require_identity(&space) == YOSHI_MEMORY_OK);
    void *resolved = NULL;
    CHECK(yoshi_memory_resolve(&space, base + size - 1, 1, YOSHI_MEM_WRITE, &resolved) == YOSHI_MEMORY_OK);
    CHECK(resolved == (unsigned char *)owner.host + size - 1);
    ((unsigned char *)owner.host)[0] = 0x5a;
    CHECK(yoshi_region_reserve(&duplicate, &space, base, size) == YOSHI_REGION_OS_ERROR);
    CHECK(duplicate.host == NULL && space.count == 1 && ((unsigned char *)owner.host)[0] == 0x5a);
    CHECK(yoshi_region_protect(&owner, YOSHI_MEM_READ) == YOSHI_REGION_OK);
    CHECK(yoshi_memory_resolve(&space, base, 1, YOSHI_MEM_WRITE, &resolved) == YOSHI_MEMORY_PERMISSION);
    MEMORY_BASIC_INFORMATION mbi;
    CHECK(VirtualQuery(owner.host, &mbi, sizeof mbi) == sizeof mbi && mbi.Protect == PAGE_READONLY);
    CHECK(yoshi_region_protect(&owner, YOSHI_MEM_WRITE) == YOSHI_REGION_INVALID);
    CHECK(yoshi_region_protect(&owner, 3) == YOSHI_REGION_OK);
    CHECK(((unsigned char *)owner.host)[0] == 0x5a);
    CHECK(yoshi_region_release(&owner) == YOSHI_REGION_OK && space.count == 0 && !owner.host);
    CHECK(yoshi_region_release(&owner) == YOSHI_REGION_INVALID);
    CHECK(VirtualQuery((void *)(uintptr_t)base, &mbi, sizeof mbi) == sizeof mbi && mbi.State == MEM_FREE);
    YoshiAddressSpace no_capacity = {NULL, 0, 0};
    CHECK(yoshi_region_reserve(&owner, &no_capacity, base, size) == YOSHI_REGION_MAPPING_ERROR);
    CHECK(!owner.host && no_capacity.count == 0);
    CHECK(VirtualQuery((void *)(uintptr_t)base, &mbi, sizeof mbi) == sizeof mbi && mbi.State == MEM_FREE);
    puts("owned Windows region tests passed");
}
