# pcdinobuds

Experimental Windows recompilation research for *Yoshi and the Mysterious Book* (Switch 2, version 1.0.2).

The current direction is a Yoshi-owned host/runtime and renderer, developed in small, testable stages. The inherited DKB/TotK runtime remains a reference and a regression baseline. This repository preserves its Git history and attribution; it is not a finished standalone game port.

## Current work

- `independent-runtime/`: a guest ABI adapter pilot, with table validation and synthetic lookup tests. It still consumes the legacy CPU headers. It does not yet implement an independent loader, services, renderer, or boot.
- `runtime/`, `recomp/`, and existing tools: preserved legacy source baseline. Later local gameplay experiments have not been comprehensively reconciled into this checkout.
- [Engineering checkpoint](docs/continuation-2026-11-14.md): measured results, current failures, and the next implementation steps.
- [Historical build guide](docs/legacy-build-guide.md): instructions for the inherited baseline. The historical status page is not current acceptance evidence.

## Build the adapter

Use CMake, Ninja, and MSYS2 UCRT64 Clang on a 64-bit Windows host:

```powershell
cmake -S independent-runtime -B build/adapter -G Ninja -DCMAKE_C_COMPILER=clang -DYOSHI_RUNTIME_HEADERS="$PWD/runtime"
cmake --build build/adapter
ctest --test-dir build/adapter --output-on-failure
```

See `independent-runtime/README.md` for its ABI dependencies and test limits. Passing adapter tests does not prove game boot or save compatibility.

## Content and attribution

No game containers, keys, extracted files, generated guest code, game binaries, saves, or shader caches belong in Git. Local inputs and generated outputs must stay ignored. Run `python tools/check_content.py --staged` before each commit.

The preserved baseline derives from [DKB Recomp](https://github.com/Lucas-blip-png/dkb-pc-recomp), which builds on [TotK Recomp](https://github.com/valentderah/totk-pc-recomp). The repository retains GPL-3.0-or-later terms, with inherited MIT components keeping their original terms. See [LICENSE](LICENSE), [LICENSE-MIT](LICENSE-MIT), [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md), and [LEGAL.md](LEGAL.md).
