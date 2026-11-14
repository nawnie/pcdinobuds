#pragma once
#include <stddef.h>
#include <stdint.h>

typedef enum YoshiMemoryStatus {
    YOSHI_MEMORY_OK, YOSHI_MEMORY_INVALID, YOSHI_MEMORY_OVERLAP,
    YOSHI_MEMORY_UNMAPPED, YOSHI_MEMORY_PERMISSION, YOSHI_MEMORY_NOT_IDENTITY
} YoshiMemoryStatus;
enum { YOSHI_MEM_READ = 1, YOSHI_MEM_WRITE = 2 };
typedef struct YoshiMapping {
    uint64_t base, size;
    unsigned char *host;
    unsigned permissions;
} YoshiMapping;
typedef struct YoshiAddressSpace {
    YoshiMapping *mappings;
    size_t count, capacity;
} YoshiAddressSpace;

/* Storage and backing are owned by the caller and stay alive while mapped.
   Registration does not allocate, reserve or protect host memory. */
YoshiMemoryStatus yoshi_memory_map(YoshiAddressSpace *space, YoshiMapping mapping);
YoshiMemoryStatus yoshi_memory_resolve(const YoshiAddressSpace *space, uint64_t address,
                                       size_t length, unsigned permissions, void **out);
/* Generated code directly dereferences guest addresses; this check is mandatory
   before using a registered range with it. Buffer-backed tests are not executable. */
YoshiMemoryStatus yoshi_memory_require_identity(const YoshiAddressSpace *space);

typedef struct YoshiImageSegment {
    uint64_t offset, size;
    const unsigned char *bytes;
    size_t bytes_size;
} YoshiImageSegment;
/* For data, size covers initialized data plus declared BSS; bytes_size covers
   only initialized data. Image alignment padding is outside the data segment. */
typedef struct YoshiModuleImage {
    uint64_t base, size;
    YoshiImageSegment text, rodata, data;
} YoshiModuleImage;

/* Caller supplies a full writable mapping and disjoint input buffers. The image
   is zero-filled then segments are copied, only after all checks succeed. */
YoshiMemoryStatus yoshi_module_load(YoshiAddressSpace *space, const YoshiModuleImage *image);
/* Store one already-decoded AArch64 RELATIVE relocation wholly inside data,
   including declared BSS but excluding image alignment padding, little-endian.
   Arithmetic follows 64-bit unsigned wraparound; no symbol lookup is implied. */
YoshiMemoryStatus yoshi_module_relocate_relative(YoshiAddressSpace *space,
                                                  const YoshiModuleImage *image,
                                                  uint64_t offset, int64_t addend);
