# Yoshi independent runtime: guest adapter pilot

This isolated target defines a Yoshi-owned function-table validator, lookup and
call interface. The bridge imports the existing generated `RtModule` descriptors
without copying their tables. It consumes the existing `cpu.h` and `dispatch.h`
as a temporary ABI dependency, so it is not yet an independent bootable engine.
The existing headers retain their own license and attribution; no upstream code
was copied into this workspace.

Build with UCRT64 Clang and Ninja:

```
cmake -S . -B build -G Ninja -DCMAKE_C_COMPILER=C:/msys64/ucrt64/bin/clang.exe -DYOSHI_RUNTIME_HEADERS="../runtime"
cmake --build build
ctest --test-dir build --output-on-failure
```

Validation and lookup require module descriptors and arrays to remain immutable
and alive while guest code runs. Lookup accepts only exact function starts.
The call helper is raw: it does not establish thread, floating point, dispatch,
or service state. Import requires nonoverlapping input and output descriptors
and leaves output untouched on failure. The pilot requires a 64-bit host and
GNU-compatible Clang with the existing runtime headers.
These tests use synthetic functions with the real `Cpu`/`GuestFn` types. They do
not establish the ABI of generated instructions, guest memory, host calls,
floating point state, exceptions, TLS, loader or service behavior. No game data,
generated guest objects, graphics runtime, boot or gameplay are linked or tested.

The observed local generated `modules.c` exposes five lifted
modules: main, subsdk0, subsdk1, subsdk2 and sdk. It lists rtld in ExeFS hashes
but not in `rt_linked_modules`. `recomp/layout.py` explicitly excludes
rtld because the host links the modules; `runtime/loader.c` loads the
generated layout and `runtime/boot.c` supplies host callbacks for rtld's
startup role and invokes each loaded module's DT_INIT. These are source findings,
not proof the replacement implements that behavior. A local-only smoke links the
actual subsdk2 table with 2,716 abort-on-call stubs, validates its entries and
checks first and last lookups without executing a stub. This establishes table
representation and linkage only; guest functions and the other four real tables
remain untested. Generated data and stubs must remain outside public source.

The test pins all Cpu fields' offsets, size and alignment for a 64-bit host.
The real generated subsdk0 chunk includes `cpu.h` and declares `void f_...(Cpu*)`.
Next link real generated functions in a bounded harness and resolve their host
service dependencies; preserve the installed executable and active checkout.
