# Contributing to TrenchBroom

As mentioned in the [README](README.md):

- Bug reports and feature suggestions are welcome. Please submit them
  at https://github.com/TrenchBroom/TrenchBroom/issues
- If you wish to contribute code or improve the documentation, please get in touch with me at kristian.duske@gmail.com.
- All help is appreciated!

## Suggesting a Feature

If you have an idea for a nice feature that you're missing in TrenchBroom, then you can submit a request at
the [TrenchBroom issue tracker]. Try to describe your feature, but don't go into too much detail. If it gets picked up,
we will hash out the details together.

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

## Code Changes

- Please split up changes into individual commits where you add each change step by step.
    - This makes pull requests much easier to review, and also makes it easier to judge whether there are tests missing.
    - Each individual commit should compile without errors, test green and the editor should be working correctly. Think
      about it as a series of small transformations that take the code from the before state to the goal state, with
      each step being correct and testable.
- If there are changes requested in the pull request, please create individual fixup commits for the changes so that
  they get squashed into the correct commits. I'd like to keep the history nice and clean instead of putting one larger
  cleanup commit on top.

[TrenchBroom issue tracker]: https://github.com/TrenchBroom/TrenchBroom/issues/
