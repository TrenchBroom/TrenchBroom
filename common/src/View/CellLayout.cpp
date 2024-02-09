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

#include "CellLayout.h"

#include "Ensure.h"
#include "Macros.h"

#include <algorithm>
#include <cassert>

namespace TrenchBroom::View
{

float LayoutBounds::left() const
{
  return x;
}

float LayoutBounds::top() const
{
  return y;
}

float LayoutBounds::right() const
{
  return x + width;
}

float LayoutBounds::bottom() const
{
  return y + height;
}

bool LayoutBounds::containsPoint(const float pointX, const float pointY) const
{
  return pointX >= left() && pointX <= right() && pointY >= top() && pointY <= bottom();
}

bool LayoutBounds::intersectsY(const float rangeY, const float rangeHeight) const
{
  return bottom() >= rangeY && top() <= rangeY + rangeHeight;
}

LayoutCell::LayoutCell(
  std::any item,
  const float x,
  const float y,
  const float itemWidth,
  const float itemHeight,
  const float titleWidth,
  const float titleHeight,
  const float titleMargin,
  const float maxUpScale,
  const float minWidth,
  const float maxWidth,
  const float minHeight,
  const float maxHeight)
  : m_item{std::move(item)}
  , m_x{x}
  , m_y{y}
  , m_itemWidth{itemWidth}
  , m_itemHeight{itemHeight}
  , m_titleWidth{titleWidth}
  , m_titleHeight{titleHeight}
  , m_titleMargin{titleMargin}
{
  doLayout(maxUpScale, minWidth, maxWidth, minHeight, maxHeight);
}

std::any& LayoutCell::item()
{
  return m_item;
}

const std::any& LayoutCell::item() const
{
  return m_item;
}

float LayoutCell::scale() const
{
  return m_scale;
}

const LayoutBounds& LayoutCell::bounds() const
{
  return cellBounds();
}

const LayoutBounds& LayoutCell::cellBounds() const
{
  return m_cellBounds;
}

const LayoutBounds& LayoutCell::titleBounds() const
{
  return m_titleBounds;
}

const LayoutBounds& LayoutCell::itemBounds() const
{
  return m_itemBounds;
}

bool LayoutCell::hitTest(const float x, const float y) const
{
  return bounds().containsPoint(x, y);
}

void LayoutCell::updateLayout(
  const float maxUpScale,
  const float minWidth,
  const float maxWidth,
  const float minHeight,
  const float maxHeight)
{
  doLayout(maxUpScale, minWidth, maxWidth, minHeight, maxHeight);
}

void LayoutCell::doLayout(
  const float maxUpScale,
  const float minWidth,
  const float maxWidth,
  const float minHeight,
  const float maxHeight)
{
  assert(0.0f < minWidth);
  assert(0.0f < minHeight);
  assert(minWidth <= maxWidth);
  assert(minHeight <= maxHeight);

  m_scale =
    std::min(std::min(maxWidth / m_itemWidth, maxHeight / m_itemHeight), maxUpScale);
  const auto scaledItemWidth = m_scale * m_itemWidth;
  const auto scaledItemHeight = m_scale * m_itemHeight;
  const auto clippedTitleWidth = std::min(m_titleWidth, maxWidth);
  const auto cellWidth = std::max(minWidth, std::max(scaledItemWidth, clippedTitleWidth));
  const auto cellHeight = std::max(
    minHeight, std::max(minHeight, scaledItemHeight) + m_titleHeight + m_titleMargin);
  const auto itemY =
    m_y + std::max(0.0f, cellHeight - m_titleHeight - scaledItemHeight - m_titleMargin);

  m_cellBounds = LayoutBounds{m_x, m_y, cellWidth, cellHeight};
  m_itemBounds = LayoutBounds{
    m_x + (m_cellBounds.width - scaledItemWidth) / 2.0f,
    itemY,
    scaledItemWidth,
    scaledItemHeight};
  m_titleBounds = LayoutBounds{
    m_x + (m_cellBounds.width - clippedTitleWidth) / 2.0f,
    m_itemBounds.bottom() + m_titleMargin,
    clippedTitleWidth,
    m_titleHeight};
}

LayoutRow::LayoutRow(
  const float x,
  const float y,
  const float cellMargin,
  const float titleMargin,
  const float maxWidth,
  const size_t maxCells,
  const float maxUpScale,
  const float minCellWidth,
  const float maxCellWidth,
  const float minCellHeight,
  const float maxCellHeight)
  : m_cellMargin{cellMargin}
  , m_titleMargin{titleMargin}
  , m_maxWidth{maxWidth}
  , m_maxCells{maxCells}
  , m_maxUpScale{maxUpScale}
  , m_minCellWidth{minCellWidth}
  , m_maxCellWidth{maxCellWidth}
  , m_minCellHeight{minCellHeight}
  , m_maxCellHeight{maxCellHeight}
  , m_bounds{x, y, 0.0f, 0.0f}
{
}

const LayoutBounds& LayoutRow::bounds() const
{
  return m_bounds;
}

const std::vector<LayoutCell>& LayoutRow::cells() const
{
  return m_cells;
}

const LayoutCell* LayoutRow::cellAt(const float x, const float y) const
{
  for (size_t i = 0; i < m_cells.size(); ++i)
  {
    const auto& cell = m_cells[i];
    const auto& cellBounds = cell.cellBounds();
    if (x > cellBounds.right())
    {
      continue;
    }
    if (x < cellBounds.left())
    {
      return nullptr;
    }
    if (cell.hitTest(x, y))
    {
      return &cell;
    }
  }
  return nullptr;
}

bool LayoutRow::intersectsY(const float y, const float height) const
{
  return m_bounds.intersectsY(y, height);
}

bool LayoutRow::canAddItem(
  const float itemWidth,
  const float itemHeight,
  const float titleWidth,
  const float titleHeight) const
{
  auto x = m_bounds.right();
  auto width = m_bounds.width;
  if (!m_cells.empty())
  {
    x += m_cellMargin;
    width += m_cellMargin;
  }

  auto cell = LayoutCell{
    std::any{},
    x,
    m_bounds.top(),
    itemWidth,
    itemHeight,
    titleWidth,
    titleHeight,
    m_titleMargin,
    m_maxUpScale,
    m_minCellWidth,
    m_maxCellWidth,
    m_minCellHeight,
    m_maxCellHeight};
  width += cell.cellBounds().width;

  if (m_maxCells == 0 && width > m_maxWidth && !m_cells.empty())
  {
    return false;
  }
  if (m_maxCells > 0 && m_cells.size() >= m_maxCells - 1)
  {
    return false;
  }

  return true;
}

void LayoutRow::addItem(
  std::any item,
  const float itemWidth,
  const float itemHeight,
  const float titleWidth,
  const float titleHeight)
{
  float x = m_bounds.right();
  float width = m_bounds.width;
  if (!m_cells.empty())
  {
    x += m_cellMargin;
    width += m_cellMargin;
  }

  auto cell = LayoutCell{
    std::move(item),
    x,
    m_bounds.top(),
    itemWidth,
    itemHeight,
    titleWidth,
    titleHeight,
    m_titleMargin,
    m_maxUpScale,
    m_minCellWidth,
    m_maxCellWidth,
    m_minCellHeight,
    m_maxCellHeight};
  width += cell.cellBounds().width;

  const auto newItemRowHeight =
    cell.cellBounds().height - cell.titleBounds().height - m_titleMargin;
  if (newItemRowHeight > m_minCellHeight)
  {
    m_minCellHeight = newItemRowHeight;
    assert(m_minCellHeight <= m_maxCellHeight);
    readjustItems();
  }

  m_bounds = LayoutBounds{
    m_bounds.left(),
    m_bounds.top(),
    width,
    std::max(m_bounds.height, cell.cellBounds().height)};

  m_cells.push_back(std::move(cell));
}

void LayoutRow::readjustItems()
{
  for (auto& cell : m_cells)
  {
    cell.updateLayout(
      m_maxUpScale, m_minCellWidth, m_maxCellWidth, m_minCellHeight, m_maxCellHeight);
  }
}

LayoutGroup::LayoutGroup(
  std::string title,
  const float x,
  const float y,
  const float cellMargin,
  const float titleMargin,
  const float rowMargin,
  const float titleHeight,
  const float width,
  const size_t maxCellsPerRow,
  const float maxUpScale,
  const float minCellWidth,
  const float maxCellWidth,
  const float minCellHeight,
  const float maxCellHeight)
  : m_title{std::move(title)}
  , m_cellMargin{cellMargin}
  , m_titleMargin{titleMargin}
  , m_rowMargin{rowMargin}
  , m_maxCellsPerRow{maxCellsPerRow}
  , m_maxUpScale{maxUpScale}
  , m_minCellWidth{minCellWidth}
  , m_maxCellWidth{maxCellWidth}
  , m_minCellHeight{minCellHeight}
  , m_maxCellHeight{maxCellHeight}
  , m_titleBounds{0.0f, y, width + 2.0f * x, titleHeight}
  , m_contentBounds{x, y + titleHeight + m_rowMargin, width, 0.0f}
{
}

LayoutGroup::LayoutGroup(
  const float x,
  const float y,
  const float cellMargin,
  const float titleMargin,
  const float rowMargin,
  const float width,
  const size_t maxCellsPerRow,
  const float maxUpScale,
  const float minCellWidth,
  const float maxCellWidth,
  const float minCellHeight,
  const float maxCellHeight)
  : m_cellMargin{cellMargin}
  , m_titleMargin{titleMargin}
  , m_rowMargin{rowMargin}
  , m_maxCellsPerRow{maxCellsPerRow}
  , m_maxUpScale{maxUpScale}
  , m_minCellWidth{minCellWidth}
  , m_maxCellWidth{maxCellWidth}
  , m_minCellHeight{minCellHeight}
  , m_maxCellHeight{maxCellHeight}
  , m_titleBounds{x, y, width, 0.0f}
  , m_contentBounds{x, y, width, 0.0f}
{
}

const std::string& LayoutGroup::title() const
{
  return m_title;
}

const LayoutBounds& LayoutGroup::titleBounds() const
{
  return m_titleBounds;
}

LayoutBounds LayoutGroup::titleBoundsForVisibleRect(
  const float y, const float height, const float groupMargin) const
{
  if (intersectsY(y, height) && m_titleBounds.top() < y)
  {
    if (y > m_contentBounds.bottom() - m_titleBounds.height + groupMargin)
    {
      return LayoutBounds{
        m_titleBounds.left(),
        m_contentBounds.bottom() - m_titleBounds.height + groupMargin,
        m_titleBounds.width,
        m_titleBounds.height};
    }
    return LayoutBounds{
      m_titleBounds.left(), y, m_titleBounds.width, m_titleBounds.height};
  }
  return m_titleBounds;
}

const LayoutBounds& LayoutGroup::contentBounds() const
{
  return m_contentBounds;
}

LayoutBounds LayoutGroup::bounds() const
{
  return LayoutBounds{
    m_titleBounds.left(),
    m_titleBounds.top(),
    m_titleBounds.width,
    m_contentBounds.bottom() - m_titleBounds.top()};
}

const std::vector<LayoutRow>& LayoutGroup::rows() const
{
  return m_rows;
}

size_t LayoutGroup::indexOfRowAt(const float y) const
{
  for (size_t i = 0; i < m_rows.size(); ++i)
  {
    const auto& row = m_rows[i];
    const auto& rowBounds = row.bounds();
    if (y < rowBounds.bottom())
    {
      return i;
    }
  }

  return m_rows.size();
}

const LayoutCell* LayoutGroup::cellAt(const float x, const float y) const
{
  for (size_t i = 0; i < m_rows.size(); ++i)
  {
    const auto& row = m_rows[i];
    const auto& rowBounds = row.bounds();
    if (y > rowBounds.bottom())
    {
      continue;
    }
    if (y < rowBounds.top())
    {
      return nullptr;
    }
    if (const auto* cell = row.cellAt(x, y))
    {
      return cell;
    }
  }

  return nullptr;
}

bool LayoutGroup::hitTest(const float x, const float y) const
{
  return bounds().containsPoint(x, y);
}

bool LayoutGroup::intersectsY(const float y, const float height) const
{
  return bounds().intersectsY(y, height);
}

void LayoutGroup::addItem(
  std::any item,
  const float itemWidth,
  const float itemHeight,
  const float titleWidth,
  const float titleHeight)
{
  if (m_rows.empty())
  {
    const auto y = m_contentBounds.top();
    m_rows.emplace_back(
      m_contentBounds.left(),
      y,
      m_cellMargin,
      m_titleMargin,
      m_contentBounds.width,
      m_maxCellsPerRow,
      m_maxUpScale,
      m_minCellWidth,
      m_maxCellWidth,
      m_minCellHeight,
      m_maxCellHeight);
  }

  if (!m_rows.back().canAddItem(itemWidth, itemHeight, titleWidth, titleHeight))
  {
    const auto oldBounds = m_rows.back().bounds();
    const auto y = oldBounds.bottom() + m_rowMargin;
    m_rows.emplace_back(
      m_contentBounds.left(),
      y,
      m_cellMargin,
      m_titleMargin,
      m_contentBounds.width,
      m_maxCellsPerRow,
      m_maxUpScale,
      m_minCellWidth,
      m_maxCellWidth,
      m_minCellHeight,
      m_maxCellHeight);

    const auto newRowHeight = m_rows.back().bounds().height;
    m_contentBounds = LayoutBounds{
      m_contentBounds.left(),
      m_contentBounds.top(),
      m_contentBounds.width,
      m_contentBounds.height + newRowHeight + m_rowMargin};
  }

  const auto oldRowHeight = m_rows.back().bounds().height;

  assert(m_rows.back().canAddItem(itemWidth, itemHeight, titleWidth, titleHeight));
  m_rows.back().addItem(std::move(item), itemWidth, itemHeight, titleWidth, titleHeight);

  const auto newRowHeight = m_rows.back().bounds().height;
  m_contentBounds = LayoutBounds{
    m_contentBounds.left(),
    m_contentBounds.top(),
    m_contentBounds.width,
    m_contentBounds.height + (newRowHeight - oldRowHeight)};
}

CellLayout::CellLayout(const size_t maxCellsPerRow)
  : m_maxCellsPerRow{maxCellsPerRow}
{
  invalidate();
}

float CellLayout::titleMargin() const
{
  return m_titleMargin;
}

void CellLayout::setTitleMargin(const float titleMargin)
{
  if (m_titleMargin != titleMargin)
  {
    m_titleMargin = titleMargin;
    invalidate();
  }
}

float CellLayout::cellMargin() const
{
  return m_cellMargin;
}

void CellLayout::setCellMargin(const float cellMargin)
{
  if (m_cellMargin != cellMargin)
  {
    m_cellMargin = cellMargin;
    invalidate();
  }
}

float CellLayout::rowMargin() const
{
  return m_rowMargin;
}

void CellLayout::setRowMargin(const float rowMargin)
{
  if (m_rowMargin != rowMargin)
  {
    m_rowMargin = rowMargin;
    invalidate();
  }
}

float CellLayout::groupMargin() const
{
  return m_groupMargin;
}

void CellLayout::setGroupMargin(const float groupMargin)
{
  if (m_groupMargin != groupMargin)
  {
    m_groupMargin = groupMargin;
    invalidate();
  }
}

float CellLayout::outerMargin() const
{
  return m_outerMargin;
}

void CellLayout::setOuterMargin(const float outerMargin)
{
  if (m_outerMargin != outerMargin)
  {
    m_outerMargin = outerMargin;
    invalidate();
  }
}

float CellLayout::minCellWidth() const
{
  return m_minCellWidth;
}

float CellLayout::maxCellWidth() const
{
  return m_maxCellWidth;
}

void CellLayout::setCellWidth(const float minCellWidth, const float maxCellWidth)
{
  assert(0.0f < minCellWidth);
  assert(minCellWidth <= maxCellWidth);

  if (m_minCellWidth != minCellWidth || m_maxCellWidth != maxCellWidth)
  {
    m_minCellWidth = minCellWidth;
    m_maxCellWidth = maxCellWidth;
    invalidate();
  }
}

float CellLayout::minCellHeight() const
{
  return m_minCellHeight;
}

float CellLayout::maxCellHeight() const
{
  return m_maxCellHeight;
}

void CellLayout::setCellHeight(const float minCellHeight, const float maxCellHeight)
{
  assert(0.0f < minCellHeight);
  assert(minCellHeight <= maxCellHeight);

  if (m_minCellHeight != minCellHeight || m_maxCellHeight != maxCellHeight)
  {
    m_minCellHeight = minCellHeight;
    m_maxCellHeight = maxCellHeight;
    invalidate();
  }
}

float CellLayout::maxUpScale() const
{
  return m_maxUpScale;
}

void CellLayout::setMaxUpScale(const float maxUpScale)
{
  if (m_maxUpScale != maxUpScale)
  {
    m_maxUpScale = maxUpScale;
    invalidate();
  }
}

float CellLayout::width() const
{
  return m_width;
}

float CellLayout::height()
{
  if (!m_valid)
  {
    validate();
  }
  return m_height;
}

LayoutBounds CellLayout::titleBoundsForVisibleRect(
  const LayoutGroup& group, const float y, const float height) const
{
  return group.titleBoundsForVisibleRect(y, height, m_groupMargin);
}

float CellLayout::rowPosition(const float y, const int offset)
{
  if (!m_valid)
  {
    validate();
  }

  auto groupIndex = m_groups.size();
  for (size_t i = 0; i < m_groups.size(); ++i)
  {
    const LayoutGroup& candidate = m_groups[i];
    const LayoutBounds groupBounds = candidate.bounds();
    if (y + m_rowMargin > groupBounds.bottom())
    {
      continue;
    }
    groupIndex = i;
    break;
  }

  if (groupIndex == m_groups.size())
  {
    return y;
  }

  if (offset == 0)
  {
    return y;
  }

  auto rowIndex = m_groups[groupIndex].indexOfRowAt(y);
  int newIndex = int(rowIndex) + offset;
  if (newIndex < 0)
  {
    while (newIndex < 0 && groupIndex > 0)
    {
      newIndex += int(m_groups[--groupIndex].rows().size());
    }
  }
  else if (newIndex >= int(m_groups[groupIndex].rows().size()))
  {
    while (groupIndex < m_groups.size() - 1
           && newIndex >= int(m_groups[groupIndex].rows().size()))
    {
      newIndex -= int(m_groups[groupIndex++].rows().size());
    }
  }

  if (groupIndex < m_groups.size())
  {
    if (newIndex >= 0)
    {
      rowIndex = size_t(newIndex);
      if (rowIndex < m_groups[groupIndex].rows().size())
      {
        return m_groups[groupIndex].rows()[rowIndex].bounds().top();
      }
    }
  }

  return y;
}

void CellLayout::invalidate()
{
  m_valid = false;
}

void CellLayout::setWidth(const float width)
{
  if (m_width != width)
  {
    m_width = width;
    invalidate();
  }
}

const std::vector<LayoutGroup>& CellLayout::groups()
{
  if (!m_valid)
  {
    validate();
  }

  return m_groups;
}

const LayoutCell* CellLayout::cellAt(const float x, const float y)
{
  if (!m_valid)
  {
    validate();
  }

  for (size_t i = 0; i < m_groups.size(); ++i)
  {
    const auto& group = m_groups[i];
    const auto groupBounds = group.bounds();
    if (y > groupBounds.bottom())
    {
      continue;
    }
    if (y < groupBounds.top())
    {
      return nullptr;
    }
    if (const auto* cell = group.cellAt(x, y))
    {
      return cell;
    }
  }

  return nullptr;
}

void CellLayout::addGroup(std::string title, const float titleHeight)
{
  if (!m_valid)
  {
    validate();
  }

  auto y = 0.0f;
  if (!m_groups.empty())
  {
    y = m_groups.back().bounds().bottom() + m_groupMargin;
    m_height += m_groupMargin;
  }

  m_groups.emplace_back(
    std::move(title),
    m_outerMargin,
    y,
    m_cellMargin,
    m_titleMargin,
    m_rowMargin,
    titleHeight,
    m_width - 2.0f * m_outerMargin,
    m_maxCellsPerRow,
    m_maxUpScale,
    m_minCellWidth,
    m_maxCellWidth,
    m_minCellHeight,
    m_maxCellHeight);
  m_height += m_groups.back().bounds().height;
}

void CellLayout::addItem(
  std::any item,
  const float itemWidth,
  const float itemHeight,
  const float titleWidth,
  const float titleHeight)
{
  if (!m_valid)
  {
    validate();
  }

  if (m_groups.empty())
  {
    m_groups.emplace_back(
      m_outerMargin,
      m_outerMargin,
      m_cellMargin,
      m_titleMargin,
      m_rowMargin,
      m_width - 2.0f * m_outerMargin,
      m_maxCellsPerRow,
      m_maxUpScale,
      m_minCellWidth,
      m_maxCellWidth,
      m_minCellHeight,
      m_maxCellHeight);
    m_height += titleHeight;
    if (titleHeight > 0.0f)
    {
      m_height += m_rowMargin;
    }
  }

  const auto oldGroupHeight = m_groups.back().bounds().height;
  m_groups.back().addItem(
    std::move(item), itemWidth, itemHeight, titleWidth, titleHeight);
  const auto newGroupHeight = m_groups.back().bounds().height;

  m_height += (newGroupHeight - oldGroupHeight);
}

void CellLayout::clear()
{
  m_groups.clear();
  invalidate();
}

void CellLayout::validate()
{
  if (m_width <= 0.0f)
  {
    return;
  }

  m_height = 2.0f * m_outerMargin;
  m_valid = true;
  if (!m_groups.empty())
  {
    auto copy = m_groups;
    m_groups.clear();

    for (auto& group : copy)
    {
      addGroup(group.title(), group.titleBounds().height);
      for (const auto& row : group.rows())
      {
        for (const auto& cell : row.cells())
        {
          const auto& itemBounds = cell.itemBounds();
          const auto& titleBounds = cell.titleBounds();
          const auto scale = cell.scale();
          const auto itemWidth = itemBounds.width / scale;
          const auto itemHeight = itemBounds.height / scale;
          addItem(
            cell.item(), itemWidth, itemHeight, titleBounds.width, titleBounds.height);
        }
      }
    }
  }
}

} // namespace TrenchBroom::View
