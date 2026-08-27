#!/usr/bin/env python3
"""No-launch unit checks for the EV7 focus-neutral external verifier."""

import importlib.util
from pathlib import Path
import sys
import unittest


SCRIPT = Path(__file__).with_name("verify_focus_neutral_rendering.py")
SPEC = importlib.util.spec_from_file_location("ev7_focus_neutral", SCRIPT)
assert SPEC is not None and SPEC.loader is not None
EV7 = importlib.util.module_from_spec(SPEC)
sys.modules[SPEC.name] = EV7
SPEC.loader.exec_module(EV7)


class FocusNeutralDriverTest(unittest.TestCase):
    def test_document_active_and_view_lookup_are_explicit(self) -> None:
        target = Path("/tmp/ev7/target.map")
        foreground = Path("/tmp/ev7/foreground.map")
        documents = [
            {"id": "document-target", "path": str(target), "active": False},
            {"id": "document-foreground", "path": str(foreground), "active": True},
        ]
        self.assertEqual(EV7.document_id(documents, target), "document-target")
        self.assertEqual(EV7.active_document_id(documents), "document-foreground")
        self.assertEqual(
            EV7.three_d_view_id(
                [{"documentId": "document-target", "viewId": "view-3d", "type": "3d"}],
                "document-target",
            ),
            "view-3d",
        )
        with self.assertRaises(EV7.VerificationError):
            EV7.document_id(documents, Path("/tmp/ev7/missing.map"))

    def test_target_state_and_render_contract_are_strict(self) -> None:
        context = {
            "camera": {"position": [1, 2, 3]},
            "selectedNodes": [{"path": [0, 1]}],
            "selectedFaces": [],
        }
        baseline = EV7.target_state(context)
        self.assertEqual(EV7.target_state(context), baseline)
        changed = dict(context)
        changed["selectedNodes"] = []
        self.assertNotEqual(EV7.target_state(changed), baseline)
        request = EV7.render_request("document-target")
        self.assertEqual(request["documentId"], "document-target")
        self.assertEqual(request["outputs"], {"depth": False})
        EV7.require_render_result(
            {"documentId": "document-target", "revision": 7},
            "document-target",
            "test",
        )
        with self.assertRaises(EV7.VerificationError):
            EV7.require_render_result(
                {"documentId": "document-foreground", "revision": 7},
                "document-target",
                "test",
            )

    def test_active_lookup_rejects_ambiguous_foreground(self) -> None:
        with self.assertRaises(EV7.VerificationError):
            EV7.active_document_id(
                [{"id": "document-a", "active": True}, {"id": "document-b", "active": True}]
            )


if __name__ == "__main__":
    unittest.main()
