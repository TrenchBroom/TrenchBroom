/*
 Copyright (C) 2024 Kristian Duske

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

#include "ui/MapDocumentTest.h"

#include "TestUtils.h"
#include "mdl/BrushBuilder.h"
#include "mdl/BrushNode.h"
#include "mdl/EntityDefinition.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocumentCommandFacade.h"

#include "kdl/vector_utils.h"

namespace tb::ui
{

MapDocumentTest::MapDocumentTest()
  : MapDocumentTest{mdl::MapFormat::Standard}
{
}

MapDocumentTest::MapDocumentTest(const mdl::MapFormat mapFormat)
  : m_mapFormat{mapFormat}
  , taskManager{createTestTaskManager()}
{
  SetUp();
}

void MapDocumentTest::SetUp()
{
  game = std::make_shared<mdl::TestGame>();
  game->config().forceEmptyNewMap = true;

  document = MapDocumentCommandFacade::newMapDocument(*taskManager);
  document->newDocument(m_mapFormat, vm::bbox3d{8192.0}, game)
    | kdl::transform_error([](auto e) { throw std::runtime_error{e.msg}; });

  // create two entity definitions
  m_pointEntityDef = new mdl::PointEntityDefinition{
    "point_entity", Color{}, vm::bbox3d{16.0}, "this is a point entity", {}, {}, {}};
  m_brushEntityDef =
    new mdl::BrushEntityDefinition{"brush_entity", Color{}, "this is a brush entity", {}};

  document->setEntityDefinitions(kdl::vec_from(
    std::unique_ptr<mdl::EntityDefinition>{m_pointEntityDef},
    std::unique_ptr<mdl::EntityDefinition>{m_brushEntityDef}));
}

MapDocumentTest::~MapDocumentTest()
{
  m_pointEntityDef = nullptr;
  m_brushEntityDef = nullptr;
}

mdl::BrushNode* MapDocumentTest::createBrushNode(
  const std::string& materialName,
  const std::function<void(mdl::Brush&)>& brushFunc) const
{
  const auto* worldNode = document->world();
  auto builder = mdl::BrushBuilder{
    worldNode->mapFormat(),
    document->worldBounds(),
    document->game()->config().faceAttribsConfig.defaults};

  auto brush = builder.createCube(32.0, materialName) | kdl::value();
  brushFunc(brush);
  return new mdl::BrushNode{std::move(brush)};
}

mdl::PatchNode* MapDocumentTest::createPatchNode(const std::string& materialName) const
{
  // clang-format off
  return new mdl::PatchNode{mdl::BezierPatch{3, 3, {
    {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
    {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
    {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, materialName}};
  // clang-format on
}

ValveMapDocumentTest::ValveMapDocumentTest()
  : MapDocumentTest{mdl::MapFormat::Valve}
{
}

Quake3MapDocumentTest::Quake3MapDocumentTest()
  : MapDocumentTest{mdl::MapFormat::Quake3}
{
}

} // namespace tb::ui
