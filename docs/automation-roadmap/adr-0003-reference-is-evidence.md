# ADR 0003: A reference is evidence, not ground truth

- **Status:** Accepted
- **Date:** 2026-08-28
- **Owners:** TrenchBroom automation and acceptance infrastructure

## Context

A comparison context gives two documents durable `reference` and `candidate` roles. In
map reconstruction the reference may itself be an imperfect export from legacy data.
The exporter, source assets, or their interpretation may contain gaps, overlaps,
misassigned materials, or inaccessible spaces that have not been manually verified.
Treating every candidate-to-reference difference as a candidate defect would preserve
known mistakes and hide useful repairs.

At the same time, silently dismissing differences because the export might be wrong
would remove the acceptance system's value. We need stable measurements without
pretending that either document is intrinsically authoritative.

## Decision

Comparison engines produce **neutral, symmetric measurements**. For occupancy these are
`referenceOnly` and `candidateOnly`; other domains use similarly role-relative names.
The engine does not assign correctness, severity, or pass/fail meaning.

A separate divergence policy may classify a measured finding as:

- `review`: unexplained and requiring attention;
- `intendedChange`: a deliberate design divergence;
- `acceptedRepair`: a reviewed correction to the reference; or
- `waived`: understood but temporarily tolerated.

Every non-review rule requires a rationale and an evidence reference. Evidence might be
an in-game screenshot, a client observation, exporter investigation, design decision,
or tracked limitation. Rules can be scoped by comparison domain, divergence direction,
and world-space bounds. Partial spatial matches do not waive an aggregate finding: the
engine partitions it into cells and classifies only cells wholly inside a rule's scope.

Severity is policy layered on top of disposition. Unmatched findings default to
`review` and error severity. Raw measurements are always retained beside their policy
classification so policy cannot rewrite history.

## Consequences

- The exported `unrest.map` is a valuable baseline, not an unquestionable oracle.
- Candidate improvements remain visible and reviewable instead of being forced back to
  exporter output.
- Accepted repairs carry provenance that future agents can inspect and challenge.
- Changing policy never changes the underlying geometric or visual measurement.
- Gates can remain strict: unexplained divergence fails, while reviewed exceptions are
  explicit rather than hidden.

The first implementation accepts policy inline with a comparison request. Persisting
rules in the acceptance project and reviewing them in the UI are follow-up work.

## Rejected alternatives

### Reference always wins

Rejected because it codifies exporter defects and conflicts with reconstruction work
whose purpose includes improving the exported map.

### Candidate always wins

Rejected because most accidental regressions also exist only in the candidate.

### Ignore known-bad regions in the measurement engine

Rejected because exclusions erase evidence. The engine must report what differs; policy
is responsible for recording why a difference is acceptable.
