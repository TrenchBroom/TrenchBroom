/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "Model/MapFormat.h"
#include "View/MapDocument.h"

#include <functional>
#include <memory>
#include <string>

namespace TrenchBroom
{
namespace Assets
{
class BrushEntityDefinition;
class PointEntityDefinition;
} // namespace Assets

namespace Model
{
class Brush;
class PatchNode;
class TestGame;
} // namespace Model

namespace View
{
class MapDocumentTest
{
private:
  Model::MapFormat m_mapFormat;

protected:
  std::shared_ptr<Model::TestGame> game;
  std::shared_ptr<MapDocument> document;
  Assets::PointEntityDefinition* m_pointEntityDef;
  Assets::BrushEntityDefinition* m_brushEntityDef;

protected:
  MapDocumentTest();
  explicit MapDocumentTest(Model::MapFormat mapFormat);

private:
  void SetUp();

protected:
  virtual ~MapDocumentTest();

public:
  Model::BrushNode* createBrushNode(
    const std::string& textureName = "texture",
    const std::function<void(Model::Brush&)>& brushFunc = [](Model::Brush&) {}) const;
  Model::PatchNode* createPatchNode(const std::string& textureName = "texture") const;
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

} // namespace View
} // namespace TrenchBroom
