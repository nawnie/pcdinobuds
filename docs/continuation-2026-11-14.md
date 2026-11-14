# November 14 engineering checkpoint

## Approach

Build an independent Yoshi host in vertical slices while keeping the existing runtime as a regression reference. Switch2Recomp is useful research, but replacing this work wholesale would discard functioning Yoshi-specific behavior before its compatibility is established. First establish the guest ABI and actual module-table boundary; then implement loader, memory, thread and service contracts before a copied-profile boot. The renderer needs explicit resource ownership per mip level, image generation and write fence.

## Evidence and limits

The adapter milestone passes Debug and Release synthetic tests, including always-on Release checks and compiled CPU ABI assertions. A separate local-only smoke test validates all 2,716 entries of the actual subsdk2 table linked against abort-on-call stubs; none were called. This tests table representation and linkage, not guest execution. It must not be described as an independent game runtime or a completed recompilation. The five lifted module descriptors exclude rtld: the legacy host performs its startup role.

Separate local gameplay experiments reached discovery and pause screens and committed a save. A later isolated reload of that save crashed before gameplay, with exit code 71. Disassembly identifies a null pointer loaded from guest global main+0x96bc1e8 and dereferenced in main+0x389fa4. Subsequent caller analysis places this in a garbage-collection destruction watchdog, with an embedded diagnostic naming GarbageCollection.cpp. The null write appears to be a diagnostic crash path; the unresolved problem is the object that remains unready for destruction. The original save was preserved unchanged. Saving and reloading are therefore not accepted end to end.

The local split-mip candidate fixed inspected terrain mip uploads, but monocle BC3 mip levels 1 and 2 still show corruption. The unresolved boundary is a texture-to-texture copy at frame 2974, fence 13550. Capture source GPU bytes before the copy and destination bytes after it, after refresh and at the draw. Identify resources by image generation as well as address because the source address is later reused.

Source recovery also gained 1,000 randomized differential cases covering entropy-table selection and callback mutations without mismatches. These are bounded semantic checks, not exact recompilation matches; the exact-matched function count did not increase.

These experiments and private evidence remain outside this repository. The legacy source here is the preserved published baseline, not a verified integration of all later local fixes.

## Next acceptance steps

1. Extend the verified generated leaf/table path to required host services and loader relocation contracts, keeping generated data local.
2. Trace the object-readiness virtual call in the garbage-collection watchdog and compare failed and successful saved-profile runs. Resolve the cleanup dependency rather than skipping the diagnostic crash.
3. Resolve the first BC3 source/destination copy divergence with byte comparisons at the recorded fence.
4. Require boot, discovery, pause, save and reload on a copied profile before replacing the playable reference.

## Continued implementation

The independent memory layer validates ranges, permissions and image segments before copying. Its Windows layer reserves exact addresses, tracks ownership, changes whole-region OS protections and releases only its owned reservations. Debug and Release CTest pass all three adapter/memory/Windows tests. Duplicate reservations preserve the existing allocation; failed map registration rolls back the new reservation.

A separate private-input smoke test verified the five ExeFS input hashes against the existing build manifest, compared decompressed bytes with saved analysis images, and loaded all five simultaneously at their actual layout bases. All image comparisons and declared BSS zero checks passed; the harness confirmed release with the OS. It did not apply full relocations, resolve symbols, initialize TLS, call constructors, or execute guest code. Private inputs and generated harness artifacts are not included here.

The instrumented saved-profile replay subsequently reached the saved discovery scene and pause menu. Its original save remained unchanged. Instrumentation, cache state and some observed stick input differed from the failed run, so this is not a crash fix or an end-to-end acceptance claim. The watched global remained zero and the faulting function was not entered, consistent with the later garbage-collection diagnostic finding.

A separate local harness linked all four existing generated subsdk2 objects and their actual table with the adapter. It validated 2,716 entries and executed one pure leaf against six inputs, matching the original ARM64 comparison and return behavior. Unresolved runtime service entry points used abort-on-call stubs, none of which fired. This establishes one actual generated-function path; it does not establish full module execution or service coverage.
