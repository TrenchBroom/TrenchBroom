# Fixture notes

`minimal.tbscene.fixture.json` is the canonical manifest-shaped fixture, not a ZIP
archive. Its zero-length payload entries use the SHA-256 of empty bytes so JSON Schema
and manifest-index validation can run without a binary package writer. The component
extension documents the representative semantic coverage expected from the first full
package fixture.

The codec work package must replace this with an actual deterministic `.tbscene`
archive containing a non-empty `TBM1` triangle, one convex solid, entity records, and
the four standardized component records. That test must derive the document hash; the
placeholder all-zero document hash is intentionally invalid for a full package.
