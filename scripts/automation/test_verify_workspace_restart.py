#!/usr/bin/env python3
"""Unit checks for the PW5 restart driver; no app process is launched."""

import importlib.util
from pathlib import Path
import sys
import unittest


SCRIPT = Path(__file__).with_name("verify_workspace_restart.py")
SPEC = importlib.util.spec_from_file_location("pw5_restart", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
PW5 = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = PW5
SPEC.loader.exec_module(PW5)


class RestartDriverTest(unittest.TestCase):
    def test_source_document_requires_exact_map_path(self) -> None:
        source = Path("/tmp/pw5/source.map")
        self.assertEqual(
            PW5.source_document_id(
                [{"id": "document-1", "path": str(source)}], source
            ),
            "document-1",
        )
        with self.assertRaises(PW5.VerificationError):
            PW5.source_document_id([{"id": "document-2", "path": "/tmp/other.map"}], source)

    def test_workspace_lookup_and_json_rpc_validation_are_strict(self) -> None:
        workspace = PW5.workspace_by_id(
            [{"workspaceId": "workspace-1", "runtimeStatus": "dormant"}], "workspace-1"
        )
        self.assertEqual(workspace["runtimeStatus"], "dormant")
        self.assertEqual(PW5.response_result({"jsonrpc": "2.0", "result": {}}, "test"), {})
        with self.assertRaises(PW5.VerificationError):
            PW5.response_result({"jsonrpc": "2.0", "error": {"code": -1}}, "test")


if __name__ == "__main__":
    unittest.main()
