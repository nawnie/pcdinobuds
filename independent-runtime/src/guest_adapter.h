#pragma once
#include <stddef.h>
#include <stdint.h>
#include "cpu.h"

/* The generated guest code owns Cpu and GuestFn; this adapter owns lookup. */
typedef struct YoshiGuestModule {
    const char *name;
    uint64_t base, size;
    const uint64_t *addresses;
    const GuestFn *functions;
    size_t count;
} YoshiGuestModule;

typedef enum YoshiGuestStatus {
    YOSHI_GUEST_OK, YOSHI_GUEST_INVALID_MODULE, YOSHI_GUEST_OVERLAP,
    YOSHI_GUEST_INVALID_ENTRY, YOSHI_GUEST_NOT_FOUND
} YoshiGuestStatus;

YoshiGuestStatus yoshi_guest_validate(const YoshiGuestModule *modules, size_t count);
/* Lookup/call require successfully validated, immutable, live tables. The call is
   raw: it does not establish runtime thread, FP, dispatch, or service state. */
GuestFn yoshi_guest_lookup(const YoshiGuestModule *modules, size_t count, uint64_t pc);
YoshiGuestStatus yoshi_guest_call(const YoshiGuestModule *modules, size_t count, Cpu *cpu);
