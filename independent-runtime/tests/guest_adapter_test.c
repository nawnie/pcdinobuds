#include "guest_adapter.h"
#include "legacy_module_bridge.h"
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#define CHECK(expr) do { if (!(expr)) { fprintf(stderr, "CHECK failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); exit(1); } } while (0)

/* Pin the 64-bit host ABI the current generated C and adapter share. */
_Static_assert(sizeof(void *) == 8, "64-bit host required");
_Static_assert(sizeof(Cpu) == 1024 && _Alignof(Cpu) == 16, "Cpu size/alignment changed");
_Static_assert(offsetof(Cpu, x) == 0 && offsetof(Cpu, sp) == 256, "register layout changed");
_Static_assert(offsetof(Cpu, pc) == 264 && offsetof(Cpu, nzcv) == 272, "control layout changed");
_Static_assert(offsetof(Cpu, tpidr) == 280 && offsetof(Cpu, tpidrro) == 288, "TLS layout changed");
_Static_assert(offsetof(Cpu, excl_addr) == 296 && offsetof(Cpu, excl_val) == 304, "exclusive layout changed");
_Static_assert(offsetof(Cpu, thread) == 312 && offsetof(Cpu, fpcr) == 320 && offsetof(Cpu, fpsr) == 328, "host/FP layout changed");
_Static_assert(offsetof(Cpu, excl_val_hi) == 336 && offsetof(Cpu, v) == 512, "SIMD layout changed");

static void guest_a(Cpu *cpu) { cpu->x[0] += 7; cpu->pc = cpu->x[30]; }
static void guest_b(Cpu *cpu) { cpu->x[1] = cpu->sp; }

int main(void) {
    const uint64_t addresses[] = {0x1000, 0x1008};
    const GuestFn functions[] = {guest_a, guest_b};
    YoshiGuestModule m[] = {{"test", 0x1000, 0x100, addresses, functions, 2}};
    RtModule source[] = {{"test", 0x1000, 0x100, addresses, functions, 2}};
    YoshiGuestModule imported[1];
    CHECK(yoshi_guest_import_modules(imported, 1, source, 1) == YOSHI_GUEST_OK);
    CHECK(imported[0].addresses == addresses && imported[0].functions == functions);
    CHECK(yoshi_guest_lookup(imported, 1, 0x1008) == guest_b);
    CHECK(yoshi_guest_import_modules(imported, 0, source, 1) == YOSHI_GUEST_INVALID_MODULE);
    CHECK(yoshi_guest_validate(m, 1) == YOSHI_GUEST_OK);
    CHECK(yoshi_guest_validate(NULL, 1) == YOSHI_GUEST_INVALID_MODULE);
    CHECK(yoshi_guest_lookup(m, 1, 0x1004) == NULL);
    CHECK(yoshi_guest_lookup(m, 1, 0x1100) == NULL);
    Cpu cpu = {0}; cpu.pc = 0x1000; cpu.x[0] = 4; cpu.x[30] = 0x2220;
    CHECK(yoshi_guest_call(m, 1, &cpu) == YOSHI_GUEST_OK);
    CHECK(cpu.x[0] == 11 && cpu.pc == 0x2220);
    CHECK(yoshi_guest_call(m, 1, &cpu) == YOSHI_GUEST_NOT_FOUND);
    cpu.pc = 0x1008; cpu.sp = 0x4400;
    CHECK(yoshi_guest_call(m, 1, &cpu) == YOSHI_GUEST_OK && cpu.x[1] == 0x4400);
    YoshiGuestModule overlap[] = {m[0], m[0]};
    CHECK(yoshi_guest_validate(overlap, 2) == YOSHI_GUEST_OVERLAP);
    RtModule invalid[] = {source[0], source[0]};
    YoshiGuestModule untouched[2] = {{"sentinel", 42, 1, NULL, NULL, 0}, {0}};
    CHECK(yoshi_guest_import_modules(untouched, 2, invalid, 2) == YOSHI_GUEST_OVERLAP);
    CHECK(untouched[0].base == 42 && untouched[1].name == NULL);
    const uint64_t bad[] = {0x1008, 0x1008}; m[0].addresses = bad;
    CHECK(yoshi_guest_validate(m, 1) == YOSHI_GUEST_INVALID_ENTRY);
    m[0].base = UINT64_MAX - 3; m[0].size = 8;
    CHECK(yoshi_guest_validate(m, 1) == YOSHI_GUEST_INVALID_MODULE);
    puts("guest adapter tests passed");
}
