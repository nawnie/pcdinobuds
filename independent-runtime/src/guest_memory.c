#include "guest_memory.h"
#include <string.h>
#include <stdint.h>

static int valid_mapping(YoshiMapping m) {
    return m.host && m.size && m.size <= SIZE_MAX && m.base <= UINT64_MAX - m.size &&
           (m.permissions & ~(YOSHI_MEM_READ | YOSHI_MEM_WRITE)) == 0 && m.permissions &&
           (uintptr_t)m.host <= UINTPTR_MAX - (size_t)m.size;
}

YoshiMemoryStatus yoshi_memory_map(YoshiAddressSpace *s, YoshiMapping m) {
    if (!s || s->count > s->capacity || (s->capacity && !s->mappings) ||
        s->count == s->capacity || !valid_mapping(m)) return YOSHI_MEMORY_INVALID;
    for (size_t i = 0; i < s->count; ++i) {
        YoshiMapping old = s->mappings[i];
        if (!valid_mapping(old)) return YOSHI_MEMORY_INVALID;
        if (m.base < old.base + old.size && old.base < m.base + m.size)
            return YOSHI_MEMORY_OVERLAP;
    }
    s->mappings[s->count++] = m;
    return YOSHI_MEMORY_OK;
}

YoshiMemoryStatus yoshi_memory_resolve(const YoshiAddressSpace *s, uint64_t a,
                                       size_t n, unsigned p, void **out) {
    if (!s || !out || !n || !p || (p & ~(YOSHI_MEM_READ | YOSHI_MEM_WRITE)) ||
        s->count > s->capacity || (s->count && !s->mappings)) return YOSHI_MEMORY_INVALID;
    for (size_t i = 0; i < s->count; ++i) {
        YoshiMapping m = s->mappings[i];
        if (!valid_mapping(m)) return YOSHI_MEMORY_INVALID;
        if (a >= m.base && a - m.base < m.size && n <= m.size - (a - m.base)) {
            if ((m.permissions & p) != p) return YOSHI_MEMORY_PERMISSION;
            *out = m.host + (size_t)(a - m.base);
            return YOSHI_MEMORY_OK;
        }
    }
    return YOSHI_MEMORY_UNMAPPED;
}

YoshiMemoryStatus yoshi_memory_require_identity(const YoshiAddressSpace *s) {
    if (!s || !s->count || s->count > s->capacity || !s->mappings) return YOSHI_MEMORY_INVALID;
    for (size_t i = 0; i < s->count; ++i) {
        if (!valid_mapping(s->mappings[i])) return YOSHI_MEMORY_INVALID;
        if ((uint64_t)(uintptr_t)s->mappings[i].host != s->mappings[i].base)
            return YOSHI_MEMORY_NOT_IDENTITY;
    }
    return YOSHI_MEMORY_OK;
}

static int segment_ok(YoshiImageSegment s, uint64_t size) {
    return s.offset <= size && s.size <= size - s.offset &&
           s.bytes_size <= s.size && (!s.bytes_size || s.bytes);
}
static int image_ok(const YoshiModuleImage *m) {
    return m && m->size && m->size <= SIZE_MAX && m->base <= UINT64_MAX - m->size &&
           segment_ok(m->text, m->size) && segment_ok(m->rodata, m->size) &&
           segment_ok(m->data, m->size) &&
           m->text.offset + m->text.size <= m->rodata.offset &&
           m->rodata.offset + m->rodata.size <= m->data.offset &&
           !(m->data.offset & 0xfff);
}

YoshiMemoryStatus yoshi_module_load(YoshiAddressSpace *s, const YoshiModuleImage *m) {
    if (!image_ok(m)) return YOSHI_MEMORY_INVALID;
    void *host;
    YoshiMemoryStatus status = yoshi_memory_resolve(s, m->base, (size_t)m->size, YOSHI_MEM_WRITE, &host);
    if (status != YOSHI_MEMORY_OK) return status;
    YoshiImageSegment segments[] = {m->text, m->rodata, m->data};
    uintptr_t lo = (uintptr_t)host, hi = lo + (size_t)m->size;
    for (size_t i = 0; i < 3; ++i) {
        uintptr_t source = (uintptr_t)segments[i].bytes;
        if (segments[i].bytes_size && (source > UINTPTR_MAX - segments[i].bytes_size ||
            (source < hi && lo < source + segments[i].bytes_size))) return YOSHI_MEMORY_INVALID;
    }
    memset(host, 0, (size_t)m->size);
    for (size_t i = 0; i < 3; ++i)
        if (segments[i].bytes_size) memcpy((unsigned char *)host + segments[i].offset,
                                           segments[i].bytes, segments[i].bytes_size);
    return YOSHI_MEMORY_OK;
}

YoshiMemoryStatus yoshi_module_relocate_relative(YoshiAddressSpace *s, const YoshiModuleImage *m,
                                                  uint64_t offset, int64_t addend) {
    if (!image_ok(m) || offset < m->data.offset || offset > m->size || m->size - offset < 8)
        return YOSHI_MEMORY_INVALID;
    void *host;
    YoshiMemoryStatus status = yoshi_memory_resolve(s, m->base + offset, 8, YOSHI_MEM_WRITE, &host);
    if (status != YOSHI_MEMORY_OK) return status;
    uint64_t value = m->base + (uint64_t)addend;
    for (unsigned i = 0; i < 8; ++i) ((unsigned char *)host)[i] = (unsigned char)(value >> (8 * i));
    return YOSHI_MEMORY_OK;
}
