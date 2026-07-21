# Contributing to TrenchBroom

- Bug reports and feature suggestions are welcome. Please submit them
  at https://github.com/TrenchBroom/TrenchBroom/issues
- If you wish to contribute code or improve the documentation, please create an issue, or
  take an existing issue and ping me on it. It is usually better to be aligned before
  submitting a pull request.
- All help is appreciated!

## Suggesting a Feature

If you have an idea for a nice feature that you're missing in TrenchBroom, then you can
submit a request at the [TrenchBroom issue tracker]. Try to describe your feature, but
don't go into too much detail. If it gets picked up, we will hash out the details
together.

## Reporting Bugs

You can submit bug reports at the [TrenchBroom issue tracker]. Be sure to include the following information:

- *TrenchBroom version*: e.g., "*"Version 2.0.0 f335082 D" see below
- *Operation system and version*: e.g. "Windows 7 64bit"
- *Crash report and the map file*: When TrenchBroom crashes, it saves a crash report and the map file automatically.
  These files are placed in the folder containing the current map file, or in your documents folder if the current map
  hasn't been saved yet. For example, if the map file you are editing has the name "rtz_q1.map", the crash report will
  be named "rtz_q1-crash.txt", and the saved map file will be named "rtz_q1-crash.map". Existing files are not
  overwritten - TrenchBroom creates new file names by attaching a number at the end. Please choose the files with the
  highest numbers when reporting a bug.
- *Exact steps to reproduce*: It is really helpful if you can provide exact info on how to reproduce the problem.
  Sometimes this can be difficult to describe, so you can attach screenshots or make screencasts if necessary. If you
  cannot reproduce the problem, please submit a bug report either way. The cause of the problem can often be deduced
  anyway.

## Submitting Pull Requests

> [!IMPORTANT]
> TrenchBroom is a hobby project for me. Reviewing pull requests is not my favorite thing
> to do in my free time. If a pull request is hard to review because we weren't aligned on
> the approach beforehand, or it is badly structured, or achieving the required level of
> quality would take too much effort, it is likely to be rejected. Please read the
> following points carefully BEFORE starting your work.

- Before you start working on a pull request, please file an issue or ping me on an
  existing issue so that we can discuss the approach and agree on it. This will
  significantly reduce churn and PR review effort later on.
- Before you make a pull request, please split up changes into individual commits where
  you add each change step by step. Don't touch code twice unnecessarily. Don't push
  commits to fix problems of your earlier commits. The history should be clean, as if you
  had perfectly planned it and executed the plan
  - This makes pull requests much easier to review, and also makes it easier to judge whether there are tests missing.
  - Each individual commit should compile without errors, test green and the editor should
    be working correctly. Think about it as a series of small transformations that take
    the code from the before state to the goal state, with each step being correct and
    testable.
  - Don't merge the master branch into your PR. Rebase your branch onto master instead.
- Add unit tests wherever possible, even if tests are missing. Add tests for existing code
  for bonus points.
- Please respond timely to review comments, even if you did not receive feedback in a
  timely fashion.
- If there are changes requested in the pull request, please create individual fixup
  commits for the changes so that they get squashed into the correct commits. I'd like to
  keep the history nice and clean instead of putting one larger cleanup commit on top.

[TrenchBroom issue tracker]: https://github.com/TrenchBroom/TrenchBroom/issues/

## AI-Assisted Contributions

The use of AI-assisted development tools is permitted. Contributors remain solely
responsible for all submitted code, including code generated or modified with AI
assistance.

By submitting a contribution, you assert that:
- you have reviewed and understood the contribution,
- you are capable of explaining how it works,
- your contribution complies with TrenchBroom's license requirements,
- your contribution does not knowingly incorporate incompatible third-party code,
- you accept responsibility for defects, security issues, and licensing problems in the
  submitted code.

Please do not submit code that you do not understand.