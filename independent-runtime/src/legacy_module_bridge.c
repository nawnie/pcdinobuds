#include "legacy_module_bridge.h"
#include <stdint.h>

YoshiGuestStatus yoshi_guest_import_modules(YoshiGuestModule *out, size_t capacity,
                                            const RtModule *in, size_t count) {
    if (count > capacity || (count && (!out || !in))) return YOSHI_GUEST_INVALID_MODULE;
    /* Validate each descriptor and each pair before touching the output. */
    for (size_t i = 0; i < count; ++i) {
        if (in[i].count > SIZE_MAX) return YOSHI_GUEST_INVALID_MODULE;
        YoshiGuestModule candidate = {in[i].name, in[i].base, in[i].size,
                                      in[i].addrs, in[i].fns, (size_t)in[i].count};
        YoshiGuestStatus status = yoshi_guest_validate(&candidate, 1);
        if (status != YOSHI_GUEST_OK) return status;
        for (size_t j = 0; j < i; ++j)
            if (in[i].base < in[j].base + in[j].size &&
                in[j].base < in[i].base + in[i].size) return YOSHI_GUEST_OVERLAP;
    }
    for (size_t i = 0; i < count; ++i)
        out[i] = (YoshiGuestModule){in[i].name, in[i].base, in[i].size,
                                    in[i].addrs, in[i].fns, (size_t)in[i].count};
    return YOSHI_GUEST_OK;
}
