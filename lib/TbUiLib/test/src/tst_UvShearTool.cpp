/*
 Copyright (C) 2026 Kristian Duske

 This file is part of TrenchBroom.

 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "gl/Camera.h"
#include "gl/Material.h"
#include "gl/OrthographicCamera.h"
#include "gl/Texture.h"
#include "gl/TextureResource.h"
#include "mdl/Brush.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushFace.h"
#include "mdl/BrushFaceHandle.h"
#include "mdl/BrushNode.h"
#include "mdl/EditorContext.h"
#include "mdl/LayerNode.h" // IWYU pragma: keep
#include "mdl/Map.h"
#include "mdl/MapFixture.h"
#include "mdl/Map_Nodes.h"
#include "mdl/Map_Selection.h"
#include "mdl/PickResult.h"
#include "mdl/WorldNode.h"
#include "ui/CatchConfig.h"
#include "ui/GestureTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/PickRequest.h"
#include "ui/ToolController.h"
#include "ui/UvShearTool.h"
#include "ui/UvViewHelper.h"

#include "kd/result.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

TEST_CASE("UvShearTool")
{
  // declared before the fixture, so it outlives (is destroyed after) the brush that
  // references it via a raw, non-owning pointer
  auto material =
    gl::Material{"material", gl::createTextureResource(gl::Texture{64, 64})};

  // shearing is only supported for the Valve UV coordinate system (ParaxialUvCoordSystem,
  // used by the default Standard format, treats shear as a no-op), so use the Quake
  // fixture config, which maps to MapFormat::Valve
  auto fixture = MapDocumentFixture{};
  auto& document = fixture.create(mdl::QuakeFixtureConfig);
  auto& map = document.map();

  auto builder = mdl::BrushBuilder{map.worldNode().mapFormat(), map.worldBounds()};
  auto brush = builder.createCuboid(vm::bbox3d{16.0}, "material") | kdl::value();
  const auto topFaceIndex = *brush.findFace(vm::vec3d{0, 0, 1});

  auto* brushNode = new mdl::BrushNode{std::move(brush)};
  mdl::addNodes(map, {{map.editorContext().currentLayer(), {brushNode}}});

  // addNodes resolves materials by name against the (empty, in this fixture) material
  // collection, which would clear a material set before insertion. Set it afterwards,
  // directly via setBrush, which does not touch material resolution.
  auto updatedBrush = brushNode->brush();
  updatedBrush.face(topFaceIndex).setMaterial(&material);
  brushNode->setBrush(std::move(updatedBrush));

  const auto faceHandle = mdl::BrushFaceHandle{brushNode, topFaceIndex};
  mdl::selectBrushFaces(map, {faceHandle});

  auto camera = gl::OrthographicCamera{
    1.0f,
    8000.0f,
    gl::Camera::Viewport{0, 0, 1024, 768},
    vm::vec3f{0, 0, 100},
    vm::vec3f{0, 0, -1},
    vm::vec3f{0, 1, 0}};
  auto helper = UvViewHelper{camera};
  helper.setFaceHandle(faceHandle);

  auto tool = UvShearTool{document, helper};
  auto& controller = static_cast<ToolController&>(tool);

  const auto inputStateFor = [&](const vm::ray3d& ray) {
    auto inputState = InputState{};
    inputState.setPickRequest(PickRequest{ray, helper.camera()});
    auto pickResult = mdl::PickResult{};
    controller.pick(inputState, pickResult);
    inputState.setPickResult(std::move(pickResult));
    return inputState;
  };

  // the UV coordinate system's own origin (offset 0, scale 1) is at world (0, 0, 16)
  // here, which is at least 16 units from any corner of the face, i.e. far enough from
  // the (corner) pivot origin to be accepted as a shear handle
  const auto gridOriginRay = vm::ray3d{{0, 0, 100}, {0, 0, -1}};

  SECTION("cancel always returns false")
  {
    CHECK(!controller.cancel());
  }

  SECTION("tool returns itself")
  {
    CHECK(&controller.tool() == static_cast<Tool*>(&tool));
  }

  SECTION("pick")
  {
    SECTION("with an invalid helper, nothing is hit")
    {
      auto emptyCamera = gl::OrthographicCamera{};
      auto emptyHelper = UvViewHelper{emptyCamera};
      auto emptyTool = UvShearTool{document, emptyHelper};
      auto& emptyController = static_cast<ToolController&>(emptyTool);

      auto inputState = InputState{};
      inputState.setPickRequest(PickRequest{gridOriginRay, emptyCamera});
      auto pickResult = mdl::PickResult{};
      emptyController.pick(inputState, pickResult);

      CHECK(pickResult.empty());
    }

    SECTION("hits both shear handles at a grid intersection")
    {
      const auto inputState = inputStateFor(gridOriginRay);
      CHECK(inputState.pickResult().all().size() == 2u);
    }

    SECTION("hits nothing away from any grid intersection")
    {
      const auto inputState = inputStateFor(vm::ray3d{{5, 5, 100}, {0, 0, -1}});
      CHECK(inputState.pickResult().empty());
    }
  }

  SECTION("acceptMouseDrag")
  {
    SECTION("returns nullptr without the Alt modifier")
    {
      auto inputState = inputStateFor(gridOriginRay);
      inputState.mouseDown(MouseButtons::Left);

      CHECK(controller.acceptMouseDrag(inputState) == nullptr);
    }

    SECTION("returns nullptr with the wrong mouse button")
    {
      auto inputState = inputStateFor(gridOriginRay);
      inputState.setModifierKeys(ModifierKeys::Alt);
      inputState.mouseDown(MouseButtons::Right);

      CHECK(controller.acceptMouseDrag(inputState) == nullptr);
    }

    SECTION("returns nullptr when no handle is hit")
    {
      auto inputState = inputStateFor(vm::ray3d{{5, 5, 100}, {0, 0, -1}});
      inputState.setModifierKeys(ModifierKeys::Alt);
      inputState.mouseDown(MouseButtons::Left);

      CHECK(controller.acceptMouseDrag(inputState) == nullptr);
    }

    SECTION("starts a drag with Alt and a handle hit")
    {
      auto inputState = inputStateFor(gridOriginRay);
      inputState.setModifierKeys(ModifierKeys::Alt);
      inputState.mouseDown(MouseButtons::Left);

      CHECK(controller.acceptMouseDrag(inputState) != nullptr);
    }

    SECTION("starts a drag with Alt+CtrlCmd and a handle hit")
    {
      auto inputState = inputStateFor(gridOriginRay);
      inputState.setModifierKeys(ModifierKeys::Alt | ModifierKeys::CtrlCmd);
      inputState.mouseDown(MouseButtons::Left);

      CHECK(controller.acceptMouseDrag(inputState) != nullptr);
    }

    SECTION("shearing changes the face's UV axes")
    {
      auto inputState = inputStateFor(gridOriginRay);
      inputState.setModifierKeys(ModifierKeys::Alt | ModifierKeys::CtrlCmd);
      inputState.mouseDown(MouseButtons::Left);

      auto tracker = controller.acceptMouseDrag(inputState);
      REQUIRE(tracker != nullptr);

      const auto uAxisBefore = faceHandle.face().uAxis();
      const auto vAxisBefore = faceHandle.face().vAxis();

      auto dragInputState = inputStateFor(vm::ray3d{{0, 10, 100}, {0, 0, -1}});
      dragInputState.setModifierKeys(ModifierKeys::Alt | ModifierKeys::CtrlCmd);
      CHECK(tracker->update(dragInputState));

      const auto uAxisAfterDrag = faceHandle.face().uAxis();
      const auto vAxisAfterDrag = faceHandle.face().vAxis();
      CHECK((uAxisAfterDrag != uAxisBefore || vAxisAfterDrag != vAxisBefore));

      SECTION("ending the drag keeps the change")
      {
        tracker->end(dragInputState);
        CHECK(faceHandle.face().uAxis() == uAxisAfterDrag);
        CHECK(faceHandle.face().vAxis() == vAxisAfterDrag);
      }

      SECTION("cancelling the drag reverts the change")
      {
        tracker->cancel();
        CHECK(faceHandle.face().uAxis() == uAxisBefore);
        CHECK(faceHandle.face().vAxis() == vAxisBefore);
      }
    }
  }
}

} // namespace tb::ui
