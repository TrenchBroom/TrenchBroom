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

#include "ui/OutlinerModel.h"

#include <QPalette>

#include "mdl/BrushNode.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/LayerNode.h"
#include "mdl/Map.h"
#include "mdl/Node.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"
#include "ui/MapDocument.h"

#include "kd/overload.h"

#include <algorithm>
#include <iterator>

namespace tb::ui
{
namespace
{

QString typeName(const mdl::Node& node)
{
  return node.accept(kdl::overload(
    [](const mdl::WorldNode&) { return QObject::tr("World"); },
    [](const mdl::LayerNode&) { return QObject::tr("Layer"); },
    [](const mdl::GroupNode&) { return QObject::tr("Group"); },
    [](const mdl::EntityNode&) { return QObject::tr("Entity"); },
    [](const mdl::BrushNode&) { return QObject::tr("Brush"); },
    [](const mdl::PatchNode&) { return QObject::tr("Patch"); }));
}

} // namespace

OutlinerModel::OutlinerModel(MapDocument& document, QObject* parent)
  : QAbstractItemModel{parent}
  , m_document{document}
{
  connectObservers();
}

QModelIndex OutlinerModel::index(
  const int row, const int column, const QModelIndex& parent) const
{
  if (!hasIndex(row, column, parent))
  {
    return {};
  }

  if (!parent.isValid())
  {
    const auto top = topLevelNodes();
    if (row < 0 || static_cast<size_t>(row) >= top.size())
    {
      return {};
    }
    return createIndex(row, column, top[static_cast<size_t>(row)]);
  }

  auto* parentNode = toNode(parent);
  const auto& children = parentNode->children();
  if (row < 0 || static_cast<size_t>(row) >= children.size())
  {
    return {};
  }
  return createIndex(row, column, children[static_cast<size_t>(row)]);
}

QModelIndex OutlinerModel::parent(const QModelIndex& child) const
{
  if (!child.isValid())
  {
    return {};
  }

  auto* childNode = toNode(child);
  auto* parentNode = childNode->parent();
  auto& world = m_document.map().worldNode();

  if (!parentNode || parentNode == &world)
  {
    // childNode is a layer, i.e. top-level in this model
    return {};
  }

  auto* grandparentNode = parentNode->parent();

  int row = 0;
  if (!grandparentNode || grandparentNode == &world)
  {
    const auto top = topLevelNodes();
    const auto it = std::find(top.begin(), top.end(), parentNode);
    row = it != top.end() ? static_cast<int>(std::distance(top.begin(), it)) : 0;
  }
  else
  {
    const auto& siblings = grandparentNode->children();
    const auto it = std::find(siblings.begin(), siblings.end(), parentNode);
    row =
      it != siblings.end() ? static_cast<int>(std::distance(siblings.begin(), it)) : 0;
  }

  return createIndex(row, 0, parentNode);
}

int OutlinerModel::rowCount(const QModelIndex& parent) const
{
  if (parent.column() > 0)
  {
    return 0;
  }

  if (!parent.isValid())
  {
    return static_cast<int>(topLevelNodes().size());
  }

  return static_cast<int>(toNode(parent)->childCount());
}

int OutlinerModel::columnCount(const QModelIndex&) const
{
  return ColumnCount;
}

QVariant OutlinerModel::data(const QModelIndex& index, const int role) const
{
  if (!index.isValid())
  {
    return {};
  }

  const auto* node = toNode(index);

  switch (role)
  {
  case Qt::DisplayRole:
  case Qt::ToolTipRole:
    switch (index.column())
    {
    case NameColumn:
      return QString::fromStdString(node->name());
    case TypeColumn:
      return typeName(*node);
    case VisibleColumn:
      return node->hidden() ? tr("Hidden") : QVariant{};
    case LockedColumn:
      return node->locked() ? tr("Locked") : QVariant{};
    default:
      return {};
    }
  case Qt::ForegroundRole:
    if (node->hidden())
    {
      return QVariant::fromValue(QPalette{}.color(QPalette::Disabled, QPalette::Text));
    }
    return {};
  default:
    return {};
  }
}

QVariant OutlinerModel::headerData(
  const int section, const Qt::Orientation orientation, const int role) const
{
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
  {
    return {};
  }

  switch (section)
  {
  case NameColumn:
    return tr("Name");
  case TypeColumn:
    return tr("Type");
  case VisibleColumn:
    return tr("Visible");
  case LockedColumn:
    return tr("Locked");
  default:
    return {};
  }
}

Qt::ItemFlags OutlinerModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
  {
    return Qt::NoItemFlags;
  }
  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex OutlinerModel::indexForNode(const mdl::Node* constNode) const
{
  auto* node = const_cast<mdl::Node*>(constNode);
  if (!node)
  {
    return {};
  }

  auto& world = m_document.map().worldNode();
  if (node == &world)
  {
    return {};
  }

  auto chain = std::vector<mdl::Node*>{};
  for (auto* n = node; n && n != &world; n = n->parent())
  {
    chain.push_back(n);
  }
  std::reverse(chain.begin(), chain.end());

  auto result = QModelIndex{};
  for (auto* n : chain)
  {
    auto* parentNode = n->parent();

    int row = -1;
    if (!parentNode || parentNode == &world)
    {
      const auto top = topLevelNodes();
      const auto it = std::find(top.begin(), top.end(), n);
      if (it == top.end())
      {
        return {};
      }
      row = static_cast<int>(std::distance(top.begin(), it));
    }
    else
    {
      const auto& siblings = parentNode->children();
      const auto it = std::find(siblings.begin(), siblings.end(), n);
      if (it == siblings.end())
      {
        return {};
      }
      row = static_cast<int>(std::distance(siblings.begin(), it));
    }

    result = index(row, 0, result);
    if (!result.isValid())
    {
      return {};
    }
  }

  return result;
}

mdl::Node* OutlinerModel::nodeForIndex(const QModelIndex& index) const
{
  return index.isValid() ? toNode(index) : nullptr;
}

void OutlinerModel::connectObservers()
{
  m_notifierConnection +=
    m_document.documentWasLoadedNotifier.connect([&] { reload(); });
  m_notifierConnection +=
    m_document.nodesWereAddedNotifier.connect([&](const std::vector<mdl::Node*>&) {
      reload();
    });
  m_notifierConnection +=
    m_document.nodesWillBeRemovedNotifier.connect([&](const std::vector<mdl::Node*>&) {
      reload();
    });
  m_notifierConnection += m_document.documentDidChangeNotifier.connect([&] {
    refreshAllData();
  });
  m_notifierConnection += m_document.nodeVisibilityDidChangeNotifier.connect(
    [&](const std::vector<mdl::Node*>& nodes) { refreshDataForNodes(nodes); });
  m_notifierConnection += m_document.nodeLockingDidChangeNotifier.connect(
    [&](const std::vector<mdl::Node*>& nodes) { refreshDataForNodes(nodes); });
}

void OutlinerModel::reload()
{
  beginResetModel();
  endResetModel();
}

void OutlinerModel::refreshAllData()
{
  emitDataChangedRecursive({});
}

void OutlinerModel::emitDataChangedRecursive(const QModelIndex& parent)
{
  const auto rc = rowCount(parent);
  if (rc == 0)
  {
    return;
  }

  emit dataChanged(index(0, 0, parent), index(rc - 1, ColumnCount - 1, parent));

  for (int row = 0; row < rc; ++row)
  {
    emitDataChangedRecursive(index(row, 0, parent));
  }
}

void OutlinerModel::refreshDataForNodes(const std::vector<mdl::Node*>& nodes)
{
  for (auto* node : nodes)
  {
    const auto idx = indexForNode(node);
    if (idx.isValid())
    {
      const auto first = index(idx.row(), 0, idx.parent());
      const auto last = index(idx.row(), ColumnCount - 1, idx.parent());
      emit dataChanged(first, last);
    }
  }
}

std::vector<mdl::Node*> OutlinerModel::topLevelNodes() const
{
  auto& world = m_document.map().worldNode();
  const auto layers = world.allLayersUserSorted();
  return {layers.begin(), layers.end()};
}

mdl::Node* OutlinerModel::toNode(const QModelIndex& index)
{
  return static_cast<mdl::Node*>(index.internalPointer());
}

} // namespace tb::ui
