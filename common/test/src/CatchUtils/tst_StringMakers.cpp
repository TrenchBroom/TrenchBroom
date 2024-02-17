/*
 Copyright (C) 2023 Kristian Duske

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

#include "Error.h"
#include "Model/BezierPatch.h"
#include "Model/Brush.h"
#include "Model/BrushBuilder.h"
#include "Model/BrushNode.h"
#include "Model/Entity.h"
#include "Model/EntityNode.h"
#include "Model/Group.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"
#include "Result.h"
#include "StringMakers.h"
#include "TestUtils.h"

#include "vm/bbox.h"

#include "Catch2.h"

namespace TrenchBroom::Model
{

TEST_CASE("convertToString")
{
  auto worldNode = WorldNode({}, Entity{}, MapFormat::Quake3);

  // explicitly set link IDs
  setLinkId(worldNode, "world_link_id");

  CHECK(convertToString(worldNode) == R"(WorldNode{
  m_entityPropertyConfig: EntityPropertyConfig{defaultModelScaleExpression: nullopt, setDefaultProperties: 0, updateAnglePropertyAfterTransform: 1},
  m_mapFormat: Quake3,
  m_entity: Entity{m_properties: [EntityProperty{m_key: classname, m_value: worldspawn}], m_protectedProperties: []},
  m_children: [
    LayerNode{
      m_layer: Layer{m_defaultLayer: 1, m_name: Default Layer, m_sortIndex: nullopt, m_color: nullopt, m_omitFromExport: 0},
      m_children: [],
    }
  ],
})");

  auto brushBuilder = BrushBuilder{worldNode.mapFormat(), vm::bbox3d{8192.0}};

  auto* groupNode = new GroupNode{Group{"group"}};
  auto* entityNode = new EntityNode{Entity{}};
  auto* brushNode = new BrushNode{brushBuilder.createCube(64.0, "texture").value()};

  // clang-format off
  auto* patchNode = new PatchNode{BezierPatch{3, 3, {
      {0, 0, 0}, {1, 0, 1}, {2, 0, 0},
      {0, 1, 1}, {1, 1, 2}, {2, 1, 1},
      {0, 2, 0}, {1, 2, 1}, {2, 2, 0} }, "texture"}};
  // clang-format on

  // explicitly set link IDs
  setLinkId(*groupNode, "group_link_id");
  setLinkId(*entityNode, "entity_link_id");
  setLinkId(*brushNode, "brush_link_id");
  setLinkId(*patchNode, "patch_link_id");

  groupNode->addChildren({entityNode, brushNode, patchNode});
  worldNode.defaultLayer()->addChild(groupNode);

  CHECK(convertToString(worldNode) == R"(WorldNode{
  m_entityPropertyConfig: EntityPropertyConfig{defaultModelScaleExpression: nullopt, setDefaultProperties: 0, updateAnglePropertyAfterTransform: 1},
  m_mapFormat: Quake3,
  m_entity: Entity{m_properties: [EntityProperty{m_key: classname, m_value: worldspawn}], m_protectedProperties: []},
  m_children: [
    LayerNode{
      m_layer: Layer{m_defaultLayer: 1, m_name: Default Layer, m_sortIndex: nullopt, m_color: nullopt, m_omitFromExport: 0},
      m_children: [
        GroupNode{
          m_group: Group{m_name: group, m_transformation: 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1},
          m_linkId: group_link_id,
          m_children: [
            EntityNode{
              m_entity: Entity{m_properties: [], m_protectedProperties: []},
              m_linkId: entity_link_id,
              m_children: [],
            },
            BrushNode{
              m_brush: Brush{m_faces: [BrushFace{m_points: [-32 -32 -32,-32 -31 -32,-32 -32 -31], m_boundary: { normal: (-1 0 0), distance: 32 }, m_attributes: BrushFaceAttributes{m_textureName: texture, m_offset: 0 0, m_scale: 1 1, m_rotation: 0, m_surfaceContents: nullopt, m_surfaceFlags: nullopt, m_surfaceValue: nullopt, m_color: nullopt}, m_textureReference: AssetReference<T>{m_asset: null}},BrushFace{m_points: [-32 -32 -32,-32 -32 -31,-31 -32 -32], m_boundary: { normal: (0 -1 0), distance: 32 }, m_attributes: BrushFaceAttributes{m_textureName: texture, m_offset: 0 0, m_scale: 1 1, m_rotation: 0, m_surfaceContents: nullopt, m_surfaceFlags: nullopt, m_surfaceValue: nullopt, m_color: nullopt}, m_textureReference: AssetReference<T>{m_asset: null}},BrushFace{m_points: [-32 -32 -32,-31 -32 -32,-32 -31 -32], m_boundary: { normal: (0 0 -1), distance: 32 }, m_attributes: BrushFaceAttributes{m_textureName: texture, m_offset: 0 0, m_scale: 1 1, m_rotation: 0, m_surfaceContents: nullopt, m_surfaceFlags: nullopt, m_surfaceValue: nullopt, m_color: nullopt}, m_textureReference: AssetReference<T>{m_asset: null}},BrushFace{m_points: [32 32 32,32 33 32,33 32 32], m_boundary: { normal: (0 0 1), distance: 32 }, m_attributes: BrushFaceAttributes{m_textureName: texture, m_offset: 0 0, m_scale: 1 1, m_rotation: 0, m_surfaceContents: nullopt, m_surfaceFlags: nullopt, m_surfaceValue: nullopt, m_color: nullopt}, m_textureReference: AssetReference<T>{m_asset: null}},BrushFace{m_points: [32 32 32,33 32 32,32 32 33], m_boundary: { normal: (0 1 0), distance: 32 }, m_attributes: BrushFaceAttributes{m_textureName: texture, m_offset: 0 0, m_scale: 1 1, m_rotation: 0, m_surfaceContents: nullopt, m_surfaceFlags: nullopt, m_surfaceValue: nullopt, m_color: nullopt}, m_textureReference: AssetReference<T>{m_asset: null}},BrushFace{m_points: [32 32 32,32 32 33,32 33 32], m_boundary: { normal: (1 0 0), distance: 32 }, m_attributes: BrushFaceAttributes{m_textureName: texture, m_offset: 0 0, m_scale: 1 1, m_rotation: 0, m_surfaceContents: nullopt, m_surfaceFlags: nullopt, m_surfaceValue: nullopt, m_color: nullopt}, m_textureReference: AssetReference<T>{m_asset: null}}]},
              m_linkId: brush_link_id,
              m_children: [],
            },
            PatchNode{
              m_patch: BezierPatch{m_pointRowCount: 3, m_pointColumnCount: 3, m_bounds: { min: (0 0 0), max: (2 2 2) }, m_controlPoints: [0 0 0 0 0,1 0 1 0 0,2 0 0 0 0,0 1 1 0 0,1 1 2 0 0,2 1 1 0 0,0 2 0 0 0,1 2 1 0 0,2 2 0 0 0], m_textureName: texture},
              m_linkId: patch_link_id,
              m_children: [],
            }
          ],
        }
      ],
    }
  ],
})");
}

} // namespace TrenchBroom::Model
