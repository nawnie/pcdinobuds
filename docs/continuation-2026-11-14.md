# November 14 engineering checkpoint

## Approach

Build an independent Yoshi host in vertical slices while keeping the existing runtime as a regression reference. Switch2Recomp is useful research, but replacing this work wholesale would discard functioning Yoshi-specific behavior before its compatibility is established. First establish the guest ABI and actual module-table boundary; then implement loader, memory, thread and service contracts before a copied-profile boot. The renderer needs explicit resource ownership per mip level, image generation and write fence.

## Evidence and limits

The adapter milestone passes Debug and Release synthetic tests, including always-on Release checks and compiled CPU ABI assertions. A separate local-only smoke test validates all 2,716 entries of the actual subsdk2 table linked against abort-on-call stubs; none were called. This tests table representation and linkage, not guest execution. It must not be described as an independent game runtime or a completed recompilation. The five lifted module descriptors exclude rtld: the legacy host performs its startup role.

Separate local gameplay experiments reached discovery and pause screens and committed a save. A later isolated reload of that save crashed before gameplay, with exit code 71. Disassembly identifies a null pointer loaded from guest global main+0x96bc1e8 and dereferenced in main+0x389fa4. Its initialization failure is not yet explained. The original save was preserved unchanged. Saving and reloading are therefore not accepted end to end.

The local split-mip candidate fixed inspected terrain mip uploads, but monocle BC3 mip levels 1 and 2 still show corruption. The unresolved boundary is a texture-to-texture copy at frame 2974, fence 13550. Capture source GPU bytes before the copy and destination bytes after it, after refresh and at the draw. Identify resources by image generation as well as address because the source address is later reused.

Source recovery also gained 1,000 randomized differential cases covering entropy-table selection and callback mutations without mismatches. These are bounded semantic checks, not exact recompilation matches; the exact-matched function count did not increase.

These experiments and private evidence remain outside this repository. The legacy source here is the preserved published baseline, not a verified integration of all later local fixes.

## Next acceptance steps

1. Validate real generated module-table linkage locally without publishing generated data; distinguish stub-backed table checks from actual guest execution.
2. Trace writes to the null global and compare the same executable with fresh and copied saved profiles. Fix the producer or initialization contract, rather than skipping the faulting store.
3. Resolve the first BC3 source/destination copy divergence with byte comparisons at the recorded fence.
4. Require boot, discovery, pause, save and reload on a copied profile before replacing the playable reference.
