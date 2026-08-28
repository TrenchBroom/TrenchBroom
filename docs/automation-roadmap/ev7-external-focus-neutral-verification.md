# EV7 external focus-neutral verification

`scripts/automation/verify_focus_neutral_rendering.py` is an opt-in macOS integration
driver for real-view and virtual-rendering isolation. It defaults to a no-launch dry run:

```sh
python3 scripts/automation/verify_focus_neutral_rendering.py --dry-run
```

Run it only with a built app bundle, matching `tbctl`, and a known-good saved map:

```sh
python3 scripts/automation/verify_focus_neutral_rendering.py --run \
  --app build-root-automation/app/TrenchBroom/TrenchBroom.app \
  --tbctl build-root-automation/app/TbCtl/tbctl \
  --source-map /absolute/path/to/fixture.map
```

The driver copies the app and two map copies into a unique temporary directory. It
launches only that app with `open -n -g -j` and `--portable`, reads `tbctl` discovery
only from the copied bundle, and checks that every discovered PID still belongs to the
copy before it sends `SIGTERM`. It never activates a desktop window or consults normal
automatic discovery. Pass `--keep-artifacts` to retain the isolated bundle and maps.

It records an explicit target document/3D-view camera and selection fingerprint, then
opens the second copied map through `reference.open` and requires it to be active in the
private instance. While that second document is foregrounded, the harness checks
`render.capture`, `render.pick`, real-view `context.capture`, and
`cameras.create`/`cameras.capture` against the target. Every response must echo the
requested document ID (and the real view or camera ID where applicable), publish its
declared image, and retain a revision. A fresh target context after every operation must
have exactly its original camera, selected nodes, and selected faces; the copied
foreground document must also remain active after every target operation.

Run the no-launch driver checks independently:

```sh
python3 scripts/automation/test_verify_focus_neutral_rendering.py
```
