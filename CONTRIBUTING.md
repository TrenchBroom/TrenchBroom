# Contributing to TrenchBroom

As mentioned in the [README](README.md):

- Bug reports and feature suggestions are welcome. Please submit them
  at https://github.com/TrenchBroom/TrenchBroom/issues
- If you wish to contribute code or improve the documentation, please get in touch with me at kristian.duske@gmail.com.
- All help is appreciated!

## Code Changes

- Please split up changes into individual commits where you add each change step by step.
    - This makes pull requests much easier to review, and also makes it easier to judge whether there are tests missing.
    - Each individual commit should compile without errors, test green and the editor should be working correctly. Think
      about it as a series of small transformations that take the code from the before state to the goal state, with
      each step being correct and testable.
- If there are changes requested in the pull request, please create individual fixup commits for the changes so that
  they get squashed into the correct commits. I'd like to keep the history nice and clean instead of putting one larger
  cleanup commit on top.
