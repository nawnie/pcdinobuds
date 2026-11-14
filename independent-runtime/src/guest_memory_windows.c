#include "guest_memory_windows.h"
#include <stdint.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static YoshiMapping *owned_mapping(YoshiOwnedRegion *o, size_t *index) {
    if (!o || !o->host || !o->space || o->space->count > o->space->capacity ||
        (o->space->count && !o->space->mappings)) return NULL;
    for (size_t i = 0; i < o->space->count; ++i) {
        YoshiMapping *m = &o->space->mappings[i];
        if (m->base == o->base && m->size == o->size && m->host == o->host) {
            if (index) *index = i;
            return m;
        }
    }
    return NULL;
}

YoshiRegionStatus yoshi_region_reserve(YoshiOwnedRegion *o, YoshiAddressSpace *s,
                                       uint64_t base, size_t size) {
    SYSTEM_INFO info; GetSystemInfo(&info);
    if (!o || o->host || o->space || o->base || o->size || !s || !base || !size ||
        base > UINTPTR_MAX || base > UINT64_MAX - size ||
        base % info.dwAllocationGranularity || size % info.dwPageSize)
        return YOSHI_REGION_INVALID;
    void *p = VirtualAlloc((void *)(uintptr_t)base, size, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (!p) return YOSHI_REGION_OS_ERROR;
    if ((uintptr_t)p != base) { VirtualFree(p, 0, MEM_RELEASE); return YOSHI_REGION_OS_ERROR; }
    YoshiMapping m = {base, size, p, YOSHI_MEM_READ | YOSHI_MEM_WRITE};
    if (yoshi_memory_map(s, m) != YOSHI_MEMORY_OK) {
        VirtualFree(p, 0, MEM_RELEASE);
        return YOSHI_REGION_MAPPING_ERROR;
    }
    *o = (YoshiOwnedRegion){s, base, size, p};
    return YOSHI_REGION_OK;
}

YoshiRegionStatus yoshi_region_protect(YoshiOwnedRegion *o, unsigned permissions) {
    YoshiMapping *m = owned_mapping(o, NULL);
    if (!m || (permissions != YOSHI_MEM_READ && permissions != (YOSHI_MEM_READ | YOSHI_MEM_WRITE)))
        return YOSHI_REGION_INVALID;
    DWORD old;
    if (!VirtualProtect(o->host, (SIZE_T)o->size,
                        permissions == YOSHI_MEM_READ ? PAGE_READONLY : PAGE_READWRITE, &old))
        return YOSHI_REGION_OS_ERROR;
    m->permissions = permissions;
    return YOSHI_REGION_OK;
}

YoshiRegionStatus yoshi_region_release(YoshiOwnedRegion *o) {
    size_t index;
    if (!owned_mapping(o, &index)) return YOSHI_REGION_INVALID;
    if (!VirtualFree(o->host, 0, MEM_RELEASE)) return YOSHI_REGION_OS_ERROR;
    YoshiAddressSpace *s = o->space;
    for (size_t i = index + 1; i < s->count; ++i) s->mappings[i - 1] = s->mappings[i];
    --s->count;
    *o = (YoshiOwnedRegion){0};
    return YOSHI_REGION_OK;
}
#else
YoshiRegionStatus yoshi_region_reserve(YoshiOwnedRegion *o, YoshiAddressSpace *s, uint64_t b, size_t n) {
    (void)o; (void)s; (void)b; (void)n; return YOSHI_REGION_UNSUPPORTED;
}
YoshiRegionStatus yoshi_region_protect(YoshiOwnedRegion *o, unsigned p) {
    (void)o; (void)p; return YOSHI_REGION_UNSUPPORTED;
}
YoshiRegionStatus yoshi_region_release(YoshiOwnedRegion *o) {
    (void)o; return YOSHI_REGION_UNSUPPORTED;
}
#endif
