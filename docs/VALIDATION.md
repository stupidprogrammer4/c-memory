# Validation status

Recorded October 19, 2026. The declared initial modules are implemented; the
repository stays private until independent CI and release checks finish.

## Completed locally

Environment: Linux x86-64, GCC 15.2.0, CMake 3.31.6.

- Debug: 9 CTest cases, including a fresh installed-package consumer.
- Release with warnings as errors: 9 CTest cases.
- Shared-library Release build: 9 CTest cases.
- AddressSanitizer, LeakSanitizer and UndefinedBehaviorSanitizer: 8 cases;
  installed-package validation is intentionally performed in ordinary builds.
- GCC `-fanalyzer -Werror`: all library sources compiled without diagnostics.
- Strict ISO C17 build with conversion, prototype, pointer, alignment and
  format-string warnings treated as errors: all targets and 9 CTest cases pass.
- clang-format 23.1.3 checks pass; per-project compilation databases are exported
  for editor analysis without suppressing diagnostics.
- Seeded stress: 30,000 operations each through pool, slab and tracker adapters;
  payload integrity, alignment, uniqueness, counts and final cleanup checked.
- Injected backing-allocation failure at every slab initialization stage;
  all earlier allocations released on failure.
- CPack archive built, extracted to a relocated prefix containing spaces, and
  consumed by a separate CMake application successfully.
- Five benchmark samples, with raw measurements and limitations in `BENCHMARKS.md`.

TDD cycles introduced allocator/arena and pool/slab/tracker tests before their
implementations. Additional failing tests identified and corrected output-pointer
cleanup in the raw allocator callbacks before the final suite passed.

## Required before publication

- Independent GCC/Clang CI, including shared libraries and sanitizers. The first
  hosted workflow could not schedule its jobs; no test step ran.
- Verify the final release archive corresponds to the release commit, publish
  release notes and SHA-256 checksums, then make the completed project public.

These checks are evidence for the tested contracts, not a proof that arbitrary
caller misuse or every possible memory error can be prevented.
