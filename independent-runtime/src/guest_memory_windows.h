#pragma once
#include "guest_memory.h"

/* One owner per reservation. Initialize to zero; do not copy a live owner.
   The address space and owner must not be mutated concurrently. */
typedef struct YoshiOwnedRegion {
    YoshiAddressSpace *space;
    uint64_t base, size;
    void *host;
} YoshiOwnedRegion;

typedef enum YoshiRegionStatus {
    YOSHI_REGION_OK, YOSHI_REGION_INVALID, YOSHI_REGION_OS_ERROR,
    YOSHI_REGION_MAPPING_ERROR, YOSHI_REGION_UNSUPPORTED
} YoshiRegionStatus;

/* Base must be allocation-granularity aligned; size must be page aligned.
   Reserves/commits RW at exactly base and registers one identity mapping. */
YoshiRegionStatus yoshi_region_reserve(YoshiOwnedRegion *owner, YoshiAddressSpace *space,
                                       uint64_t base, size_t size);
/* Changes permissions for the entire owned region, updating metadata only after
   the OS succeeds. Supported permissions are read and read/write. */
YoshiRegionStatus yoshi_region_protect(YoshiOwnedRegion *owner, unsigned permissions);
/* Releases only the exact reservation owned by this object and unregisters it. */
YoshiRegionStatus yoshi_region_release(YoshiOwnedRegion *owner);
