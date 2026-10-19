# Validation status

Recorded October 19, 2026 for version 0.1.0. The declared initial modules are
implemented. Local validation replaces hosted CI for this release by the
maintainer's decision; the workflow is retained for manual execution.

## Completed locally

Environment: Linux x86-64, GCC 15.2.0, Clang 20.1.8, CMake 3.31.6.

- Debug: 10 CTest cases per compiler, including a fresh installed-package consumer.
- Release with warnings as errors: 10 CTest cases per compiler.
- Shared-library Release build: 10 CTest cases per compiler.
- AddressSanitizer, LeakSanitizer and UndefinedBehaviorSanitizer: 9 cases per compiler;
  installed-package validation is intentionally performed in ordinary builds.
- GCC `-fanalyzer -Werror` and Clang's static analyzer: all library sources
  checked without diagnostics.
- Strict ISO C17 build with conversion, prototype, pointer, alignment and
  format-string warnings treated as errors: all targets and behavior tests pass.
- clang-format 23.1.3 checks pass; per-project compilation databases are exported
  for editor analysis without suppressing diagnostics.
- Every public header parses independently in C17 and C++17 with both GCC and
  Clang, with warnings and pedantic diagnostics treated as errors.
- Seeded stress: 30,000 operations each through pool, slab and tracker adapters;
  payload integrity, alignment, uniqueness, counts and final cleanup checked.
- Injected backing-allocation failure at every slab initialization stage;
  all earlier allocations released on failure.
- Contract tests cover default/extended alignment, a callback returning no
  allocation, zero-size resize, live-block reset, failed init preserving state,
  overflow rollback, tracker capacity exhaustion and successful tracked resize.
- CPack archive built, extracted to a relocated prefix containing spaces, and
  consumed by a separate CMake application successfully.
- Five benchmark samples, with raw measurements and limitations in `BENCHMARKS.md`.

TDD cycles introduced allocator/arena and pool/slab/tracker tests before their
implementations. Additional failing tests identified and corrected output-pointer
cleanup in the raw allocator callbacks before the final suite passed.
Clang release validation also caught an enum-to-floating arithmetic warning in
the benchmark; explicit conversion of both operation counters fixed the cause.

## Release validation policy

The local matrix comprises 78 successful CTest executions across both compilers,
plus static analysis and benchmark smoke runs. `scripts/validate.sh` reproduces
the matrix without GitHub Actions. Release validation additionally builds from
an exported clean source tree, consumes relocated static/shared packages, checks
the source installer, and verifies the archives against their SHA-256 manifest.
The published tag identifies the source used for packaging. The release makes
no claim that hosted CI executed.

These checks are evidence for the tested contracts, not a proof that arbitrary
caller misuse or every possible memory error can be prevented.
