# PW5 external restart verification

`scripts/automation/verify_workspace_restart.py` verifies the durable workspace path in
a real, separately launched TrenchBroom process. It is deliberately opt-in and defaults
to a dry run:

```sh
python3 scripts/automation/verify_workspace_restart.py --dry-run
```

On macOS, run it against built artifacts and a known-good saved map:

```sh
python3 scripts/automation/verify_workspace_restart.py --run \
  --app build-root-automation/app/TrenchBroom/TrenchBroom.app \
  --tbctl build-root-automation/app/TbCtl/tbctl \
  --source-map /absolute/path/to/fixture.map
```

The driver copies the app bundle into a unique temporary directory, launches that copy
with `open -n -g -j` and `--portable`, and reads `tbctl`'s socket only from a copied
bundle's `Contents/MacOS/config/automation/*.json` discovery record. It never uses automatic socket
discovery, never activates a window, and only signals a discovered PID after confirming
that its command still belongs to the copied bundle.

The verification sequence is:

1. Open the copied source map, fork a workspace, checkpoint it, and close its branch.
2. Terminate only the verified private PID with `SIGTERM`.
3. Restart the same copied bundle, list the dormant workspace, recover it from its
   workspace ID alone, attach that source, and diff it.
4. Require an unchanged branch to have no conflicts or merge operations, then terminate
   the verified private PID and remove the temporary bundle/config/maps.

Pass `--keep-artifacts` to retain the isolated bundle after a run. If a private process
does not exit after `SIGTERM`, the driver refuses to force-kill it and retains artifacts
for diagnosis.

The no-launch unit harness can be run independently:

```sh
python3 scripts/automation/test_verify_workspace_restart.py
```
