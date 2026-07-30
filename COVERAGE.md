# Coverage-guided testing

How to find and close the test coverage gaps of a library that carry the most risk.
The goal is not a higher percentage; it is finding the untested paths where a defect
is most likely to be hiding. Two of the three runs of this process so far turned up
real production bugs.

The process works one library at a time. Do not merge profiles from several test
binaries: each library is measured by its own test suite, so the numbers answer
"what does this library's suite verify?" rather than "what does the whole codebase
happen to execute?".

## 1. Set up a coverage build

Configure a separate build directory with LLVM source-based coverage enabled. Mirror
the settings of your normal build directory (compiler, generator, Qt prefix path),
but turn the sanitizers off — they slow the run down and add nothing here.

```bash
cmake -S . -B build-coverage -G Ninja \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_PREFIX_PATH=<your Qt prefix, as in build/CMakeCache.txt> \
  -DTB_ENABLE_CCACHE=1 -DTB_ENABLE_PCH=1 \
  -DTB_ENABLE_UBSAN=0 -DTB_ENABLE_ASAN=0 -DTB_ENABLE_TSAN=0 \
  -DTB_ENABLE_LCOV=1
```

If an existing `build-coverage` fails to reconfigure, delete it and start over —
a stale `CMAKE_TOOLCHAIN_FILE` pointing at a removed vcpkg checkout is the usual
cause.

## 2. Produce a profile for one library

```bash
LIB=TbBaseLib
cd build-coverage
ninja ${LIB}Test
LLVM_PROFILE_FILE=$PWD/cov.profraw ./lib/$LIB/test/${LIB}Test
xcrun llvm-profdata merge -sparse cov.profraw -o cov.profdata
```

The summary, to see where you are starting from:

```bash
xcrun llvm-cov report -instr-profile=cov.profdata \
  lib/$LIB/test/${LIB}Test ../lib/$LIB/src ../lib/$LIB/include
```

Watch the **branch** column, not just lines. Once line coverage is reasonable, the
remaining defects live in conditions that only ever went one way.

## 3. Rank the gaps

For a small library the report is short enough to read directly. For a large one
(TbUiLib has ~3500 functions with gaps) sorting by uncovered line count is useless —
it puts long stretches of UI plumbing above short, dense, widely used logic. Export
the JSON and score the functions instead:

```bash
xcrun llvm-cov export -instr-profile=cov.profdata \
  lib/$LIB/test/${LIB}Test ../lib/$LIB/src ../lib/$LIB/include > cov.json
```

A throwaway script over that JSON is enough. The scoring that worked:

    risk = volume x reach x error_proneness x testability

- **volume** — uncovered regions plus half-taken branches. A proxy for untested
  branching, not for size, so a dense 20-line function outranks 200 lines of
  straight-line setup.
- **reach** — how many production files reference the symbol. Measure it on the
  *class* for member functions: names like `move`, `data` and `size` collide with
  the standard library and produce nonsense fan-in otherwise.
- **error_proneness** — density of `Result<`, `throw`, casts, loops and indexing,
  with bonuses for floating-point maths and for path/parsing code.
- **testability** — free functions and `*Utils` units score up; painting, widget
  construction and lifecycle methods score down. A cheap test pinning real logic is
  worth more than an expensive one pinning layout.

Two things to get right when writing that script:

- Group records by source function, so template instantiations and repeated inline
  emissions share an entry and a region counts as covered if *any* instantiation
  covered it. Otherwise one cold instantiation of a common template floods the top
  of the list.
- Demangle with `xcrun llvm-cxxfilt -n`. Without `-n` it strips the leading
  underscore, fails to parse every name, and returns them unchanged — which then
  silently zeroes the fan-in measurement.

Also diff the library's `src/*.cpp` against the filenames present in the export, and
report the difference: those files are invisible rather than uncovered (see the
pitfalls below).

Rank per function. A per-file aggregate rewards sheer size and will put the largest
UI file on top regardless of risk; it is only useful for spotting units with no
tests at all.

## 4. Work through the list

One compilation unit at a time, and stop after each for review and a commit. Per unit:

1. Read the ranked entries for it, then read the code. **Verify the gap is real
   before writing anything** — a good fraction of what the tool reports is
   measurement artifact (see below).
2. For branch gaps, get the specific uncovered branches rather than guessing:

   ```bash
   xcrun llvm-cov export -instr-profile=cov.profdata lib/$LIB/test/${LIB}Test \
     ../lib/$LIB/src/Foo.cpp | python3 -c "
   import json,sys
   d=json.load(sys.stdin)['data'][0]
   for f in d['files']:
       for b in f['branches']:
           if b[4]==0 or b[5]==0:
               print('line', b[0], 'true=', b[4], 'false=', b[5])"
   ```

   `true=0` or `false=0` names the exact condition that never went that way, which
   usually tells you the test case to write.
3. Write the tests following the conventions in `AGENTS.md` (one test case per
   class, one section per member function; for free functions one test case per
   file with a section per function).
4. Build and run in the normal build directory, format the changed files with
   clang-format, then rebuild in `build-coverage`, rerun, and re-measure to confirm
   the gap actually closed.
5. If the unit is foundational, run the dependent suites too — `KdLibTest`,
   `TbBaseLibTest`, `TbElLibTest`, `TbFsLibTest`, `TbMdlLibTest`. `TbMdlLibTest`
   copies its fixtures in a `POST_BUILD` step, so editing a file under
   `app/TrenchBroom/resources/games` does not refresh the copy unless the test
   target relinks. Delete the binary and rebuild, or you will debug a fix that
   already works.

## 5. Hazards when writing the tests

- **Build-mode-dependent code.** `Logger::log` drops debug messages under `NDEBUG`;
  `PreferenceManager::shouldSaveInstantly` returns a different value on Apple
  platforms. A test asserting the local behaviour passes here and fails in CI. Guard
  the assertion with the same `#ifdef` the production code uses.
- **Global state.** Anything reached through a singleton needs an RAII guard that
  tears it down, so a failing assertion cannot leak state into later test cases.
- **Non-deterministic output.** If a function joins values from an unordered
  container, its output order is unspecified and an exact-match assertion will be
  flaky. Prefer fixing the production code to be deterministic over writing a test
  that tolerates both orders.
- **Re-entrancy.** Deferred add/remove machinery (`Notifier`) only runs when a
  callback mutates the container mid-notification. Nothing exercises it by accident.
- **A test that does not call what its name says.** `tst_Expression.cpp` had a
  `SECTION("tryEvaluate")` that called the file's `evaluate` helper, so
  `ExpressionNode::tryEvaluate` sat at 0% while looking tested. When the report
  says a function is uncovered but a section appears to name it, read the section.
- **Code the grammar cannot reach.** EL accepts `..` only inside a subscript, so no
  source expression can put a `Range` value on either side of a comparison — that
  arm of `evaluateCompare` is reachable only by injecting the value through a
  variable. Before writing an unreachable test, work out how a real caller would
  get there; if none can, say so instead of contorting the test.

## 6. When a test finds a bug

This happens, and it is the point of the exercise. Do not encode the buggy
behaviour as the expected result.

1. Confirm the defect with a minimal standalone check.
2. Assess the blast radius — grep for the call sites and say plainly whether it is
   biting today or is a latent trap. Grep the shipped resources too, not just the
   code: a parser bug can have data depending on it.
3. Report it with the proposed fix and **ask before changing production code**. The
   coverage task is to add tests; changing a foundational primitive's semantics is a
   separate decision.
4. Keep the suite green in the meantime: leave out the assertion that fails rather
   than committing a red test or a wrong expectation, and say so.
5. After applying the fix, run the dependent suites before believing it. Fixing a
   parser to reject what it should always have rejected can break input that was
   written against the buggy behaviour — see the `heretic2.fgd` case below.

## 7. Measurement pitfalls

These cost real time to rediscover. Read them before trusting any number.

- **Files absent from the report are not covered — they are invisible.** The linker
  drops objects no test references, so a translation unit nothing exercises produces
  no coverage mapping and never appears in `llvm-cov report`. It is *not* listed at
  0%. Never infer "this file is fine" from its absence — check the library's source
  list against the filenames in the report. Adding the first test for such a unit
  makes the library's reported total go *down*, because previously invisible code
  joins the denominator.
- **`STATIC_CHECK` and `constexpr` produce no coverage data.** Compile-time
  assertions never execute in the instrumented binary. `ColorT.h` and
  `ColorComponentType.h` carry about a hundred `STATIC_CHECK`s between them and look
  poorly covered as a result. A `STATIC_CHECK` is *stronger* than a runtime check —
  it fails the build. Do not add runtime duplicates to move the number.
- **Template instantiations are counted separately.** A function tested thoroughly
  for one instantiation shows zero counts for its siblings. `Cf::fromValues` is
  tested for success and failure, yet `ColorT.h` still reports uncovered regions
  because `Cb`'s instantiation is never called. Chasing every instantiation is
  combinatorial busywork.
- **Switch statements keep one unreachable branch.** The compiler-generated
  no-case-matched path of a switch over a scoped enum cannot be reached without
  casting an out-of-range value in, which is undefined behaviour. 87.5% on an
  8-branch switch is done.
- **A test filter that matches nothing still reports success.** `TbBaseLibTest
  "Foo"` prints "No tests ran", not a failure. Check the assertion count. Catch2
  filters match test case names, not the `tst_` file name, so check `--list-tests`
  before assuming a filter is wrong.
- **A function at 0% may be dead code rather than untested code.** Two of TbElLib's
  were: `el::typeForName` and `ELTokenizer::appendUntil` had no callers anywhere in
  `lib/` or `app/`. Grep for callers before writing the test — the useful outcome
  may be a deletion, and pinning dead code with tests only makes it harder to
  remove later.

Together these mean the tool systematically under-reports constexpr-heavy and
template-heavy headers, and silently omits untested units. Read the code before
concluding a gap is real.

## 8. Worked example — TbBaseLib

Starting point: 74.5% lines, 62.8% branches. After ten units: **92.4% lines, 84.9%
branches, 93.5% functions**, against a region count that grew from 488 to 587 as
invisible translation units came into view.

Two production bugs, both found at branches the ranking flagged as never-taken:

- **A leading `+` on any number silently parsed as 0.** `Tokenizer::readInteger` and
  `readDecimal` accept a leading `+`, but `std::from_chars` rejects it and
  `.value_or(0.0)` swallowed the failure. Every map, FGD, DEF, shader, ASE and EL
  number was affected. Fixed in all eight `kdl::str_to_*` functions.
- **`NotifierConnection` move-assignment leaked connections.** `= default` replaced
  the connection vector without disconnecting, leaving observers registered against
  the class's own RAII contract. Latent — every call site used `operator+=` — but a
  trap for the next caller.

Plus one design fix: `Tokenizer::TokenNameMap` changed from `std::unordered_map` to
`std::map`, so parser error messages like `Expected integer or decimal, but got ...`
no longer word themselves differently between platforms.

Still open in TbBaseLib: `ColorChannel.cpp` and `Uuid.cpp` have no tests at all and
are invisible to the report; the `Color*` headers are at the point where the
remaining gap is `STATIC_CHECK` and instantiation artifact rather than risk.

## 9. Worked example — TbElLib

Starting point: 78.8% lines, 68.4% branches, 76.5% functions. After five units:
**98.8% lines, 93.3% branches, 100% functions**.

The ranking put `Value.cpp` on top for a structural reason the numbers only hint at:
there was no `tst_Value.cpp`. Its 900 lines were covered incidentally through the
expression tests, and `at`, `atOrDefault`, `contains` and `keys` had never been
called at all. `tst_EL.cpp` turned out to test nothing but `Value`, so it was
renamed and restructured into sections rather than duplicated alongside a new file.

Four production bugs, every one at a branch the ranking flagged as never-taken:

- **Six typed accessors named the wrong type.** `Value::booleanValue` and its five
  siblings each passed a hardcoded, wrong `ValueType` to `DereferenceError`, so
  `stringValue` on a number reported "as type 'Boolean'". These messages reach
  users through entity property and FGD evaluation errors.
- **`typeForName` had no case for `"Null"`.** `typeName(ValueType::Null)` returns
  `"Null"`, but feeding it back hit `contract_assert(false)`. Latent — the function
  had no callers — but a trap. Pinned afterwards with a round-trip loop over all
  eight values, which will catch the next enum addition.
- **`&&` and `||` never evaluated their right operand** on the fallback path.
  `std::make_optional<Value>()` returns an *engaged* optional holding a default
  `Value`, which is Null, so `if (!rhs) { rhs = evaluateRhs(); }` was dead code and
  the error named `Null` instead of the operand's real type. The coverage
  fingerprint was unmistakable: the guard showed `true=0 false=0`.
- **A lone `=` parsed as `==` and swallowed the next character.** The tokenizer's
  inner-switch `case '='` tested `curChar()` — always `'='` there, since that switch
  does not advance first — instead of `lookAhead()`, as the `case '.'` beside it
  correctly does. `1 = 2` silently parsed as `1 == 2`, and `1 =+ 2` ate the `+`.

That last fix broke a shipped game file. `heretic2.fgd` wrote `target => "..."` 39
times, which had only ever worked because the bug consumed the `>`. The same file
spells it `==` 31 times elsewhere, so it was one author's typo that the tokenizer
had been silently accepting. Corrected in the FGD — but note that
third-party FGDs carrying the same typo now fail to load, which is the kind of
consequence worth surfacing before the fix lands rather than after.

Plus two cleanups the report surfaced as 0% rows: `ELTokenizer::appendUntil` was
dead and was deleted along with the two includes it alone needed, and `NumberDelim`
omitted the comparison and bitwise characters, so `1==2`, `1<2` and `1&2` failed to
parse while `1+2` worked.

What remains uncovered is artifact, not risk: of the 90 uncovered branches, 59 are
compiler-generated switch case labels in `Expression.cpp` and 21 more in
`Value.cpp`, the rest being `switchDefault()` and `contract_assert` arms. Two are
genuinely unreachable by construction — `!optimizedLeftOperand` in `optimize`, since
the left operand is always evaluated first, and the false arm of `leftPrecedence <
rightPrecedence`, since the parser only builds left-leaning trees.
