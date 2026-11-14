#include "guest_adapter.h"
#include <stdint.h>
#include <stddef.h>

/* Generated guest objects and the adapter must share this 64-bit Cpu layout. */
_Static_assert(sizeof(void *) == 8, "64-bit host required");
_Static_assert(sizeof(Cpu) == 1024 && _Alignof(Cpu) == 16, "Cpu size/alignment changed");
_Static_assert(offsetof(Cpu, x) == 0 && offsetof(Cpu, sp) == 256, "register layout changed");
_Static_assert(offsetof(Cpu, pc) == 264 && offsetof(Cpu, nzcv) == 272, "control layout changed");
_Static_assert(offsetof(Cpu, tpidr) == 280 && offsetof(Cpu, tpidrro) == 288, "TLS layout changed");
_Static_assert(offsetof(Cpu, excl_addr) == 296 && offsetof(Cpu, excl_val) == 304, "exclusive layout changed");
_Static_assert(offsetof(Cpu, thread) == 312 && offsetof(Cpu, fpcr) == 320 && offsetof(Cpu, fpsr) == 328, "host/FP layout changed");
_Static_assert(offsetof(Cpu, excl_val_hi) == 336 && offsetof(Cpu, v) == 512, "SIMD layout changed");

YoshiGuestStatus yoshi_guest_validate(const YoshiGuestModule *m, size_t count) {
    if (count && !m) return YOSHI_GUEST_INVALID_MODULE;
    for (size_t i = 0; i < count; ++i) {
        if (!m[i].name || !m[i].size || m[i].size > UINT64_MAX - m[i].base ||
            (m[i].count && (!m[i].addresses || !m[i].functions)))
            return YOSHI_GUEST_INVALID_MODULE;
        uint64_t end = m[i].base + m[i].size;
        for (size_t j = 0; j < i; ++j)
            if (m[i].base < m[j].base + m[j].size && m[j].base < end)
                return YOSHI_GUEST_OVERLAP;
        for (size_t j = 0; j < m[i].count; ++j) {
            uint64_t pc = m[i].addresses[j];
            if (pc < m[i].base || pc >= end || (pc & 3) || !m[i].functions[j] ||
                (j && pc <= m[i].addresses[j - 1])) return YOSHI_GUEST_INVALID_ENTRY;
        }
    }
    return YOSHI_GUEST_OK;
}

/* Caller validates immutable tables before publishing them to guest threads. */
GuestFn yoshi_guest_lookup(const YoshiGuestModule *m, size_t count, uint64_t pc) {
    for (size_t i = 0; i < count; ++i) {
        if (pc < m[i].base || pc - m[i].base >= m[i].size) continue;
        size_t lo = 0, hi = m[i].count;
        while (lo < hi) {
            size_t mid = lo + (hi - lo) / 2;
            if (m[i].addresses[mid] < pc) lo = mid + 1; else hi = mid;
        }
        return lo < m[i].count && m[i].addresses[lo] == pc ? m[i].functions[lo] : NULL;
    }
    return NULL;
}

YoshiGuestStatus yoshi_guest_call(const YoshiGuestModule *m, size_t count, Cpu *cpu) {
    if (!cpu) return YOSHI_GUEST_INVALID_ENTRY;
    GuestFn fn = yoshi_guest_lookup(m, count, cpu->pc);
    if (!fn) return YOSHI_GUEST_NOT_FOUND;
    fn(cpu);
    return YOSHI_GUEST_OK;
}
