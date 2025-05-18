/*
 Copyright (C) 2010 Kristian Duske

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

#pragma once

#include "mdl/MapFormat.h"
#include "mdl/TestGame.h"
#include "ui/MapDocument.h"

#include "kdl/task_manager.h"

#include <functional>
#include <memory>
#include <string>

namespace tb::mdl
{
class Brush;
struct EntityDefinition;
class PatchNode;
class TestGame;
} // namespace tb::mdl

namespace tb::ui
{

class MapDocumentTest
{
private:
  mdl::MapFormat m_mapFormat;

protected:
  std::unique_ptr<kdl::task_manager> taskManager;
  std::shared_ptr<mdl::TestGame> game;
  std::shared_ptr<MapDocument> document;
  const mdl::EntityDefinition* m_pointEntityDef = nullptr;
  const mdl::EntityDefinition* m_brushEntityDef = nullptr;

protected:
  MapDocumentTest();
  explicit MapDocumentTest(mdl::MapFormat mapFormat);

private:
  void SetUp();

protected:
  virtual ~MapDocumentTest();

public:
  mdl::BrushNode* createBrushNode(
    const std::string& materialName = "material",
    const std::function<void(mdl::Brush&)>& brushFunc = [](mdl::Brush&) {}) const;
  mdl::PatchNode* createPatchNode(const std::string& materialName = "material") const;
};

class ValveMapDocumentTest : public MapDocumentTest
{
protected:
  ValveMapDocumentTest();
};

class Quake3MapDocumentTest : public MapDocumentTest
{
public:
  Quake3MapDocumentTest();
};

} // namespace tb::ui
