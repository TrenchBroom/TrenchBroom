/*
 Copyright (C) 2026

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

#include <QtTest/QSignalSpy>

#include "mdl/Brush.h"
#include "mdl/BrushNode.h"
#include "mdl/Entity.h"
#include "mdl/EntityNode.h"
#include "mdl/Group.h"
#include "mdl/GroupNode.h"
#include "mdl/Layer.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Map_Layers.h"
#include "mdl/Map_NodeLocking.h"
#include "mdl/Map_NodeVisibility.h"
#include "mdl/Map_Nodes.h"
#include "mdl/TestFactory.h"
#include "mdl/WorldNode.h"
#include "ui/AppControllerFixture.h"
#include "ui/CatchConfig.h"
#include "ui/MapDocument.h"
#include "ui/MapDocumentFixture.h"
#include "ui/OutlinerModel.h"

#include <catch2/catch_test_macros.hpp>

namespace tb::ui
{

namespace
{

/**
 * Depth-first search for the row index whose OutlinerModel::NameColumn text matches
 * `name`, starting at `parent`. Used to locate nodes in the model without relying on
 * OutlinerModel::indexForNode, so that the two code paths are cross-checked against each
 * other.
 */
QModelIndex findByName(
  const OutlinerModel& model, const QString& name, const QModelIndex& parent = {})
{
  const auto rc = model.rowCount(parent);
  for (int row = 0; row < rc; ++row)
  {
    const auto idx = model.index(row, OutlinerModel::NameColumn, parent);
    if (model.data(idx, Qt::DisplayRole).toString() == name)
    {
      return idx;
    }
    if (const auto found = findByName(model, name, model.index(row, 0, parent));
        found.isValid())
    {
      return found;
    }
  }
  return {};
}

} // namespace

TEST_CASE("OutlinerModel")
{
  auto appControllerFixture = AppControllerFixture{};

  auto documentFixture = MapDocumentFixture{};
  auto& document = documentFixture.create();
  auto& map = document.map();

  auto model = OutlinerModel{document};

  SECTION("the default layer is the only top-level row initially")
  {
    CHECK(model.rowCount({}) == 1);
    const auto defaultLayerIndex = model.index(0, 0, {});
    CHECK(
      model.data(defaultLayerIndex, Qt::DisplayRole).toString()
      == QString::fromStdString(map.worldNode().defaultLayer()->name()));
    CHECK(
      model.data(defaultLayerIndex.siblingAtColumn(OutlinerModel::TypeColumn))
        .toString()
      == "Layer");
  }

  SECTION("reflects layers, groups, entities and brushes")
  {
    auto* customLayerNode = new mdl::LayerNode{mdl::Layer{"custom layer"}};
    mdl::addNodes(map, {{&map.worldNode(), {customLayerNode}}});

    auto* groupNode = new mdl::GroupNode{mdl::Group{"my group"}};
    auto* brushInLayerNode = mdl::createBrushNode(map);
    mdl::addNodes(map, {{customLayerNode, {groupNode, brushInLayerNode}}});

    auto* entityNode = new mdl::EntityNode{mdl::Entity{{{"classname", "my_entity"}}}};
    mdl::addNodes(map, {{groupNode, {entityNode}}});

    // Two top-level rows: default layer + custom layer, with the default layer first.
    REQUIRE(model.rowCount({}) == 2);
    const auto customLayerIndex = findByName(model, "custom layer");
    REQUIRE(customLayerIndex.isValid());
    CHECK(customLayerIndex.row() == 1);

    // The custom layer has two children: the group and the brush.
    CHECK(model.rowCount(customLayerIndex) == 2);

    const auto groupIndex = findByName(model, "my group");
    REQUIRE(groupIndex.isValid());
    CHECK(
      model.data(groupIndex.siblingAtColumn(OutlinerModel::TypeColumn)).toString()
      == "Group");
    CHECK(model.parent(groupIndex) == customLayerIndex);

    // The group has one child: the entity.
    REQUIRE(model.rowCount(groupIndex) == 1);
    const auto entityIndex = model.index(0, 0, groupIndex);
    CHECK(model.data(entityIndex, Qt::DisplayRole).toString() == "my_entity");
    CHECK(
      model.data(entityIndex.siblingAtColumn(OutlinerModel::TypeColumn)).toString()
      == "Entity");

    const auto brushIndex = findByName(model, "brush");
    REQUIRE(brushIndex.isValid());
    CHECK(
      model.data(brushIndex.siblingAtColumn(OutlinerModel::TypeColumn)).toString()
      == "Brush");
    CHECK(model.parent(brushIndex) == customLayerIndex);
  }

  SECTION("indexForNode and nodeForIndex round-trip for deeply nested nodes")
  {
    auto* customLayerNode = new mdl::LayerNode{mdl::Layer{"custom layer"}};
    mdl::addNodes(map, {{&map.worldNode(), {customLayerNode}}});

    auto* outerGroupNode = new mdl::GroupNode{mdl::Group{"outer"}};
    mdl::addNodes(map, {{customLayerNode, {outerGroupNode}}});

    auto* innerGroupNode = new mdl::GroupNode{mdl::Group{"inner"}};
    mdl::addNodes(map, {{outerGroupNode, {innerGroupNode}}});

    auto* entityNode = new mdl::EntityNode{mdl::Entity{{{"classname", "leaf"}}}};
    mdl::addNodes(map, {{innerGroupNode, {entityNode}}});

    const auto index = model.indexForNode(entityNode);
    REQUIRE(index.isValid());
    CHECK(model.nodeForIndex(index) == static_cast<mdl::Node*>(entityNode));
    CHECK(model.data(index, Qt::DisplayRole).toString() == "leaf");

    // World node itself is not represented (it's the implicit root).
    CHECK_FALSE(model.indexForNode(&map.worldNode()).isValid());
  }

  SECTION("visibility and lock changes update data without resetting the model")
  {
    auto* customLayerNode = new mdl::LayerNode{mdl::Layer{"custom layer"}};
    mdl::addNodes(map, {{&map.worldNode(), {customLayerNode}}});

    auto* brushNode = mdl::createBrushNode(map);
    mdl::addNodes(map, {{customLayerNode, {brushNode}}});

    const auto index = model.indexForNode(brushNode);
    REQUIRE(index.isValid());
    const auto visibleIndex = index.siblingAtColumn(OutlinerModel::VisibleColumn);
    const auto lockedIndex = index.siblingAtColumn(OutlinerModel::LockedColumn);
    CHECK(model.data(visibleIndex).toString().isEmpty());
    CHECK(model.data(lockedIndex).toString().isEmpty());

    auto resetSpy = QSignalSpy{&model, &QAbstractItemModel::modelReset};
    auto dataChangedSpy = QSignalSpy{&model, &QAbstractItemModel::dataChanged};

    mdl::hideNodes(map, {static_cast<mdl::Node*>(brushNode)});
    mdl::lockNodes(map, {static_cast<mdl::Node*>(brushNode)});

    CHECK(resetSpy.isEmpty());
    CHECK_FALSE(dataChangedSpy.isEmpty());

    CHECK(model.data(visibleIndex).toString() == "Hidden");
    CHECK(model.data(lockedIndex).toString() == "Locked");
  }

  SECTION("adding nodes resets the model so new rows appear")
  {
    auto resetSpy = QSignalSpy{&model, &QAbstractItemModel::modelReset};

    auto* customLayerNode = new mdl::LayerNode{mdl::Layer{"custom layer"}};
    mdl::addNodes(map, {{&map.worldNode(), {customLayerNode}}});

    CHECK_FALSE(resetSpy.isEmpty());
    CHECK(model.rowCount({}) == 2);
  }

  SECTION("renaming a layer updates data without resetting the model")
  {
    auto* customLayerNode = new mdl::LayerNode{mdl::Layer{"custom layer"}};
    mdl::addNodes(map, {{&map.worldNode(), {customLayerNode}}});

    const auto index = model.indexForNode(customLayerNode);
    REQUIRE(index.isValid());

    auto resetSpy = QSignalSpy{&model, &QAbstractItemModel::modelReset};
    auto dataChangedSpy = QSignalSpy{&model, &QAbstractItemModel::dataChanged};

    mdl::renameLayer(map, customLayerNode, "renamed layer");

    CHECK(resetSpy.isEmpty());
    CHECK_FALSE(dataChangedSpy.isEmpty());
    CHECK(model.data(index, Qt::DisplayRole).toString() == "renamed layer");
  }

  SECTION("a pure geometry edit (nodesDidChangeNotifier with no name/visibility/lock "
          "change) does not reset the model")
  {
    auto* customLayerNode = new mdl::LayerNode{mdl::Layer{"custom layer"}};
    mdl::addNodes(map, {{&map.worldNode(), {customLayerNode}}});

    auto* brushNode = mdl::createBrushNode(map);
    mdl::addNodes(map, {{customLayerNode, {brushNode}}});

    auto resetSpy = QSignalSpy{&model, &QAbstractItemModel::modelReset};

    // Simulates what dragging a brush face does: nodesDidChangeNotifier fires for the
    // brush being dragged, once per mouse-move, without ever touching the whole tree.
    const auto changedNodes = std::vector<mdl::Node*>{static_cast<mdl::Node*>(brushNode)};
    document.nodesDidChangeNotifier(changedNodes);
    document.nodesDidChangeNotifier(changedNodes);

    CHECK(resetSpy.isEmpty());
    CHECK(model.data(findByName(model, "brush"), Qt::DisplayRole).toString() == "brush");
  }
}

} // namespace tb::ui
