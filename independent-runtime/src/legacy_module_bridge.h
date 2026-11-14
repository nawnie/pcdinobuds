#pragma once
#include "guest_adapter.h"
#include "dispatch.h"

/* Translate a generated RtModule table without copying guest function tables.
   Source module descriptors and their arrays must remain alive and immutable.
   Input and output descriptor storage must not overlap. On failure output is untouched. */
YoshiGuestStatus yoshi_guest_import_modules(YoshiGuestModule *out, size_t capacity,
                                            const RtModule *in, size_t count);
