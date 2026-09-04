# Search / Filter Query Language for TrenchBroom

## Context

TrenchBroom currently has no free-text way to find or isolate objects in a map. The only filtering is boolean/checkbox-based (`View > Filter`, backed by `mdl::EditorContext`'s fixed flags and hidden-tag/entity-definition bitsets) and per-browser substring search boxes (`EntityBrowser`/`MaterialBrowser`, via `ui::createSearchBox()`). None of this lets a user ask "select every trigger brush in this room" or "find the entity named door3" in one shot.

The goal is a search/filter feature: a text box where simple input does substring matching, but the same box accepts a small, more expressive query language for filtering by object type, entity/brush properties, and spatial extent — with a visual query builder as a later follow-up. Because TrenchBroom already ships an expression language (EL, `lib/TbElLib`) used in compile profiles, launch configs, and `.fgd` model expressions, the design goal is for this query language to look and feel like EL rather than inventing something unrelated — reusing its literals, operators, precedence, and `Value` model — while extending EL itself with the handful of generally-useful constructs it doesn't have yet: glob/substring matching, membership/containment, and basic vector/bounding-box math. Search then becomes "write an EL expression, evaluated with per-object variables bound in" — no separate/sibling grammar needed at all.

This document has two parts: the language spec (grammar, schema, type inference, examples) to review, followed by a concrete four-PR implementation plan with file-level detail. Writing this plan doesn't itself change any code — that starts once the plan is approved.

## What EL already gives us (reused as-is)

Confirmed from `lib/TbElLib/include/el/*` and the manual (`app/TrenchBroom/resources/documentation/manual/index.md`, "Expression Language" section):

- **Literals**: `true`/`false`; `'...'`/`"..."` strings; numbers (`123`, `1.23`); arrays `[e1, e2, ...]` (elements may be ranges); maps `{ key: expr, "key with space": expr }`.
- **Variables**: bare names (`( _ | Alpha ) { _ | Alpha | Numeric }`), resolved through a variable store — this is the hook we use to bind per-object fields (see below).
- **Operators**, high→low precedence today: unary `+ - ! ~`; `* / %`; binary `+ -` (also string/array concat, map merge); `<< >>`; `< <= > >=`; `== !=`; `&` (bitwise and); `^` (xor); `|` (bitwise or); `&&`; `||`; `..` (range); `->` (case).
- **Subscript** `expr[idx, ...]` on String/Array/Map, including auto-ranges (`[..3]`, `[3..]`) and negative indices.
- **Switch/case** `{{ cond1 -> val1, cond2 -> val2, default }}` — a piecewise conditional. Comes for free; rarely needed for filters but not disallowed.
- Full type-coercion rules between Boolean/String/Number (e.g. `"200" > 100` works) — useful for numeric property comparisons like `properties.light > 200`.
- **Comparisons are `Undefined`-safe.** Map subscript on a *missing key* returns `Undefined` rather than erroring, and the manual's comparison table defines `Undefined` against any other type as simply not-equal/lesser (e.g. `null == undefined` demonstrates the pattern) — so `properties.targetname == "door1"` on an entity that lacks `targetname` evaluates to `false`, it does not throw.
- **Subscripting `Undefined` itself currently throws — fixed as a preparation step (see below).** `evaluateSubscript` (`lib/TbElLib/src/Expression.cpp:984-1114`) switches on the left operand's type and falls through to `throw IndexError{...}` whenever the left operand isn't already a String/Array/Map — including when it's `Undefined`. Since dot-access evaluates through the same `evaluateSubscript` logic, `x.y` would throw if `x` itself were `Undefined`, even though `map["missing"]` doesn't throw. This spec assumes that's fixed first (Preparation, in Follow-up work), so every field below — container-typed or scalar — can be treated uniformly: unbound is just unbound, safe to chain through, never a crash.

What EL does **not** have yet, and that a query language needs: glob/substring matching, a membership/containment test, function calls, and vector/bounding-box math. All four are proposed as core EL additions below, not search-specific bolt-ons — they're generally useful (e.g. a compile-profile or `.fgd` expression doing a glob match or coordinate math is a reasonable thing to want, independent of search).

## New EL constructs (in `lib/TbElLib` itself)

### Operators: `like` and `contains`, as functions plus generic infix sugar

Earlier drafts of this spec added `like`/`in` as dedicated `BinaryOperation` enum members (their own tokens, keyword-reservation, dispatch cases). That's been replaced with something more general and, on inspection of how EL's parser actually works, no more work to build: **`like`/`contains` are ordinary builtin functions** (same table as `vec`/`bbox`/`distanceTo`/`intersects`), plus a new **generic infix-call shorthand** that lets *any* two-argument function be written `lhs name rhs` instead of `name(lhs, rhs)`. `x like "a*"` is sugar for `like(x, "a*")`; nothing about `like` itself is special-cased in the grammar. (`in` was renamed to `contains` — with the argument order swapped so the *container* comes first — partway through implementation, since `x in a` read backwards once generalized past `like`/`in`'s original comparison-operator framing: `tags contains "Detail"` and `bbox(...) contains point` both read as "container, then the thing it holds," matching normal English and how `Value::contains`/`vm::bbox3d::contains` are already named in this codebase.)

This was confirmed feasible by reading `Parser.cpp`/`Expression.cpp` directly: EL's parser doesn't do tiered recursive-descent precedence parsing. `parseCompoundTerm` parses every binary operator into one flat left-to-right chain, and a separate post-pass, `ExpressionNode::rebalanceByPrecedence()`, rotates that flat tree into the precedence-correct shape afterward using a `precedence(BinaryOperation)` table. See "Infix call sugar" below (in the New EL constructs section) for the mechanism — it turns out to require **no changes at all** to the rebalancing pass, by representing the infix call as a `BinaryExpression` with a new `BinaryOperation::InfixCall` tag rather than as a new AST node type, so it rides along for free through machinery that already dispatches generically on `BinaryExpression`/`.operation`.

`like`/`contains` semantics table:

| Function | Signature | Meaning |
|---|---|---|
| `like(lhs, rhs)` | `(String\|Array, String) -> Boolean` | Glob/substring match. `String like String` → case-insensitive glob (`*`/`?` wildcards; a pattern with no wildcards matches as a substring). `Array like String` → true if *any* element matches. Never throws — an `Undefined` operand simply fails to match. Matches the glob semantics already used by game-config tag `match`/`pattern` entries (e.g. `"pattern": "func_detail*"` in `Quake/GameConfig.cfg`), so this isn't a new idea for TrenchBroom users. Implemented via the existing `kdl::ci::str_matches_glob` (case-insensitive glob, already used elsewhere in the codebase) — a wildcard-free pattern is wrapped in `*...*` before matching, to get the spec's substring behavior. |
| `contains(lhs, rhs)` | polymorphic | Polymorphic membership/containment, also never throws on `Undefined`: `Array contains X` → element membership; `Map contains String` → key membership; `Range contains Number` → range membership; `BBox contains Vec3` → point containment (via `vm::bbox3d::contains(point)`); `BBox contains BBox` → full containment/subset (via `vm::bbox3d::contains(bbox)`). Container always comes first, `lhs`. |

Both are invoked infix wherever this spec's worked examples use `like`/`contains` (`materials like "*trigger*"`, `tags contains "Detail"`) — that syntax is now just the generic infix-call sugar applied to these two specific functions, not bespoke grammar for them. A pleasant side effect: `like`/`contains` are no longer reserved words at all — a variable could still be named `like` (only *unusable* as one immediately after a complete term, same as any other function name would be there).

### Value types

- **`Vec3`** — a 3-component vector (backs `vm::vec3d`). Component access via numeric subscript (`v[0] v[1] v[2]`) and string-keyed subscript (`v["x"] v["y"] v["z"]`, so `v.x`/`v.y`/`v.z` via dot sugar). Arithmetic: `+`/`-` component-wise between two `Vec3`; `*`/`/` between `Vec3` and `Number` (scale). `==`/`!=` compare components.
- **`BBox`** — an axis-aligned box (backs `vm::bbox3d`), holding a min and max `Vec3`. Accessed via string-keyed subscript like a map: `b["min"]`/`b.min`, `b["max"]`/`b.max`.

These are genuinely new `el::Value` variant members alongside the existing `Boolean/String/Number/Array/Map/Range/Null/Undefined`, not sugar over `Array` — reusing bare arrays as vectors would collide with `Array`'s existing `+` (concatenation, not vector addition).

### Builtin functions

EL currently has no function-call syntax at all (`Name "(" ... ")"` is unused grammar space). This proposal adds it, but stays true to EL's "no user-defined functions" philosophy: a small, fixed, closed builtin table, similar in spirit to spreadsheet formula functions — not general scripting.

```
CallTerm = Name "(" [ Expression { "," Expression } ] ")"   // new SimpleTerm alternative
```

| Function | Signature | Meaning |
|---|---|---|
| `vec(x, y, z)` | `(Number, Number, Number) -> Vec3` | construct a vector. |
| `bbox(min, max)` | `(Vec3, Vec3) -> BBox` | construct a box; corners normalized internally (component-wise min/max) so argument order doesn't matter. |
| `distanceTo(a, b)` | `(Vec3, Vec3) -> Number` | Euclidean distance — the building block for sphere-style queries. |
| `intersects(a, b)` | `(BBox, BBox) -> Boolean` | true if the two boxes overlap (as opposed to `contains`'s full-containment test). |
| `like(lhs, rhs)` | `(String\|Array, String) -> Boolean` | glob/substring match — see the operators section above. |
| `contains(lhs, rhs)` | polymorphic | membership/containment, container first — see the operators section above. |

This table is deliberately small; it's also exactly the shape a future query-builder UI would want (function name → labeled, typed argument slots). Natural future additions without further grammar change: `length(vec)`, `normalize(vec)`, more math helpers — none of this is search-specific. Every two-argument entry in this table (today: `distanceTo`, `intersects`, `like`, `contains`) is automatically usable via the infix sugar below, with no per-function work.

### Infix call sugar: `lhs name rhs` as sugar for `name(lhs, rhs)`

Alongside prefix call syntax, add a generic shorthand for any two-argument function call:

```
InfixCall = SimpleTerm Name SimpleTerm   // new CompoundTerm alternative, binding tighter
                                          // than every real binary operator -- see below
```

`a like "x"` and `like(a, "x")` are exactly equivalent, evaluating via the same `evaluateCall` path. Represented as a **`BinaryExpression` whose `operation` is a new `InfixCall` alternative**, not a new AST node type — an infix call and a prefix call already have identical evaluation semantics (unlike, say, dot-access vs. bracket-subscript, which was the case that justified `DotExpression` as its own node), so there's no need to duplicate `BinaryExpression`'s shape.

**Prerequisite refactor, done first and on its own: `BinaryOperation` changes from a plain `enum class` to a `std::variant`.** The alternative considered — bolting a `std::string functionName` field directly onto `BinaryExpression`, defaulted so it's a no-op for the other 20 operations — works, but makes an invalid state representable (every `Addition`/`Less`/`Case`/etc. node carries an always-empty string it never uses). Modeling each operation as its own (mostly empty) struct and `BinaryOperation` as the variant of all of them means only `InfixCall`'s struct *can* hold a function name — the type system rules out the invalid state instead of a convention (leave it empty) ruling it out. This also brings `BinaryOperation` in line with how `Value`, `Expression`, and `RangeType` (`std::variant<LeftBoundedRange, RightBoundedRange, BoundedRange>`, already in `Types.h`) are all modeled in this codebase — `kdl::overload`/`std::visit` dispatch is the established idiom here, not enum switches. Since this is a pure representation change (identical behavior, just a different C++ encoding of the same 20 operations), it's done as its own preliminary, purely-mechanical, behavior-preserving step — see step (f-0) below — before any new capability (`InfixCall`) is added on top of it.

```cpp
namespace binop
{
// one (mostly) empty struct per existing operation, each carrying its own precedence
// (colocated with the operation, rather than re-listed in a separate table that could
// drift out of sync) as a static constexpr member:
struct Addition { static constexpr size_t precedence = 11; kdl_reflect_inline_empty(Addition); };
struct Subtraction { static constexpr size_t precedence = 11; kdl_reflect_inline_empty(Subtraction); };
// ... one per existing BinaryOperation enumerator ...
struct BoundedRange { static constexpr size_t precedence = 2; kdl_reflect_inline_empty(BoundedRange); };
                                                 // nested to avoid colliding with the
                                                 // *unrelated* top-level `el::BoundedRange`
                                                 // struct that RangeType uses
struct Case { static constexpr size_t precedence = 1; kdl_reflect_inline_empty(Case); };

// the one alternative with a real payload, added in step (f-i):
struct InfixCall
{
  static constexpr size_t precedence = 13;  // binds tighter than every real operator --
                                             // see the correctness note below
  std::string functionName;
  kdl_reflect_inline(InfixCall, functionName);
};
} // namespace binop

using BinaryOperation = std::
  variant<binop::Addition, binop::Subtraction, /* ... */, binop::Case, binop::InfixCall>;
```

`precedence(const BinaryOperation&)` then collapses to `std::visit([](const auto& op) { return op.precedence; }, operation)` — one generic lambda reading each struct's own member, no per-operation case to keep in sync at all.

`kdl_reflect_inline_empty`/`kdl_reflect_decl`+`kdl_reflect_impl` are existing macros in `lib/KdLib/include/kd/reflection_decl.h`/`reflection_impl.h` — `kdl_reflect_inline_empty` specifically exists for exactly this "empty struct, still needs `==`/`<<`" case, and `LeftBoundedRange`/`RightBoundedRange`/`BoundedRange` in `Types.h` already use the non-empty variant of this same macro pair, so this isn't a new pattern, just this macro pair's first use for an *empty* struct in this codebase. The tag structs live in a nested `binop` namespace specifically because `BoundedRange` (an existing `BinaryOperation` enumerator, for the `..` operator) would otherwise collide with the *already-existing*, unrelated top-level `el::BoundedRange` struct used by `RangeType` — confirmed via grep before finalizing this shape, since naming this refactor's structs directly at `el` scope (mirroring `RangeType`'s own alternatives, which don't need a wrapping namespace) seemed cleaner at first glance but silently breaks on this one name.

**Correctness note — why 13, not 8 alongside `==`/`!=` as originally planned**: giving `InfixCall` the *same* tier as `==`/`!=` (as first implemented) is wrong, caught by working through a concrete example: `x distanceTo y == z distanceTo x` should mean `distanceTo(x, y) == distanceTo(z, x)`, two independent calls compared by `==` — the same way `x * y == z * x` already means `(x*y) == (z*x)` today. But equal-precedence operators never rotate relative to each other in this rebalancing scheme (rotation requires strictly *greater* precedence), so at tier 8 the flat parse stays exactly as typed, left-to-right: `(x distanceTo y == z) distanceTo x` — nonsensical, since `==`'s Boolean result becomes `distanceTo`'s first argument. An infix call needs to behave like a real function call always does: self-contained regardless of what's adjacent, i.e. it must bind *tighter than every real operator*, not sit among them at any particular tier. Precedence 13 (the same sentinel value `precedence(const Expression&)`'s catch-all already uses for every atomic leaf — literals, variables, prefix calls) is exactly that: since no real operator can ever exceed it, an `InfixCall` node can never be the "weaker" side of a rotation, so it's never split apart — matching how a prefix call's parens already make it fully self-contained, without needing actual parens in the infix form. Verified via the `1 + 2 like 3 + 4` cross-tier trace at both the wrong tier (8) and the fixed one (13) before writing any code, then confirmed with tests covering `+`, `*`, `&&`, and `==` explicitly (see step (f-i) below) — every tier gets the same treatment, since the whole point is that `InfixCall` doesn't have "a tier" in the traditional sense.

`evaluateBinaryExpression` and `operator<<` for `BinaryExpression` change from a `switch` (with `switchDefault()`, this codebase's marker for "compiler-enforced exhaustive, no default case") to a `std::visit(kdl::overload(...), operation)` — same exhaustiveness guarantee, just via the variant idiom instead of the enum one. `precedence(const BinaryOperation&)` goes further: each tag struct carries its own `static constexpr size_t precedence` member (colocated with the operation it belongs to, rather than listed a second time in a separate table that could drift out of sync), collapsing the function to `std::visit([](const auto& op) { return op.precedence; }, operation)` — one generic lambda, no per-operation case at all; a future alternative missing the member fails to compile rather than silently defaulting to something wrong. `operator==(const BinaryExpression&, const BinaryExpression&)` needs **no change at all**: `lhs.operation == rhs.operation` already recurses correctly into `InfixCall`'s `functionName` once `BinaryOperation`'s per-alternative `==` exists (generated by the reflect macros), exactly the same way it already correctly compares `leftOperand`/`rightOperand` today.

**`InfixCall` itself is a small, additive second step (f-i) once the refactor lands**: add `binop::InfixCall` (`precedence = 13`, binding tighter than every real operator — see the correctness note above) to the variant; add its case to the two remaining `std::visit`s (`precedence` needs no new case at all, since it already just reads `op.precedence` generically — `evaluateBinaryExpression` routes it to `evaluateCall(context, infixCall.functionName, {evaluateLhs(), evaluateRhs()}, expressionNode)`; `operator<<` prints `lhs.leftOperand << " " << infixCall.functionName << " " << lhs.rightOperand`); and, in `Parser::parseCompoundTerm`, check for `ElToken::Name` *before* the existing `TokenMap` lookup (which only knows fixed operator-symbol tokens) and, when found, build `BinaryExpression{BinaryOperation{binop::InfixCall{token.data()}}, std::move(lhs), parseSimpleTermOrSwitch()}` instead of looking `TokenMap` up.

**Why this needs no changes to the rebalancer, either before or after the refactor**: `precedence(const Expression&)`'s `BinaryExpression` case already just forwards to `precedence(binaryExpression.operation)`, and `rebalanceByPrecedence()`'s rotation logic reaches into a child via `std::get<BinaryExpression>(*childNode.m_expression)` — since the infix-call node structurally *is* a `BinaryExpression` (just carrying a different `operation` alternative), both stay correct completely unmodified, whether `operation` is the old enum or the new variant. (Hand-traced `1 like 2 + 3` to confirm real precedence participation is necessary, not just nice-to-have: it must parse as `(1 like 2) + 3`, `like` grabbing only its immediate neighbors since it binds tighter than `+` — a fixed/atomic-leaf treatment that skipped participating in rebalancing entirely would instead leave the flat left-to-right parse unrotated, which happens to look the same for *this* particular example but was proven wrong in general by the `x distanceTo y == z distanceTo x` case in the correctness note above.)

**Consequence, deliberately accepted**: any bare word following a complete expression is now a function-call attempt (`a foo b` parses fine regardless of whether `foo` is registered, erroring "Unknown function: 'foo'" only at evaluation) — before this change such input was a parse error. This exactly matches how a mistyped *prefix* call (`foo(a, b)`) already behaves, so it's not a new class of surprise, just the same one in a new position. Confirmed with the user: infix sugar should work for *any* function name syntactically (not a parser-level allow-list restricted to `like`/`contains`), consistent with EL's existing "unknown function is an evaluation-time error, not a parse-time one" philosophy.

### Field access sugar: dot syntax

EL currently requires `map["key"]` for every field access, which gets noisy for the kind of chained property lookups a query does constantly (`properties["classname"]`, `bounds["min"]`). Add `.` as sugar for string-keyed subscript:

```
DotAccess = SimpleTerm "." Name    // sugar for SimpleTerm "[" "Name-as-string-literal" "]"
```

`expr.name` evaluates identically to `expr["name"]` — same underlying `evaluateSubscript` logic, no new evaluation semantics — so it's valid wherever string subscript already is: `Map` (`properties.classname`), and the new `Vec3`/`BBox` types (`center.x`, `bounds.min`, `bounds.max.z`). It chains and composes with bracket subscript normally (`a.b.c`, `a.b[0]`, `a[0].b`). Bracket-with-string form remains necessary (not replaced) for keys that aren't valid identifiers — spaces, punctuation — or for a dynamically computed key (`properties[someVar]`). It's kept as its own `DotExpression` AST node rather than literally rewritten into a `SubscriptExpression` at parse time, so that printing an expression back out (`asString()`) preserves the dot syntax it was written with instead of always rendering as bracket form.

This is a lexer-level addition, not just a parser one: EL's number literals already use `.` for decimals (`1.5`), so the tokenizer needs to keep treating a `.` that's part of a digit run as a decimal point, and only emit a standalone `.` (dot-access) token when it appears after a completed term — the same disambiguation every C-like/Lua-like tokenizer with both decimals and dot-access already does, not a novel problem.

### `BoundValue`: a lazy, descriptor-driven `Value` alternative — replaces the original PR3 `NodeVariableStore`/`FaceVariableStore` design

**Why**: the first cut of PR3 (built and fully tested earlier, then discarded on a fresh branch per explicit decision) bound each node/face's schema via a hand-written `el::VariableStore` subclass whose `value(name)` was a long if/else chain, and whose `entity`/`properties` fields *eagerly* built a full `el::Value::Map` — copying every one of an entity's properties into fresh `Value::String`s — every time either field was referenced. For an entity that owns many brushes (a large `func_detail`), a query touching `entity.classname` rebuilds that entire property map once per brush, just to read one field. This was flagged and deliberately deferred as a "possible future optimization" during the original design — see the git history/prior plan revision for that discussion — but the user has since decided to build it now, before continuing PR3, and to generalize it: the same mechanism should also replace hand-written `VariableStore` implementations wherever it's a good fit, not just serve the search feature.

**The mechanism**: a new, generic, type-erased `el::Value` alternative — `BoundValue` — that behaves like a read-only `Map` but never materializes one. It holds three raw (non-owning) pointers:

```cpp
struct BoundValueDescriptor
{
  // Looks up `key` against `object` (and optional `context` pointer for cases that need
  // ambient data beyond the bound object itself, e.g. the owning Map for tag lookups).
  // Returns nullopt on a missing key -- the caller (evaluateSubscript) turns that into
  // Value::Undefined, exactly like a Map's missing-key behavior today.
  std::function<std::optional<Value>(const void* object, const void* context, const std::string& key)> at;
  std::function<std::vector<std::string>(const void* object, const void* context)> names;
};

struct BoundValue
{
  const void* object;             // the bound C++ object, e.g. a `const Node*`
  const void* context;            // optional secondary object, e.g. a `const Map*`; nullptr if unused
  const BoundValueDescriptor* descriptor;
};
```

`at`/`names` are `std::function`, not raw function pointers — a per-kind descriptor's `at` closure typically needs to capture its own `static const std::map<std::string, std::function<Value(const void*, const void*)>>` of per-field accessors (or, for the dynamic case below, just close over nothing and call straight into the object's own API), and `std::function` is the natural vocabulary type for "a callable descriptor entry," matching how the codebase already reaches for `std::function` at similar seams (e.g. `TestFactory.h`'s `createBrushNode`'s `brushFunc` parameter). Two kinds of `BoundValueDescriptor` cover every case we have:
- **Static schema** (the node/face field tables, and any future "expose this C++ type's fixed fields" need): `at`/`names` close over a `static const std::map<std::string, std::function<Value(const void*, const void*)>>` built once per concrete C++ type (e.g. one table for `EntityNode`, one for `BrushNode`, ...). A field that doesn't apply to a given kind is simply **absent from that kind's table** — `at` returns `nullopt`, which becomes `Undefined`, with no explicit "-> Undefined" branch to write or forget, unlike the discarded design's `kdl::overload` visitors.
- **Dynamic schema** (a property bag whose keys aren't known at compile time, e.g. `Entity`'s properties): `at`/`names` call straight through to the wrapped object's own API (`entity->property(key)`, `entity->propertyKeys()`), no table at all.

Constructing a `BoundValue` is three pointer-copies — no allocation, no field materialization. Only the *specific* key someone actually subscripts (`entity.classname`) ever runs its accessor and produces a real `Value`. Nested fields compose naturally: an `entity` field's accessor returns `Value{BoundValue{&entityNodeBase->entity(), nullptr, &EntityDescriptor}}`, itself lazy, and `entity.classname` resolves by re-entering `evaluateSubscript` on that nested `BoundValue`.

**Scope, deliberately narrow for this first version** (mirrors how `BBox` in PR2 got only construction/subscript/comparison, not full arithmetic):
- **Supported**: string-keyed subscript/dot-access (`evaluateSubscript`'s `Map`-handling logic gets a parallel `BoundValue` case; numeric/array-of-keys subscript stays unsupported, matching `Map`'s own restriction there), `contains(context, key)` (existence check without evaluating the value — reuses the same descriptor lookup `evaluateContains`'s `Map` case already does via `Value::contains`), `appendToStream` (prints as `{ k: v, ... }` by iterating `names()` and calling `at()` per key — same shape as `Map`'s printer, so a `BoundValue` in an error message or `asString()` looks exactly like the `Map` an EL author believes it to be), and `operator==` (reference equality: same `object`+`context`+`descriptor` triple — cheap, well-defined, sufficient since nobody compares two whole bound objects in a filter query; they drill into fields instead).
- **Unsupported** (falls to the existing "invalid type" `break`/throw path already in every relevant switch, exactly like `Map` itself is unsupported for these): unary operators, arithmetic (`+`/`-`/etc. — no merge semantics, unlike `Map + Map`), ordering comparison (`<`/`<=`/`>`/`>=`) via `evaluateCompare`, and numeric/string indexing.
- **`typeName(ValueType::BoundValue)` returns `"Map"`**, not `"BoundValue"` — the internal `ValueType` enumerator has to be real and distinct (so the exhaustive switches route to the right case), but every user-facing string (error messages, any hypothetical type-reflection) should preserve the illusion that this is just a `Map`, since from the EL author's perspective it always behaves like one.

**Touch points, catalogued exhaustively against source** (so this is a scoped, known-size change, not open-ended): `Types.h`/`Types.cpp` (new `ValueType::BoundValue` enumerator; `typeName`/`typeForName`, per above). `Value.h`/`Value.cpp`: the variant itself; `Value(BoundValue)` ctor; `type()`; a `boundValue(context)` accessor (pattern-matches `mapValue(context)`, throws `DereferenceError` on a type mismatch); `length()` (→ `names().size()`); `convertibleTo`/`convertTo` (identity-to-self only, like every other non-coercible type); `appendToStream`; `contains(context, size_t)` (unsupported, like `Map`); `contains(context, string)` (real: descriptor-existence check); `at`/`atOrDefault(size_t)` (unsupported); `at`/`atOrDefault(string)` (real: descriptor lookup, `IndexOutOfBoundsError`/default on miss, mirroring `Map`'s exact behavior at `Value.cpp:1005-1058`); `operator==` (reference equality, per above). `Expression.cpp`: `evaluateUnaryPlus/Minus/LogicalNegation/BitwiseNegation` (add to the exhaustive `break` group, no behavior — compiler forces this to be visited, that's the point); `evaluateCompare` (add to the `break` group — no ordering/equality via EL comparison operators, deliberately, per Scope above); `evaluateContains` (real case, mirrors `Map`'s at `Expression.cpp:1436-1438`); `evaluateSubscript` (real case, mirrors `Map`'s at `Expression.cpp:1228-1240` — this is the one that actually makes `entity.classname`/`properties.targetname` work); `computeIndexArray`/`computeIndex` (no change needed — `BoundValue` falls into the existing generic numeric-conversion attempt the same way `Map` already does, which already fails correctly). `evaluateAddition`'s `Map + Map` merge is an `if (lhs.hasType(Map) && ...)` check, not a switch — `BoundValue` simply won't match it, so **no change needed there at all**. Also confirm during implementation whether `std::hash<Value>` (needed because `EvaluationContext::m_trace` is a `std::unordered_map<Value, ExpressionNode>`) is itself exhaustive over the variant and needs a `BoundValue` arm — neither research pass into this plan specifically audited the hash specialization, only the `ValueType` switches.

**Replacing `VariableStore` with this, per the user's request to go as far as reasonably possible**: a repo-wide survey found exactly 3 real (non-inherited) implementations of `el::VariableStore`, plus a family of `el::VariableTable` subclasses:
- **`el::VariableTable`** (`lib/TbElLib/include/el/VariableStore.h:55-70`) — wraps a real, mutable `std::map<std::string, Value>`; `set()` is a genuine operation, not a stub. This is the default store (`EvaluationContext()`'s parameterless ctor) and the base class `ui::CompilationVariables`'s whole family (`CommonVariables`/`CommonCompilationVariables`/`CompilationWorkDirVariables`/`CompilationVariables`/`LaunchGameEngineVariables`, `lib/TbUiLib/include/ui/CompilationVariables.h`) is built on, via eager `set()` calls in each constructor. **Left unchanged.** `BoundValue`'s accessors are pure functions of a bound object — there's no way to `set()` into one, and these stores' whole job is being small, real, mutable key/value bags, not lazy projections of a bigger C++ object. Forcing them onto `BoundValue` would trade working code for no actual laziness benefit.
- **`el::NullVariableStore`** (same header) — always returns `Value::Null`, `set()` a no-op stub. Used for preview/no-data contexts (e.g. `lib/TbUiLib/src/EntityBrowserView.cpp:236`). **Left unchanged** — trivial, nothing to gain.
- **`mdl::EntityPropertiesVariableStore`** (`lib/TbMdlLib/include/mdl/EntityPropertiesVariableStore.h`) — wraps `const Entity&`; `value(name)` → `entity.property(name)` (falls back to `Value{""}` on miss, **not** `Undefined` — a pre-existing quirk to preserve exactly, since call sites may depend on it); `names()` → `propertyKeys()`; `set()` a no-op stub. **This is the one real migration target**: replace its hand-written body with the generic adapter below plus a "dynamic schema" descriptor calling straight through to `Entity::property`/`propertyKeys` — demonstrating the mechanism replaces at least one existing, shipped store, not only the not-yet-rebuilt PR3 ones. The `Value{""}`-on-miss quirk becomes a one-line wrapper around the generic descriptor-miss (`nullopt`) result at this specific call site, so nothing downstream changes behavior.

New generic adapter, `lib/TbElLib/include/el/BoundValueStore.h`/`.cpp` (or similar — exact name TBD at implementation time): a small `el::VariableStore` implementation wrapping one `BoundValue`, forwarding `value(name)` → `boundValue.at(name)` (miss → `Value::Undefined`, the `VariableStore`-level default), `names()` → `boundValue.names()`, `clone()` → copy the three pointers, `set()` → no-op (consistent with two of the three existing stores already being no-op there). This one adapter is what both `EntityPropertiesVariableStore`'s replacement and the rebuilt PR3 node/face stores (below) are built from — no more hand-written `VariableStore` subclasses needed for either.

## Object schema (implicit variables, bound by the search feature)

The search feature's only job, grammar-wise, is nothing — it reuses stock `el::Parser`/`el::ExpressionNode` unchanged. Its actual job is supplying a per-node `VariableStore` (the same mechanism compile profiles already use to inject `MAP_BASE_NAME` etc.) that binds these variables while a candidate node is evaluated. Every bare field name is, conceptually, shorthand for `<object>.field` where `<object>` is the node currently being tested — the search feature just flattens that implicit object's fields directly into the top-level `VariableStore` rather than requiring a literal `<object>.` prefix, so there's no behavioral difference, just less typing.

| Variable | Type | Bound on | Meaning |
|---|---|---|---|
| `type` | String | always | one of `"world" "layer" "group" "entity" "brush" "patch"` (or `"face"` in face domain — see below). Always available as an explicit, unambiguous way to narrow a query, even though most queries won't need to write it — see type/domain inference below. |
| `name` | String | entity, layer, group | display label (entity's `classname`; layer/group's own stored name). `Undefined` on world/brush/patch, which have no intrinsic display name — this is what bare fuzzy text (Tier 1) matches against. |
| `classname` | String | entity | the entity's own classname. `Undefined` on every other node kind — **no implicit inheritance** from a containing entity (a brush's `classname` is `Undefined`, full stop); see `entity.classname` below for reaching the owner. |
| `properties` | Map | entity | entity key/value properties, via `Entity::property`/`propertyKeys`. Access with `properties.targetname`. |
| `entity` | Map | entity (self), brush, patch (owner) | the relevant entity's data as `{classname: ..., properties: {...}}`. For an `EntityNode` this is itself; for a `BrushNode`/`PatchNode` it's the entity that contains it (via `BrushNode::entity()`), which is `"worldspawn"` for structural geometry. This is the mechanism for a brush-focused query to reach its owner: `entity.classname == "func_detail"`. |
| `materials` | Array\<String\> | brush, patch | distinct face material/texture names — for a brush, the (possibly several) distinct materials across its faces; for a patch, a single-element array holding its one material (a patch has one surface, not per-face materials like a brush). Use this to filter by a face/surface property without leaving node domain; switch to face domain (below) for individual brush faces as the result — patches don't participate in face domain, since they have no per-face structure to iterate. |
| `bounds` | BBox | every node kind | world-space `logicalBounds()`. |
| `center` | Vec3 | every node kind | convenience midpoint of `bounds`. |
| `layerName`, `groupName` | String | every node kind (as applicable) | name of the containing `LayerNode`/innermost `GroupNode`; `Undefined` if not applicable (e.g. `groupName` outside any group). |
| `tags` | Array\<String\> | entity, brush, patch | the node's tag names; test with `tags contains "Detail"`. |
| `visible`, `locked`, `selected` | Boolean | every node kind | from `Node::visible()`/`locked()`/`selected()`. |
| `linked` | Boolean | every node kind | true if the node belongs to a linked group. |

Every field above — scalar or container-typed — is simply left unbound (`Undefined`) on node kinds it doesn't apply to; there's no special "bind an empty `{}`/`[]` instead" discipline to maintain, because the Preparation step (Follow-up work, below) makes subscripting/dot-chaining into an `Undefined` value itself safe. A `GroupNode`'s `properties` is genuinely `Undefined`, and `properties.targetname` on it safely evaluates to `Undefined` rather than throwing, the same way a missing map key already does. This uniformity is also what keeps query-type inference correct (next section): a field being `Undefined` when absent, rather than an empty stand-in, is what makes "was this field referenced at all" and "does this node actually have it" the same question.

## Inferring the query's target type and domain

Earlier drafts of this spec used bare boolean sugar (`entity && classname == "..."`) to pick which node kind a query targets, and a bare `face` marker to switch to face results. Both are unnecessary: since every field in the schema above is only meaningfully bound on a specific node kind (or kind set), the query itself already says what it's looking for. `classname == "info_player_start"` can only ever match an entity — nothing else binds `classname` — so it should just mean that, without an explicit `type == "entity" &&` prefix. This section defines how that inference works, and why it's not purely a convenience/performance feature — it's required for negation to behave correctly.

### Why inference is more than an optimization

Consider `!(classname == "func_detail")` evaluated with **no** domain narrowing, i.e. against every node kind. On a `BrushNode`, `classname` is `Undefined`; `Undefined == "func_detail"` is `false` (comparisons are `Undefined`-safe); so the negation is `true` — every brush in the map would match a query that was clearly meant to mean "entities that aren't func_detail," not "everything that isn't an entity named func_detail." The fix is that this query must only ever be *evaluated* against entities in the first place — narrowing the iteration domain to `{entity}` before evaluating means `classname` is always a real value there, and the negation means what it looks like it means. So: type inference decides which node kinds (or `face`) actually get iterated and evaluated at all; it is load-bearing for negated queries, not just an optimization for large maps (though it's that too).

### Field → kind-set table

| Field(s) | Compatible kinds |
|---|---|
| `classname`, `properties` (non-empty), `name`\* | `{entity}` |
| `entity` (non-empty) | `{entity, brush, patch}` |
| `materials` | `{brush, patch}` |
| `material`, `normal` (face-only, see below) | `{face}` |
| `tags` | `{entity, brush, patch}` |
| `type`, `bounds`, `center`, `layerName`, `groupName`, `visible`, `locked`, `selected`, `linked` | no narrowing — available on every kind |

\* `name` is also bound on `layer`/`group`, so its compatible set is really `{entity, layer, group}`, not `{entity}` alone.

### Algorithm

Walk the parsed expression: a **conjunction** (`&&`) intersects the compatible kind-sets of its operands (narrowing); a **disjunction** (`||`) unions them (broadening, since either side matching is enough); a **negation** (`!`) doesn't change its operand's kind-set — the domain is decided by what's referenced, not by whether it's negated, which is exactly what fixes the example above. A literal `type == "X"` (or `type == "face"`) comparison is treated as an explicit hint with compatible set `{X}`, folded into the same intersection logic — this is the escape hatch for queries with no naturally narrowing field (`visible == false` alone doesn't narrow anything, so `type == "layer" && visible == false` is how you'd scope it).

Resulting cases:
- **A single kind** (e.g. `{entity}`, `{brush}`, `{face}`) → iterate only that kind.
- **A set of kinds** (e.g. `{entity, brush, patch}` from referencing `entity.classname` alone) → iterate the union; e.g. `entity.classname == "func_detail"` matches both the `func_detail` entity itself and its child brushes/patches. Add `&& type == "brush"` to get just the geometry.
- **No narrowing signal at all** (only common fields referenced, e.g. `visible == false`) → fall back to iterating every node kind. Always correct, just potentially the least efficient case — still a cheap linear scan.
- **Empty intersection** (a field exclusive to one kind AND-combined with a field exclusive to an incompatible kind, e.g. `classname == "x" && materials like "y"`) → the query is provably unsatisfiable; return no results without iterating anything.
- **Exactly `{face}`** → switch to face domain and iterate `BrushFaceHandle`s instead of `Node*`s, binding the face schema below instead of the node schema.

### Face domain

Faces aren't `Node`s, so they need their own schema, reached the same inferred way — referencing a face-only field (`material`, `normal`), or writing `type == "face"` explicitly, is what selects face domain; there's no dedicated marker variable to remember.

| Variable | Type | Meaning |
|---|---|---|
| `material` | String | this face's material/texture name (the face-domain counterpart of the node-domain `materials` array). |
| `normal` | Vec3 | face plane normal. |
| `bounds` | BBox | bounding box of the face's polygon — same field name/meaning as the node-domain `bounds`, scoped to one face, so spatial queries (`bbox(...) contains bounds`, `intersects(bounds, ...)`) work unchanged across both domains. |
| `center` | Vec3 | centroid of the face polygon. |
| `entity` | Map | `{classname, properties}` of the owning entity (or `"worldspawn"`), same shape as the node-domain `entity` field — always bound, since every face belongs to some entity. There's no bare `classname`/`properties` shortcut in face domain (a face is never itself an entity, unlike a node), so `entity.classname == "func_detail"` is the only form. |
| `layerName`, `groupName` | String | inherited from the owning `BrushNode`'s containment. |
| `tags` | Array\<String\> | this face's own tags; always bound (`[]` if none) — TrenchBroom's tag matchers already operate at face granularity for material/surface-flag tags (`MaterialNameTagMatcher`/`SurfaceParmTagMatcher`/`ContentFlagsTagMatcher`). |
| `visible`, `locked` | Boolean | inherited from the owning `BrushNode` (faces have no independent visibility/lock state). |
| `selected` | Boolean | whether this specific face is selected (TrenchBroom supports per-face selection for texture application). |

Downstream consumers need to handle two distinct result shapes — a list of `Node*` or a list of `BrushFaceHandle` — which matters for later UI work: node results feed the outliner/3D selection as today, face results feed TrenchBroom's existing face-selection path (used for texture application), not the object outliner.

### Worked examples

```
classname == "info_player_start"                         // infers {entity} — equivalent to the old `entity && classname == ...`
materials like "*trigger*"                                // infers {brush, patch}
material like "*trigger*"                                 // infers {face} — the singular field only exists on faces
entity.classname == "func_detail"                          // infers {entity, brush, patch}: the entity itself and its geometry
entity.classname == "func_detail" && type == "brush"        // narrows further to just the brushes
!(classname == "func_detail")                               // infers {entity}: "entities that aren't func_detail", not "everything that isn't"
visible == false                                             // no narrowing signal -> iterates every node kind
type == "layer" && name like "Combat*"                        // explicit narrowing via `type`, since `name` alone spans entity/layer/group
tags contains "Detail"                                          // infers {entity, brush, patch}
type == "face" && bbox(vec(-128,-128,-128), vec(128,128,128)) contains bounds   // explicit face domain, no material/normal reference needed
```

## Two input tiers: fuzzy text and full EL

The search box supports two escalating input styles, auto-detected from the text.

### Tier 1 — fuzzy text

Plain words with no structural punctuation. `light` and `func_*` are sugar for `name like "*light*"` and `name like "func_*"` — glob wildcards work even here, since they're already meaningful to `like`. (Since `name` is only bound on entity/layer/group, bare fuzzy text only ever matches those — finding a brush by typing a bare word isn't supported; write `materials like "..."` or `entity.classname == "..."` instead.)

### Tier 2 — full EL query

Anything using comparisons, boolean nesting, parentheses, spatial functions, or vector/box literals — the full language described above, with type/domain inferred automatically per the previous section.

### Detection rule

Attempt to parse the input as an expression. If parsing succeeds **and** the input contains at least one EL-structural token (`< > = ! & | ( ) [ ] " '` or the standalone words `like`/`contains`), evaluate it as a Tier 2 query. Otherwise, treat the whole input as a Tier 1 glob/substring pattern against `name`. This keeps a bare word from ever being misread as an EL variable lookup (which would silently evaluate to `Undefined`/false for everything) — a plain word with no structural tokens always takes the fuzzy path.

## Worked examples (full EL)

```
light                                          // fuzzy: any entity/layer/group whose name contains "light"
func_*                                         // fuzzy glob: name starts with "func_"

classname == "info_player_start"
materials like "*trigger*"
properties.targetname == "door1"
!locked && visible && selected
name like "Hallway*" && type == "group"

bbox(vec(-128,-128,-128), vec(128,128,128)) contains bounds              // fully inside the box
intersects(bounds, bbox(vec(-128,-128,0), vec(128,128,256)))             // overlapping the box
intersects(bounds, bbox(vec(-128,-128,0), vec(128,128,256))) && !(tags contains "Detail")

classname == "light" && properties.light > 200   // safe even if "light" is unset: undefined > 200 is false, not an error
!(tags contains "Detail") && !(tags contains "Trigger") && type == "brush"
distanceTo(center, vec(0,0,0)) <= 256 && layerName == "Combat"

material == "trigger"
material like "*lava*"
normal.z > 0.9                                  // roughly upward-facing faces
type == "face" && intersects(bounds, bbox(vec(-128,-128,0), vec(128,128,256)))
entity.classname == "func_detail" && !locked && type == "face"
```

## Implementation plan

Four separately-shippable PRs, in dependency order: each of the first three is buildable and testable entirely on its own, with no UI change and no user-visible behavior until PR 4. Query-builder UI (a fifth, later milestone) isn't detailed here — it's a follow-on once the text-based language is in use.

### PR 1 — EL fix: subscript of `Undefined` no longer throws

**Goal**: make `x[i]`/`x.name` on an `Undefined` `x` evaluate to `Undefined`, matching Map's existing missing-key behavior, without changing any other error case.

**Step 1 (own commit) — characterize current behavior before touching it.** Audit `tst_Expression.cpp`'s existing coverage of `evaluateSubscript` (984-1120): confirm there are tests for each reachable case — `String`/`Array`/`Map` subscript (including the auto-range and negative-index paths already documented in the manual), and the error paths (`Array`/`String` out-of-bounds, and a subscript attempt on each of `Boolean`/`Number`/`Range`/`Null`/`Undefined`). Add whatever's missing so every branch of the current `switch` has at least one test pinning today's behavior — this is what makes step 2 a verifiable, reviewable diff rather than a leap of faith. Commit this addition on its own, with no production-code change, so it can be reviewed purely as "here's what already works today."

**Step 2 (own commit) — the fix.** `lib/TbElLib/src/Expression.cpp`, `evaluateSubscript`. Currently `ValueType::Undefined` falls into the shared case group at lines 1111-1116 that leads to `throw IndexError{...}` at line 1119. Pull it out into its own case placed before that group:
```cpp
case ValueType::Undefined:
  return Value::Undefined;
```
leaving `Boolean`/`Number`/`Range`/`Null` in the existing fallthrough group, still throwing — those remain genuine-mistake cases (subscripting a number, a range, an explicit `null`), not "this thing doesn't exist." Update only the `Undefined`-specific test(s) from step 1 to expect `Value::Undefined` instead of `IndexError`; every other test from step 1 should keep passing unmodified — that's the regression check that the fix is exactly as scoped as intended. Add new cases for `undefinedVar[0]`, `undefinedVar["key"]`, and `undefinedVar.name` (dot-access doesn't exist yet at this point in the plan — PR 2 — so this last one is really just `undefinedVar["name"]`, re-added once dot-access lands in PR 2).

**Verification**: `cmake --build <build-dir> --target TbElLibTest --parallel` and run the resulting test binary (via `ctest`/`catch_discover_tests`) — green after each of the two commits.

### PR 2 — EL additions: dot-access, types, functions, operators

Independent of PR 1 (touches the same file but different code paths), and independent of the query language itself — these are general EL capabilities. Sub-steps, each its own small reviewable diff, in this order: **(a) dot-access sugar → (b) `Vec3` value type → (c) call-syntax infrastructure (own branch, predates `Vec3`) → (c′) rebase `Vec3` onto it + register `vec()` → (d) `BBox` value type + `bbox()` constructor → (e) `distanceTo()`/`intersects()` functions → (f-0) refactor `BinaryOperation` from enum to variant, pure/behavior-preserving → (f-i) add `InfixCall` as a new `BinaryOperation` alternative, i.e. generic infix-call sugar (`lhs name rhs`, proven via the functions that already exist) → (f-ii) register `like`/`contains` as builtin functions, immediately usable both prefix and infix.**

This revises the original four-step split after (b) shipped: a real (if easy to miss) asymmetry exists between "functions need types" and "types are awkward to test without a function to construct them." `vec`/`bbox`/`distance`/`intersects` all construct or consume `Vec3`/`BBox`, so no function can be *implemented* before its type exists — that's a hard, one-directional dependency, confirmed when this ordering was first chosen. But the reverse isn't symmetric: a type doesn't *need* a constructor function to be implemented, only to be pleasantly *testable* — with no literal syntax, `Vec3`'s own tests had to reach it via a bound variable (`tst_Expression.cpp`'s "Vec3" section), the same workaround already used for `Range` elsewhere in that file. That's precedented, not broken, but avoidable: the generic call-*syntax* machinery (`CallExpression`, parsing, dispatch) has no type dependency of its own — only individual builtin functions do. Pulling that machinery forward and registering each constructor as part of its own type's step (rather than deferring all four functions to one later step) means every type after `Vec3` gets a literal-like construction form immediately, and `Vec3`'s existing tests get retrofitted once `vec()` exists in step (c).

**Step (a) — dot-access sugar. Done, uncommitted, verified.**
- `lib/TbElLib/src/Parser.cpp`: tokenizer's `case '.':` block emits a new `ElToken::Dot` when the dot isn't the start of a `..` range and isn't immediately followed by a digit (so `.5` still parses as a number — this exact case was an initial regression, caught by the existing `parse(".0") == lit(0.0)` test, fixed by the digit check). The postfix loop in `parseSimpleTermOrSubscript` handles `Dot` alongside `OBracket`, calling a new `parseDotAccess`.
- `lib/TbElLib/include/el/Expression.h` + `src/Expression.cpp`: **not** desugared into `SubscriptExpression` as originally planned — a genuinely separate `DotExpression{operand, fieldName}` node. `evaluate`/`optimize` delegate to the same `evaluateSubscript` function `SubscriptExpression` uses (identical semantics, including the PR-1 `Undefined`-safety fix), but `operator<<` renders it as `a.b`. This was changed after review: collapsing dot-access into `SubscriptExpression` meant `asString()` always printed bracket form even for expressions written with dots, losing round-trip fidelity.
- Discovered fallout, fixed as part of this step: `lib/TbMdlLib/src/EntParser.cpp`'s `parseModel` used to disambiguate a `model="..."` XML attribute between "literal path" and "EL expression" purely by whether `el::parseExpression` threw. Dot-access removes the one remaining reliable throw for a bare unquoted path with a file extension (`models/foo.md3` tokenizes to a division chain ending in `foo.md3` as valid, if nonsensical, EL), so that heuristic silently broke. Fixed by only attempting EL parsing when the trimmed model string starts with `{` (the only two real forms — map or switch literal); everything else goes straight to the literal-path fallback without risking a spurious successful parse. `FgdParser`/`DefParser` needed no change — traced their model-parsing (`ParseModelDefinition.h`) and confirmed they dispatch between EL and a *structured* legacy grammar, never relying on EL-parse-failure as a proxy signal the way `EntParser` did.
- Tests: `tst_Parser.cpp` (`a.b` parses to its own `DotExpression`, chaining `a.b.c`/`a.b[0]`/`a[0].b`, the `1.5`/`.0` decimal-point non-regression), `tst_Expression.cpp` (evaluation equivalence with bracket form, missing-key safety, chaining, and `asString()` round-trip cases proving `x.y` prints back as `x.y` not `x["y"]`), `tst_EntParser.cpp` (the original bare-path regression case, plus a new case confirming `{`-prefixed-but-malformed content still falls back to a literal path). Verified: `TbElLibTest` 1546 assertions/11 cases, `TbMdlLibTest` 76007 assertions/129 cases, both green.

**Step (b) — `Vec3` value type. Done, uncommitted, verified.** Exhaustively-switched throughout `Value.cpp`, so it needs a case added at every site (compiler enforces this — no `default:` anywhere in these switches):
- `lib/TbElLib/include/el/Types.h` + `src/Types.cpp`: `Vec3` added to `ValueType`; `using Vec3Type = vm::vec3d`; `typeName()`/`typeForName()` extended; `VmLib` added as an explicit `CMakeLists.txt` dependency (its headers are now used from a public header, `Types.h`, not just transitively).
- `lib/TbElLib/include/el/Value.h` / `src/Value.cpp`: threaded through the variant, ctor, `vec3Value()` accessor, `type()`, `length()` (3), `convertibleTo()`/`convertTo()` (trivial-to-self, like `Array`/`Map`/`Range` — plus `String`, both directions, added after review: `"1.2 3 4"` is exactly the format vector-valued entity properties like `origin` use, via `vm::parse<double, 3>`/`vm::operator<<` from `vm/vec_io.h`), `appendToStream()` (prints as `vec(x, y, z)`, matching its future construction syntax so it round-trips once step (c) lands), `operator==`; added to the `contains()`/`at()`/`atOrDefault()` unsupported-fallthrough (that generic accessor API stays `Array`/`Map`/`String`-only; `Vec3` access goes exclusively through `evaluateSubscript`).
- `lib/TbElLib/src/Expression.cpp`: bracket subscript (`v[0..2]`, negative indices, out-of-bounds throws) and string-keyed subscript (`v["x"/"y"/"z"]`, unrecognized key → `Undefined`); every arithmetic operator `vm::vec3d` itself supports — `vec+vec`, `vec-vec`, `vec*vec` and `vec/vec` (both component-wise), `vec*scalar`/`scalar*vec`, `vec/scalar`, `scalar/vec` — modulus has no `vm::vec` equivalent so stays an error; comparison `==`/`!=` plus lexicographic `<`/`>` delegated directly to `vm::vec`'s own `operator<=>` (confirmed it performs the same x-then-y-then-z comparison, rather than reimplementing it by hand); unary `+`/`-` forward to `vm::vec`'s own unary operators (unary `!`/`~` still correctly throw — no sensible vector meaning).
- Tests: `tst_Types.cpp`, `tst_Value.cpp` (every existing exhaustive section extended with a `Vec3` row, including the new `String` conversion), `tst_Expression.cpp` (subscript, unary/binary arithmetic, comparison) — reachable only via a bound variable for now, the same workaround already used for `Range` elsewhere in this file, since there's no literal syntax until step (c). Verified: 1628 assertions, all 11 cases, green.

**Branch note:** step (c) lands on `1247-el-function-syntax`, branched from `1247-el-dot-access` at the "Introduce the dot access operator" commit — i.e. *before* "Introduce Vec3 type", which only exists on `1247-el-dot-access`. So step (c)'s own commit(s) add the call-syntax machinery with no builtin functions registered yet (any call evaluates to a clear "unknown function" error) — `vec()` can't be registered here since `Vec3` doesn't exist on this branch. Once the machinery is done, "Introduce Vec3 type" gets rebased from `1247-el-dot-access` onto `1247-el-function-syntax`'s tip, and only then does a follow-up commit register `vec(x, y, z) -> Vec3` in the now-populated table and retrofit step (b)'s tests.

**Step (c) — call-syntax infrastructure, on `1247-el-function-syntax`. Done, uncommitted, verified.** Confirmed there is currently no `Name "(" ")"` production at all:
- `lib/TbElLib/include/el/Expression.h`: new `struct CallExpression { std::string name; std::vector<ExpressionNode> arguments; };`, added to the `Expression` variant (44-52) and forward-declared (35-42).
- `lib/TbElLib/src/Parser.cpp`: in `parseSimpleTerm()` (397-413), the `ElToken::Name` branch currently always calls `parseVariable()` — change it to peek one token past the name for `OParen`; if present, consume it and call a new `parseCall()` (parses a comma-separated `Expression` list until `CParen`, matching `parseGroupedTerm()`'s style at 348-361); otherwise fall through to `parseVariable()` as today.
- `lib/TbElLib/src/Expression.cpp`: `evaluate()`/`optimize()`/stream/equality overloads for `CallExpression` (pattern of the `SubscriptExpression` overloads, ~1122-1134); the evaluator looks up `name` in a builtin table, arity- and type-checked, throwing a clear error for an unknown name or wrong arity/types (a typo'd function name is a genuine mistake, not a "missing field"). The table starts **empty** on this branch — no function can be registered yet, since none of `vec`/`bbox`/`distance`/`intersects` exist without their types — so every call at this stage evaluates to a clear "unknown function" error; that's exactly what proves the mechanism is wired up correctly.
- Tests: `tst_Parser.cpp` (call-syntax parsing — arities, nested calls, calls composing with other operators and with step (a)'s dot-access), `tst_Expression.cpp` (evaluating any call name to the "unknown function" error, proving the dispatch path executes).

**Step (c′) — rebase + register `vec()`, back on the `Vec3` lineage.** Once step (c) is done: rebase "Introduce Vec3 type" (currently on `1247-el-dot-access`) onto `1247-el-function-syntax`'s tip — it doesn't touch call-syntax code, so this should be a clean replay. Then, as a new commit on top:
- Register `vec(x, y, z) -> Vec3` in step (c)'s now-existing builtin table.
- Retrofit: update step (b)'s `tst_Expression.cpp` "Vec3" section to construct test values via `vec(1, 2, 3)` expression source directly instead of bound variables, matching how a real query author would actually write these — the bound-variable workaround is no longer necessary once this lands.
- Tests: `tst_Expression.cpp` (`vec()` construction, `vec(1,2,3).x`-style chaining with dot sugar).

**Step (d) — `BBox` value type + `bbox()` constructor, together. Done, uncommitted, verified.** Same exhaustive-switch treatment as step (b), built on top of `Vec3` (holds a `min`/`max` `Vec3` pair, backing `vm::bbox3d`) — bundled with its constructor from the start this time, rather than repeating step (b)'s test-ergonomics gap:
- Type plumbing mirrors step (b) exactly (`ValueType::BBox`, `BBoxType = vm::bbox3d`, variant/ctor/accessor, `convertibleTo`/`convertTo`/`appendToStream`/`operator==`, string-keyed subscript for `b["min"]`/`b["max"]`/`b.min`/`b.max`).
- `bbox(min, max) -> BBox` registered in step (c)'s builtin table, normalizing corners component-wise (`vm::vec3d`'s `min`/`max` free functions) so argument order doesn't matter.
- Tests follow step (c)'s pattern: `bbox(vec(...), vec(...))` expression source directly, no bound-variable workaround needed.
- `vm::bbox3d` only defines `==`/`!=`, no ordering, but `evaluateCompare` is one shared dispatch for all six comparison operators (`<`/`<=`/`>`/`>=`/`==`/`!=`) rather than separate paths — so `BBox` gets a `compareAsBBoxes` helper defining an arbitrary but total lexicographic order over `(min, max)` purely to satisfy that shared protocol; equality still matches `vm::bbox3d`'s own `operator==` exactly. `BBox` has no unary or arithmetic operators (only construction, `min`/`max` subscript, and comparison) — unlike `Vec3`, which forwards several operators to `vm::vec3d`.

**Step (e) — `distanceTo()`/`intersects()` functions. Done, uncommitted, verified.** Small addition once both types exist: `distanceTo(a, b) -> Number` (Euclidean distance between two `Vec3`, via `vm::distance`; named `distanceTo` rather than `distance` — renamed during step (f-i)'s design work, since a plain `distance` read awkwardly once used to motivate the infix-call precedence fix, e.g. `x distanceTo y == z distanceTo x`), `intersects(a, b) -> Boolean` (`BBox`/`BBox` overlap, via `vm::bbox3d`'s existing `intersects()`). Tests: `tst_Expression.cpp`, straightforward given the prior steps' patterns (success cases including order-symmetry, plus arity errors for each).

**Step (f) — `like`/`contains` (originally `in`), as functions plus generic infix-call sugar.** Revised three times now: (1) originally, dedicated `BinaryOperation::Like`/`In` enum members with their own keyword tokens; (2) generalized into a fully generic infix-call mechanism, but modeled as a new `InfixCallExpression` AST node (mirroring `DotExpression`'s precedent), requiring a small generalization of `rebalanceByPrecedence()`'s child-rotation logic; (3) — current — reusing `BinaryExpression` itself instead of a new node type (so the rebalancer needs no changes at all, since it already dispatches generically on `BinaryExpression`/`.operation`), and, on further review, converting `BinaryOperation` from a plain enum into a `std::variant` of per-operation structs rather than bolting an always-mostly-unused `functionName` field onto `BinaryExpression` — see "Infix call sugar" above for the full design and why this is the more type-safe encoding (an invalid state — a non-`InfixCall` node carrying a function name — becomes unrepresentable, not just unused-by-convention). Split into three independently-reviewable sub-steps, with the pure refactor deliberately first and separate from any new capability:

**Step (f-0) — refactor `BinaryOperation` from `enum class` to `std::variant`. Pure, mechanical, behavior-preserving — no new capability, no test additions, only test-call-site updates. Done, uncommitted, verified.**
- `lib/TbElLib/include/el/Expression.h`: replace `enum class BinaryOperation { Addition, ... Case };` with a nested `namespace binop` holding one struct per existing enumerator (`kdl_reflect_inline_empty(Name)` in each, as in "Infix call sugar" above — this macro already exists in `lib/KdLib/include/kd/reflection_decl.h` for exactly this "empty struct still needs `==`/`<<`" case; note the `BoundedRange` operation-tag struct must live in this nested namespace specifically to avoid colliding with the unrelated top-level `el::BoundedRange` struct `RangeType` already uses) and `using BinaryOperation = std::variant<binop::Addition, ..., binop::Case>;` (no `InfixCall` yet — that's step (f-i)).
- `lib/TbElLib/include/el/Expression.h`: each tag struct also gets a `static constexpr size_t precedence = N;` member matching its old enumerator's spot in the original precedence table (e.g. `Multiplication`/`Division`/`Modulus` at 12 down to `Case` at 1).
- `lib/TbElLib/src/Expression.cpp`: convert the `switch (operation) { case BinaryOperation::X: ...; switchDefault(); }` blocks for `evaluateBinaryExpression` and `operator<<` for `BinaryExpression` into `std::visit(kdl::overload(...), operation)` — same per-operation bodies, same exhaustiveness guarantee (a missing alternative fails to compile), just the variant idiom instead of the enum one, matching how `Value`/`Expression`/`RangeType` are already dispatched elsewhere in this file. `precedence(const BinaryOperation&)` becomes a one-line `std::visit([](const auto& op) { return op.precedence; }, operation)`, reading each struct's own member instead of re-listing every operation in a second table. `operator==(const BinaryExpression&, const BinaryExpression&)` needs no change.
- `lib/TbElLib/src/Parser.cpp`: the `TokenMap` in `parseCompoundTerm` (currently `std::unordered_map<ElToken::Type, BinaryOperation>` mapping to bare enumerators) changes each value from `BinaryOperation::Addition` to `BinaryOperation{binop::Addition{}}` (20 entries); the two direct `BinaryOperation::BoundedRange` uses (`parseExpressionOrBoundedRange`/`parseExpressionOrAnyRange`) get the same treatment.
- `lib/TbElLib/test-utils/src/TestUtils.cpp`: all ~20 binary-operator builder functions (`add`, `sub`, `mul`, ... `cs`) update their `BinaryExpression{BinaryOperation::X, ...}` construction the same way.
- `lib/TbElLib/test/src/tst_Expression.cpp`: 3 assertions directly construct `BinaryExpression{BinaryOperation::X, ...}` rather than through a `TestUtils.h` builder (an `operator!=` test) — same mechanical update; no new assertions needed, this step doesn't add capability.
- `lib/TbMdlLib/src/LegacyModelDefinitionParser.cpp` — the **one** call site outside `TbElLib` (confirmed via repo-wide grep before finalizing this design): builds `Case`/`Equal` expressions for legacy `.def`/`.fgd` model definitions, same mechanical update.
- **Verification**: run the **full** existing `TbElLibTest` and `TbMdlLibTest` suites — expect zero test *changes* beyond updating the enumerator-construction spelling at the handful of direct-construction sites above (everything going through `TestUtils.h` builders needs no test-file changes at all, since the builders' own signatures don't change), and zero behavioral differences anywhere, since this step is a pure representation change. This is the safety net that confirms the refactor is truly behavior-preserving before any new capability is layered on top of it.

**Step (f-i) — add `InfixCall` as one more `BinaryOperation` alternative. Done, uncommitted, verified.** Proven using the functions that already exist (`distanceTo`, `intersects`) rather than `like`/`in` (which don't exist yet — and `in` itself was later renamed to `contains` with swapped argument order, see the correctness/naming notes above and in step (f-ii) below) — this keeps the grammar work and the `like`/`contains` semantics work in separate, separately-verifiable commits. Small, given (f-0) already did the structural work:
- `lib/TbElLib/include/el/Expression.h`: add `struct InfixCall { std::string functionName; kdl_reflect_decl(InfixCall, functionName); };` to `namespace binop`; add it to the `BinaryOperation` variant.
- `lib/TbElLib/include/el/Expression.h`: `struct InfixCall` also gets `static constexpr size_t precedence = 13;` (binds tighter than every real operator — see the correctness note above), using `kdl_reflect_inline(InfixCall, functionName)` (the non-empty variant of the macro pair used in (f-0), matching e.g. `gl::ResourceFailed` in `Resource.h`) rather than a split `kdl_reflect_decl`/`kdl_reflect_impl`, so no `.cpp`-side reflect boilerplate is needed.
- Tests must cover precedence against multiple operator tiers, not just one — `+`/`*` (arithmetic), `&&` (logical), and `==` itself (the tier `like`/`contains` might naturally be mistaken for) — since the whole point is that `InfixCall` dominates *every* tier uniformly, and a single passing example wouldn't distinguish "always wins" from "happens to win against this one operator."
- `lib/TbElLib/src/Expression.cpp`: add the `binop::InfixCall` case to the two remaining `std::visit`s from (f-0) — `precedence` needs no new case at all, since it already just reads `op.precedence` generically; `evaluateBinaryExpression` routes to `evaluateCall(context, infixCall.functionName, {evaluateLhs(), evaluateRhs()}, expressionNode)`; `operator<<` prints `lhs.leftOperand << " " << infixCall.functionName << " " << lhs.rightOperand`.
- `lib/TbElLib/include/el/Parser.h`: add `Name` to the `ElToken::CompoundTerm` mask.
- `lib/TbElLib/src/Parser.cpp`, `parseCompoundTerm`: check for `ElToken::Name` before the existing `TokenMap` lookup (which only knows fixed operator-symbol tokens); when found, consume it and build `BinaryExpression{BinaryOperation{binop::InfixCall{token.data()}}, std::move(lhs), parseSimpleTermOrSwitch()}` instead of looking it up in `TokenMap` — this replaces today's "Unhandled binary operator" throw for that case, which currently can't be reached by a `Name` token since `Name` isn't yet in `CompoundTerm`.
- `lib/TbElLib/test-utils/include/el/TestUtils.h` + `src/TestUtils.cpp`: add an `infixCall(std::string name, ExpressionNode lhs, ExpressionNode rhs)` builder (pattern of `add`/`sub`/etc., constructing `BinaryOperation{binop::InfixCall{std::move(name)}}`) for use in parser/expression tests.
- Tests, proving the mechanism with functions that already exist:
  - `tst_Parser.cpp`: `a distance b` parses to `infixCall("distance", a, b)`; composes with prefix calls (`distance(a, b) distance c`).
  - `tst_Expression.cpp`: `vec(...) distanceTo vec(...)` evaluates identically to the prefix form; `asString()` round-trips as `a like b`, not `like(a, b)`; **precedence interactions are the important new coverage here** — e.g. `a distanceTo b == c distanceTo d` evaluated against the equivalent two-prefix-calls form, confirming each infix call is a fully self-contained unit that `==` compares the *results* of, not something `==` (or any other operator) reaches into (this is the test that caught the original tier-8 design mistake, via exactly this shape — see the correctness note above). An unregistered name used infix (`a foo b`) still produces the same "Unknown function: 'foo'" error a mistyped prefix call already gives.
  - `tst_Parser.cpp`: structural precedence coverage across multiple tiers, not just one — `1 + 2 like 3 + 4`, `1 * 2 like 3 * 4`, `a && b like c && d`, and even `a like b == c` / `a == b like c == d` (the tier `like`/`contains` might naturally be mistaken for) — each confirming `like` grabs only its own two immediate neighbors regardless of which operator surrounds it.

**Step (f-ii) — register `like`/`contains` as builtin functions. Done, uncommitted, verified.** No parser work at all, just two entries in `Expression.cpp`'s `builtinFunctions` table (same table `vec`/`bbox`/`distanceTo`/`intersects` already live in), plus two dedicated `evaluateLike`/`evaluateContains` free functions (placed alongside them, since their type-dispatch logic is meaningfully more involved than the other builtins' single-purpose bodies):
- `like(lhs, rhs) -> Boolean`: `String like String` via `kdl::ci::str_matches_glob` (`lib/KdLib/include/kd/string_compare.h`) — wrap `rhs` in `*...*` first if it contains none of the glob metacharacters (`?`, `*`, `%`), to get the spec's "no wildcards → substring" behavior; `Array like String` → true if `str_matches_glob` (with the same wildcard-wrapping) matches any element; `Undefined` on either side → `false`, never throws.
- `contains(lhs, rhs) -> Boolean`: originally implemented as `in(lhs, rhs)` with `lhs` the element and `rhs` the container (`X in Array`, matching the original design's operator framing); renamed to `contains` with the arguments swapped — `lhs` the container, `rhs` the element — right after landing, since it reads backwards once generalized: the user wanted `x distanceTo y == z distanceTo x`-style reading, and `tags contains "Detail"` / `bbox(...) contains point` read naturally the way `"Detail" in tags` / `point in bbox(...)` didn't once `in` stopped being a dedicated operator and became just another two-argument function. Dispatches on `lhs`'s type — `Array` → element membership (reuse `Value::operator==` per element); `Map` → key membership (`lhs.contains(context, rhs.stringValue(context))`, guarding the type check first — note this calls the *existing*, unrelated `Value::contains` member function, not the builtin); `Range` → numeric membership; `BBox` → `lhs.bboxValue(context).contains(rhs.vec3Value(context))` if `rhs` is `Vec3`, else `lhs.bboxValue(context).contains(rhs.bboxValue(context))` if `rhs` is also `BBox` (both via `vm::bbox3d`'s existing `contains()` overloads); anything else, or `Undefined` on either side → `false`, never throws.
- Tests: `tst_Expression.cpp`, both prefix (`like(a, b)`/`contains(a, b)`) and infix (`a like b`/`a contains b`) forms for each case in the spec's semantics table (String/String, Array/String for `like`; Array/Map/Range/BBox-contains-Vec3/BBox-contains-BBox for `contains`), plus the never-throws-on-`Undefined` guarantee for both, plus arity errors (consistent with every other builtin). Discovered while writing the `Range` cases: there's no standalone range-literal syntax outside array/subscript context (`1..10` alone doesn't parse — the `..` operator is only reachable via `parseExpressionOrBoundedRange`/`parseExpressionOrAnyRange`, used inside array-element parsing; the `TokenMap` entry for `ElToken::Range` in `parseCompoundTerm` is unreachable dead code, since `Range` was never added to the `CompoundTerm` mask), so these tests reach a `Range` value the same way `tst_Expression.cpp` already does elsewhere: a bound variable, not a literal.

> **Done.** The manual at `app/TrenchBroom/resources/documentation/manual/index.md` was updated commit-by-commit, as `fixup!` commits later squashed into each original commit (dot-access, call syntax, Vec3, BBox, infix notation, distanceTo/intersects, like/contains) — new sections "Dot Access", "Function Calls", "Infix Notation", "Vec3", "BBox", "Pattern Matching", "Membership Testing", plus `Vec3`/`BBox` rows in the Evaluation types table and Type Conversion notes, and a corrected "Infix function call" row in the Binary Operator Precedence table (previously a vague unlabeled "Other operators" placeholder).

### Follow-up (small, immediate) — teach `ModelDefinition`'s scale handling about `Vec3`

**Context**: now that `Vec3` is a real `el::Value` alternative (PR2 step (b)/(c′)), it can appear anywhere a general EL expression is evaluated — including a `.fgd`/`.def` model definition's `scale` key and a game config's default scale expression (`app/.../index.md:3036-3054`). `lib/TbMdlLib/src/ModelDefinition.cpp`'s `scaleValue()` helper (91-123), which both `convertToScale` call sites feed into (the per-model `scale` key at line 199, and the game-config default scale expression at line 228), only recognizes `Number` (uniform scale) and `String` (parsed as either a number or a `"x y z"` triplet via `vm::parse<double, 3>`) — a `Vec3` value (e.g. written directly as `vec(1, 2, 3)`, or produced by any expression that evaluates to one) falls through both checks and returns `std::nullopt`, silently discarding a perfectly good scale value.

**Fix**: in `scaleValue()`, add a `Vec3` branch returning the value directly:
```cpp
if (value.type() == el::ValueType::Vec3)
{
  return value.vec3Value(context);
}
```
placed before the existing `Number` check (91-98) — same early-return style as that branch, no other change needed. (`ModelDefinition::scale()`'s own outer switch at 194-222 needs no change: it only special-cases `Map` to pull out the `Scale` sub-value and hand it to `convertToScale`/`scaleValue`; a bare `Vec3` at the top level of a *model* expression isn't a valid model spec any more than a bare `Number` is today, so that switch's existing `Vec3` fallthrough-to-`break` is correct as-is.)

**Tests**: extend the `GENERATE` table in `lib/TbMdlLib/test/src/tst_ModelDefinition.cpp`'s `"scale"` section (129-166) with two rows following the existing pattern:
- `{R"({ path: "maps/b_shell0.bsp", skin: 1, frame: 2, scale: vec(1, 2, 3) })", std::nullopt, vm::vec3d{1, 2, 3}}` — a `Vec3` in the per-model `scale` key.
- `{R"("maps/b_shell0.bsp")", R"(vec(2, 3, 4))", vm::vec3d{2, 3, 4}}` — a `Vec3` as the game config's default scale expression.

**Manual**: update `app/TrenchBroom/resources/documentation/manual/index.md`'s `scale` key table (3049-3054) with a new row documenting the `Vec3` form, e.g. `` `"scale": vec(1, 2, 3)` `` → "A fixed non-uniform scale factor scaling X by 1, Y by 2 and Z by 3, using a `Vec3` value directly." — parallel to the existing `"1 2 3"` string-triplet row just above it, since both now do the same thing.

**Verification**: `cmake --build <build-dir> --target TbMdlLibTest --parallel`, run the resulting test binary, clang-format the changed `.cpp`. No other file needs to change — this is a self-contained, single-function fix plus its tests and docs.

**Verification**: `cmake --build <build-dir> --target TbElLibTest --parallel`, all green after each of the three sub-steps — and also `TbMdlLibTest` after (f-0) specifically, since that step's one external call site lives there. No app-level behavior change from any step in this PR — it only adds unused-so-far language capability.

### PR 3 — QL machinery: `BoundValue` foundation, then type/domain inference + search evaluator

Depends on PR 1 (safe chaining) and PR 2 (`like`/`contains`/`Vec3`/`BBox`/dot-access). No UI yet.

**Revised, mid-PR3, per explicit decision**: the first version of this PR (fully built and tested, then discarded on a fresh branch) bound the node/face schema via hand-written `el::VariableStore` subclasses with eager `Map`-building `entity`/`properties` fields. That's replaced by building on the new `BoundValue` mechanism (see "`BoundValue`: a lazy, descriptor-driven `Value` alternative" above, in the EL-constructs section) — itself now a prerequisite sub-step of this PR, done first, in `lib/TbElLib`, since it's a general EL capability, not search-specific (matching how `Vec3`/`BBox`/`like`/`contains` were framed in PR 2). `QueryDomainInference` does **not** depend on `VariableStore`/`BoundValue` at all — it only walks `el::ExpressionNode` — so it is unaffected by this revision and carries forward unchanged from before.

**Step 0 — `BoundValue` in `TbElLib`.** New `el::Value` alternative plus the generic `el::VariableStore`-adapter, per the full design above. Also migrate `mdl::EntityPropertiesVariableStore` onto it (the one existing, shipped store this is a clean fit for), leaving `VariableTable`/`NullVariableStore`/the `ui::CompilationVariables` family untouched (rationale above: they're small, real, mutable key/value bags, not lazy projections of a bigger object — `BoundValue` doesn't help there).
- Files: `lib/TbElLib/include/el/Types.h`/`src/Types.cpp` (new `ValueType`), `lib/TbElLib/include/el/Value.h`/`src/Value.cpp` (variant + every touch point catalogued above), `lib/TbElLib/src/Expression.cpp` (the `evaluateSubscript`/`evaluateContains`/exhaustive-`break`-group touch points catalogued above), a new small adapter (`el::VariableStore` implementation wrapping one `BoundValue`) — exact filename TBD at implementation time, and `lib/TbMdlLib/src/EntityPropertiesVariableStore.cpp` (rewritten to build a "dynamic schema" descriptor over `Entity::property`/`propertyKeys`, preserving its existing `Value{""}`-on-miss quirk exactly).
- Tests: `tst_Types.cpp`/`tst_Value.cpp` (every existing exhaustive section gets a `BoundValue` row, same pattern `Vec3`/`BBox` followed in PR 2), `tst_Expression.cpp` (subscript/dot-access into a `BoundValue`, `contains`, unsupported-operator error cases, reference-equality), a new `tst_EntityPropertiesVariableStore.cpp` if one doesn't already exist (confirm during implementation) proving the migration is behavior-preserving — same inputs, same outputs, including the on-miss quirk.
- **Verification**: `TbElLibTest` and `TbMdlLibTest` both green, with the migration step specifically checked for zero behavioral difference (mirrors how PR 2 step (f-0)'s `BinaryOperation` refactor was verified as pure/behavior-preserving before new capability was layered on).

**Step 1 — rebuild `NodeVariableStore`/`FaceVariableStore` on `BoundValue`.** Per-concrete-kind `static const` descriptor tables (one per `WorldNode`/`LayerNode`/`GroupNode`/`EntityNode`/`BrushNode`/`PatchNode`, and one for the face schema) replace the old `kdl::overload`-per-field-group visitors — a field that doesn't apply to a kind is simply **absent from that kind's table**, so the old design's explicit `-> el::Value::Undefined` branches disappear entirely. `NodeVariableStore`'s constructor does the one-time `node.accept(kdl::overload(...))` dispatch to pick the right descriptor and build a single `BoundValue{&node, &map, descriptor}` (the `context` pointer carries the owning `Map&`, needed by the `tags` field's `map.smartTags()` lookup); `value(name)` becomes a one-line forward into it. `entity`/`properties` become genuinely lazy: `entity`'s accessor function returns `Value{BoundValue{&entityNodeBase->entity(), nullptr, &EntityDescriptor}}` (a further "dynamic schema" descriptor over `Entity::property`/`propertyKeys`, the same shape used for the migrated `EntityPropertiesVariableStore`) rather than eagerly copying every property. All prior research into the underlying `Node`/`Object`/`Entity`/`Brush`/`Tag` APIs (accessor names, `Object` vs. `Node` split, `BrushNode::entity()` returning worldspawn for structural geometry, `map.smartTags()`+`hasTag()` for tags, `Brush::faces()`+dedup for `materials`) carries forward unchanged — only *how* the fields are exposed (descriptor table + `BoundValue`, vs. hand-written `value()` if/else) changes.
- Files: `NodeVariableStore.h`/`.cpp`, `FaceVariableStore.h`/`.cpp` (no separate `QueryVariableValues.h`/`.cpp` needed this time — the shared `entity`/`tags`/`layerName`/`groupName` logic naturally lives as shared descriptor-building helper functions, reused by both the per-kind node descriptors and the face descriptor, without needing a whole extra module split out reactively).
- Tests: re-derive `tst_NodeVariableStore.cpp`/`tst_FaceVariableStore.cpp` from the previous (now-discarded) versions' coverage — same field × node-kind matrix, same structural-brush-owned-by-worldspawn case, same face-selected-independent-of-brush-selected case — since the *external behavior* of both stores is unchanged, only their internal construction.

**Step 2 — `QueryDomainInference.h`/`.cpp`.** Unchanged from before, since it depends only on `el::ExpressionNode`, not `VariableStore`/`Value` internals — the field→kind-set table and the AND-intersects/OR-unions/NOT-passthrough algorithm.

  > **Design carries forward as-is** (originally verified against source and tested before the branch reset; re-add this file and its tests un-modified, or confirm it's already present on the new branch before starting Step 0/1 above). `QueryKind` is a 7-value enum (the six node kinds plus `Face`); `QueryDomain = std::set<QueryKind>`. Key refinement over the spec's literal field table: fields with no domain-narrowing effect of their own (`type`, `bounds`, `center`, `layerName`, `groupName`, `visible`, `locked`, `selected`, `linked`) map internally to the *full 7-kind universe* (including `Face`), not "the six node kinds" — otherwise a field like `bounds` would spuriously conflict with a genuine face-domain signal elsewhere in the same expression (e.g. `type == "face" && intersects(bounds, ...)`). The "no signal at all → six node kinds, never face" default is applied exactly once, as a post-processing step on the *final* top-level result, not per-field. `SwitchExpression` cases and `LogicalOr` both union; every other composite node intersects; `UnaryExpression`/`DotExpression` pass through their single operand unchanged (giving negation its no-op behavior for free). Tests: `tst_QueryDomainInference.cpp`, a table-driven test covering every worked example from this spec plus union/unsatisfiable/negation-passthrough cases, 16 assertions, previously all green.

**Step 3 — `Query.h`/`.cpp`.** The entry point tying it together — `parseQuery(text)` (wrapping EL's existing top-level parse entry, catching `el::Exception` for malformed input), `executeQuery(Map&, const el::ExpressionNode&)` returning a result type covering both shapes:
```cpp
using QueryResult = std::variant<std::vector<Node*>, std::vector<BrushFaceHandle>>;
```
Node-domain execution uses `mdl::collectNodesAndDescendants(std::vector<Node*>{&map.worldNode()}, predicate)` (`NodeQueries.h:391`) — note the predicate is invoked per **concrete** type (`WorldNode&`, `LayerNode&`, ... — `if constexpr (std::is_invocable_r_v<bool, Predicate, ConcreteType&>)` gates each), so it must be a single generic callable (e.g. a lambda templated on `auto&`) that upcasts to `const Node&` to build a `NodeVariableStore` and evaluate via `el::withEvaluationContext`; the direct precedent for this iteration shape is `selectEntitiesWithClassname`/`selectBrushesWithMaterial` in `Map_Selection.cpp`. Face-domain execution uses `mdl::collectBrushFaces(std::vector<Node*>{&map.worldNode()}, predicate)` (`NodeQueries.h:475-509`), whose predicate signature is fixed as `(const BrushNode&, const BrushFace&) -> bool` — build a `FaceVariableStore` per pair. Both helpers are header-only templates in `NodeQueries.h`, so no new `CMakeLists.txt` entry is needed for them specifically. An empty inferred kind-set short-circuits to an empty result without iterating either helper.

**Build wiring**: add the new files to `lib/TbMdlLib/CMakeLists.txt`'s explicit `target_sources` list (same style as `TbElLib/CMakeLists.txt`), and corresponding test files to `lib/TbMdlLib/test/CMakeLists.txt`. Same for the new `lib/TbElLib` files from Step 0.

**Tests**: unit tests for `NodeVariableStore`/`FaceVariableStore` field-by-field, `QueryDomainInference`'s table-driven worked-examples test (Step 2, unchanged), and integration tests building a small in-memory `Map` (mixed entities/brushes/layers/groups) asserting `executeQuery` returns exactly the expected `Node*`/`BrushFaceHandle` set for a handful of representative queries drawn from the spec's worked-examples lists.

**Verification**: new `mdl` test target(s) green, plus `TbElLibTest` green from Step 0. No manual/UI verification possible yet — that's PR 4.

### PR 4 — UI: search box and selection wiring

Depends on PR 3. This is the first user-visible change.

**New widget**: a `SearchPanel`/`SearchPopupEditor`, built as a sibling to `ViewPopupEditor` rather than added into `ViewEditor` itself — `ViewEditor.cpp` (835 lines) is self-contained and tightly coupled to `EditorContext` flags and `SmartTag`s, so a new, independent widget is cleaner than threading query state through it. Uses `ui::createSearchBox()` (`lib/TbUiLib/include/ui/SearchBox.h`) with the same idiom already used in `EntityBrowser.cpp`/`MaterialBrowser.cpp`:
```cpp
m_searchBox = createSearchBox();
connect(m_searchBox, &QLineEdit::textEdited, this, [&]() {
  runQuery(m_searchBox->text().toStdString());
});
```
`runQuery` calls PR 3's `parseQuery`/`executeQuery` against `document.map()`, then:
- Node results: `mdl::deselectAll(map); mdl::selectNodes(map, matchingNodes);` (`Map_Selection.h`).
- Face results: `mdl::deselectAll(map); mdl::selectBrushFaces(map, matchingHandles);` (`Map_Selection.h`).
- Parse errors: shown inline (e.g. a small error label under the box, or the box's own error/invalid styling) rather than silently falling back to fuzzy text — per this spec's Tier-1/Tier-2 detection rule, a parse failure on input that *looks* like a query (contains structural tokens) is a mistake to surface, not a name to fuzzy-match.

**Placement**: mount alongside the existing `ViewPopupEditor` in `MapViewBar.cpp` (`m_viewEditor = new ViewPopupEditor{document};`, line 54), or behind a dedicated menu command if a persistent bar entry is too cramped — a UI-only decision to make once the widget exists, not a blocker for the rest of this plan.

**Menu/shortcut**: register a new `Action` in `ActionManager.cpp` (pattern of the `Select All` action at 1246-1258: preference path, label, `ActionContext`, default `KeySequence`, `ExecuteFn`, `EnabledFn`), e.g. `Menu/Edit/Find`, focusing the search box on trigger. Confirm the chosen shortcut isn't already bound elsewhere before picking one.

**Tests**: whatever UI test coverage this codebase already has for similar widgets (confirm pattern during implementation — `EntityBrowser`/`MaterialBrowser` filter behavior may or may not have existing UI tests to mirror).

**Verification**: build the full app (`cmake --build <build-dir> --target TrenchBroom --parallel`); run the worked examples from this spec's "Worked examples" sections against a real `.map` file and confirm: node selection updates for Tier 2 entity/brush/spatial queries, face selection updates for face-domain queries, bare-word input still does Tier 1 fuzzy matching, and a malformed query (e.g. unbalanced parens) shows an inline error rather than silently selecting nothing. Per established workflow, build to verify but don't launch the app automatically — hand off to manual testing.

### Open questions carried into implementation

Whether `contains`-style spatial containment vs. `intersects` overlap is the more intuitive default for a future marquee-style "select in box" UI; whether case sensitivity of `like` should be configurable; performance for very large maps (likely fine — linear scan with early-exit boolean short-circuiting, same cost class as the existing issue browser, and narrowed further by type inference); exact placement/shortcut for the PR 4 search box; whether `NodeVariableStore`/`FaceVariableStore` need any caching (e.g. memoizing tag-name enumeration) if profiling shows it matters on very large maps. (Updating the manual for this PR's EL additions is no longer an open question — see the reminder right after step (f-ii) above.)

**No longer deferred**: the optimization idea originally raised here — `NodeVariableStore`'s `entity`/`properties` fields eagerly copying the owning entity's full property map on every bind, expensive for an entity with many owned brushes — was first postponed pending real profiling, then, per an explicit later decision, pulled forward and generalized into the `BoundValue` mechanism (see "`BoundValue`: a lazy, descriptor-driven `Value` alternative" in the EL-constructs section, and PR 3's revised Step 0/Step 1 above), which also replaces `mdl::EntityPropertiesVariableStore`'s hand-written implementation. This entry is kept only as a historical pointer to why that design exists.

### Later: query builder UI

A structured builder (field/operator/value rows, AND/OR grouping, function picker with typed argument inputs) reading/writing the same textual query, so the text box and builder stay interchangeable — the inferred-type mechanism from PR 3 doubles as a natural driver for the builder's field picker (only show fields compatible with the kinds selected so far). Not scoped in detail here; a follow-up once the text language has real usage to learn from.
