/*
 Copyright (C) 2010-2014 Kristian Duske

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

#ifndef TrenchBroom_CellLayout_h
#define TrenchBroom_CellLayout_h

#include "VecMath.h"
#include "Macros.h"

#include <algorithm>
#include <cassert>
#include <limits>
#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace View {
        class LayoutBounds {
        private:
            float m_x;
            float m_y;
            float m_width;
            float m_height;
        public:
            LayoutBounds() :
            m_x(0.0f),
            m_y(0.0f),
            m_width(0.0f),
            m_height(0.0f) {}

            LayoutBounds(const float x, const float y, const float width, const float height) :
            m_x(x),
            m_y(y),
            m_width(width),
            m_height(height) {}

            float left() const {
                return m_x;
            }

            float top() const {
                return m_y;
            }

            float right() const {
                return m_x + m_width;
            }

            float bottom() const {
                return m_y + m_height;
            }

            float midX() const {
                return m_x + m_width / 2.0f;
            }

            float midY() const {
                return m_y + m_height / 2.0f;
            }
            
            float width() const {
                return m_width;
            }

            float height() const {
                return m_height;
            }

            bool containsPoint(const float x, const float y) const {
                return x >= left() && x <= right() && y >= top() && y <= bottom();
            }

            bool intersectsY(const float y, const float height) const {
                return bottom() >= y && top() <= y + height ;
            }
        };

        template <typename CellType>
        class LayoutCell {
        public:
        private:
            CellType m_item;
            float m_x;
            float m_y;
            float m_itemWidth;
            float m_itemHeight;
            float m_titleWidth;
            float m_titleHeight;
            float m_titleMargin;
            float m_scale;
            LayoutBounds m_cellBounds;
            LayoutBounds m_itemBounds;
            LayoutBounds m_titleBounds;
            
            void doLayout(const float maxUpScale,
                                 const float minWidth, const float maxWidth,
                                 const float minHeight, const float maxHeight) {
                assert(0.0f < minWidth);
                assert(0.0f < minHeight);
                assert(minWidth <= maxWidth);
                assert(minHeight <= maxHeight);

                m_scale = std::min(std::min(maxWidth / m_itemWidth, maxHeight / m_itemHeight), maxUpScale);
                const float scaledItemWidth = m_scale * m_itemWidth;
                const float scaledItemHeight = m_scale * m_itemHeight;
                const float clippedTitleWidth = std::min(m_titleWidth, maxWidth);
                const float cellWidth = std::max(minWidth, std::max(scaledItemWidth, clippedTitleWidth));
                const float cellHeight = std::max(minHeight, std::max(minHeight, scaledItemHeight) + m_titleHeight + m_titleMargin);
                const float itemY = m_y + std::max(0.0f, cellHeight - m_titleHeight - scaledItemHeight - m_titleMargin);
                    
                m_cellBounds = LayoutBounds(m_x,
                                            m_y,
                                            cellWidth,
                                            cellHeight);
                m_itemBounds = LayoutBounds(m_x + (m_cellBounds.width() - scaledItemWidth) / 2.0f,
                                            itemY,
                                            scaledItemWidth,
                                            scaledItemHeight);
                m_titleBounds = LayoutBounds(m_x + (m_cellBounds.width() - clippedTitleWidth) / 2.0f,
                                             m_itemBounds.bottom() + m_titleMargin,
                                             clippedTitleWidth,
                                             m_titleHeight);
            }
        public:
            LayoutCell(const CellType item,
                       const float x, const float y,
                       const float itemWidth, const float itemHeight,
                       const float titleWidth, const float titleHeight,
                       const float titleMargin,
                       const float maxUpScale,
                       const float minWidth, const float maxWidth,
                       const float minHeight, const float maxHeight) :
            m_item(item),
            m_x(x),
            m_y(y),
            m_itemWidth(itemWidth),
            m_itemHeight(itemHeight),
            m_titleWidth(titleWidth),
            m_titleHeight(titleHeight),
            m_titleMargin(titleMargin) {
                doLayout(maxUpScale, minWidth, maxWidth, minHeight, maxHeight);
            }

            bool hitTest(const float x, const float y) const {
                return m_cellBounds.containsPoint(x, y) || m_titleBounds.containsPoint(x, y);
            }

            float scale() const {
                return m_scale;
            }
            
            const LayoutBounds& cellBounds() const {
                return m_cellBounds;
            }

            const LayoutBounds& titleBounds() const {
                return m_titleBounds;
            }

            const LayoutBounds& itemBounds() const {
                return m_itemBounds;
            }

            void updateLayout(const float maxUpScale,
                                     const float minWidth, const float maxWidth,
                                     const float minHeight, const float maxHeight) {
                doLayout(maxUpScale, minWidth, maxWidth, minHeight, maxHeight);
            }

            CellType item() const {
                return m_item;
            }

        };

        template <typename CellType>
        class LayoutRow {
        public:
            typedef LayoutCell<CellType> Cell;
            typedef std::vector<Cell> CellList;
        private:
            float m_cellMargin;
            float m_titleMargin;
            float m_maxWidth;
            size_t m_maxCells;
            float m_maxUpScale;
            float m_minCellWidth;
            float m_maxCellWidth;
            float m_minCellHeight;
            float m_maxCellHeight;
            LayoutBounds m_bounds;

            CellList m_cells;

            void readjustItems() {
                for (size_t i = 0; i < m_cells.size(); ++i)
                    m_cells[i].updateLayout(m_maxUpScale, m_minCellWidth, m_maxCellWidth, m_minCellHeight, m_maxCellHeight);
            }
        public:
            LayoutRow(const float x, const float y,
                      const float cellMargin,
                      const float titleMargin,
                      const float maxWidth,
                      const size_t maxCells,
                      const float maxUpScale,
                      const float minCellWidth, const float maxCellWidth,
                      const float minCellHeight, const float maxCellHeight) :
            m_cellMargin(cellMargin),
            m_titleMargin(titleMargin),
            m_maxWidth(maxWidth),
            m_maxCells(maxCells),
            m_maxUpScale(maxUpScale),
            m_minCellWidth(minCellWidth),
            m_maxCellWidth(maxCellWidth),
            m_minCellHeight(minCellHeight),
            m_maxCellHeight(maxCellHeight),
            m_bounds(x, y, 0.0f, 0.0f) {}

            const Cell& operator[] (const size_t index) const {
                assert(index >= 0 && index < m_cells.size());
                return m_cells[index];
            }

            bool addItem(CellType item,
                                const float itemWidth, const float itemHeight,
                                const float titleWidth, const float titleHeight) {
                float x = m_bounds.right();
                float width = m_bounds.width();
                if (!m_cells.empty()) {
                    x += m_cellMargin;
                    width += m_cellMargin;
                }

                Cell cell(item, x, m_bounds.top(), itemWidth, itemHeight, titleWidth, titleHeight, m_titleMargin, m_maxUpScale, m_minCellWidth, m_maxCellWidth, m_minCellHeight, m_maxCellHeight);
                width += cell.cellBounds().width();

                if (m_maxCells == 0 && width > m_maxWidth && !m_cells.empty())
                    return false;
                if (m_maxCells > 0 && m_cells.size() >= m_maxCells - 1)
                    return false;

                const float newItemRowHeight = cell.cellBounds().height() - cell.titleBounds().height() - m_titleMargin;
                bool readjust = newItemRowHeight > m_minCellHeight;
                if (readjust) {
                    m_minCellHeight = newItemRowHeight;
                    assert(m_minCellHeight <= m_maxCellHeight);
                    readjustItems();
                }
                
                m_bounds = LayoutBounds(m_bounds.left(), m_bounds.top(), width, std::max(m_bounds.height(), cell.cellBounds().height()));

                m_cells.push_back(cell);
                return true;
            }


            const CellList& cells() const {
                return m_cells;
            }

            bool cellAt(const float x, const float y, const Cell** result) const {
                for (size_t i = 0; i < m_cells.size(); ++i) {
                    const Cell& cell = m_cells[i];
                    const LayoutBounds& cellBounds = cell.cellBounds();
                    if (x > cellBounds.right())
                        continue;
                    else if (x < cellBounds.left())
                        break;
                    if (cell.hitTest(x, y)) {
                        *result = &cell;
                        return true;
                    }
                }
                return false;
            }

            const LayoutBounds& bounds() const {
                return m_bounds;
            }

            bool intersectsY(const float y, const float height) const {
                return m_bounds.intersectsY(y, height);
            }

            size_t size() const {
                return m_cells.size();
            }
        };

        template <typename CellType, typename GroupType>
        class LayoutGroup {
        public:
            typedef LayoutRow<CellType> Row;
            typedef std::vector<Row> RowList;
        private:
            GroupType m_item;
            float m_cellMargin;
            float m_titleMargin;
            float m_rowMargin;
            size_t m_maxCellsPerRow;
            float m_maxUpScale;
            float m_minCellWidth;
            float m_maxCellWidth;
            float m_minCellHeight;
            float m_maxCellHeight;
            LayoutBounds m_titleBounds;
            LayoutBounds m_contentBounds;

            RowList m_rows;
        public:
            const Row& operator[] (const size_t index) const {
                assert(index >= 0 && index < m_rows.size());
                return m_rows[index];
            }

            LayoutGroup(GroupType item,
                        const float x, const float y,
                        const float cellMargin, const float titleMargin, const float rowMargin,
                        const float titleHeight,
                        const float width,
                        const size_t maxCellsPerRow,
                        const float maxUpScale,
                        const float minCellWidth, const float maxCellWidth,
                        const float minCellHeight, const float maxCellHeight) :
            m_item(item),
            m_cellMargin(cellMargin),
            m_titleMargin(titleMargin),
            m_rowMargin(rowMargin),
            m_maxCellsPerRow(maxCellsPerRow),
            m_maxUpScale(maxUpScale),
            m_minCellWidth(minCellWidth),
            m_maxCellWidth(maxCellWidth),
            m_minCellHeight(minCellHeight),
            m_maxCellHeight(maxCellHeight),
            m_titleBounds(0.0f, y, width + 2.0f * x, titleHeight),
            m_contentBounds(x, y + titleHeight + m_rowMargin, width, 0.0f),
            m_rows() {}

            LayoutGroup(const float x, const float y,
                        const float cellMargin, const float titleMargin, const float rowMargin,
                        const float width,
                        const size_t maxCellsPerRow,
                        const float maxUpScale,
                        const float minCellWidth, const float maxCellWidth,
                        const float minCellHeight, const float maxCellHeight) :
            m_cellMargin(cellMargin),
            m_titleMargin(titleMargin),
            m_rowMargin(rowMargin),
            m_maxCellsPerRow(maxCellsPerRow),
            m_maxUpScale(maxUpScale),
            m_minCellWidth(minCellWidth),
            m_maxCellWidth(maxCellWidth),
            m_minCellHeight(minCellHeight),
            m_maxCellHeight(maxCellHeight),
            m_titleBounds(x, y, width, 0.0f),
            m_contentBounds(x, y, width, 0.0f),
            m_rows() {}

            void addItem(CellType item,
                         const float itemWidth, const float itemHeight,
                         const float titleWidth, const float titleHeight) {
                if (m_rows.empty()) {
                    const float y = m_contentBounds.top();
                    m_rows.push_back(Row(m_contentBounds.left(), y, m_cellMargin, m_titleMargin, m_contentBounds.width(), m_maxCellsPerRow, m_maxUpScale, m_minCellWidth, m_maxCellWidth, m_minCellHeight, m_maxCellHeight));
                }

                const LayoutBounds oldBounds = m_rows.back().bounds();
                const float oldRowHeight = m_rows.back().bounds().height();
                if (!m_rows.back().addItem(item, itemWidth, itemHeight, titleWidth, titleHeight)) {
                    const float y = oldBounds.bottom() + m_rowMargin;
                    m_rows.push_back(Row(m_contentBounds.left(), y, m_cellMargin, m_titleMargin, m_contentBounds.width(), m_maxCellsPerRow, m_maxUpScale, m_minCellWidth, m_maxCellWidth, m_minCellHeight, m_maxCellHeight));

                    const bool added = (m_rows.back().addItem(item, itemWidth, itemHeight, titleWidth, titleHeight));
                    assert(added);
                    unused(added);

                    const float newRowHeight = m_rows.back().bounds().height();
                    m_contentBounds = LayoutBounds(m_contentBounds.left(), m_contentBounds.top(), m_contentBounds.width(), m_contentBounds.height() + newRowHeight + m_rowMargin);
                } else {
                    const float newRowHeight = m_rows.back().bounds().height();
                    m_contentBounds = LayoutBounds(m_contentBounds.left(), m_contentBounds.top(), m_contentBounds.width(), m_contentBounds.height() + (newRowHeight - oldRowHeight));
                }
            }

            size_t indexOfRowAt(const float y) const {
                for (size_t i = 0; i < m_rows.size(); ++i) {
                    const Row& row = m_rows[i];
                    const LayoutBounds& rowBounds = row.bounds();
                    if (y < rowBounds.bottom())
                        return i;
                }
                
                return m_rows.size();
            }
            
            bool rowAt(const float y, const Row** result) const {
                size_t index = indexOfRowAt(y);
                if (index == m_rows.size())
                    return false;
                
                *result = &m_rows[index];
                return true;
            }
            
            bool cellAt(const float x, const float y, const typename Row::Cell** result) const {
                for (size_t i = 0; i < m_rows.size(); ++i) {
                    const Row& row = m_rows[i];
                    const LayoutBounds& rowBounds = row.bounds();
                    if (y > rowBounds.bottom())
                        continue;
                    else if (y < rowBounds.top())
                        break;
                    if (row.cellAt(x, y, result))
                        return true;
                }

                return false;
            }

            bool hitTest(const float x, const float y) const {
                return bounds().containsPoint(x, y);
            }

            const LayoutBounds& titleBounds() const {
                return m_titleBounds;
            }

            const LayoutBounds titleBoundsForVisibleRect(const float y, const float height, const float groupMargin) const {
                if (intersectsY(y, height) && m_titleBounds.top() < y) {
                    if (y > m_contentBounds.bottom() - m_titleBounds.height() + groupMargin)
                        return LayoutBounds(m_titleBounds.left(), m_contentBounds.bottom() - m_titleBounds.height() + groupMargin, m_titleBounds.width(), m_titleBounds.height());
                    return LayoutBounds(m_titleBounds.left(), y, m_titleBounds.width(), m_titleBounds.height());
                }
                return m_titleBounds;
            }

            const LayoutBounds& contentBounds() const {
                return m_contentBounds;
            }

            const LayoutBounds bounds() const {
                return LayoutBounds(m_titleBounds.left(), m_titleBounds.top(), m_titleBounds.width(), m_contentBounds.bottom() - m_titleBounds.top());
            }

            bool intersectsY(const float y, const float height) const {
                return bounds().intersectsY(y, height);
            }

            GroupType item() const {
                return m_item;
            }

            size_t size() const {
                return m_rows.size();
            }
        };

        template <typename CellType, typename GroupType>
        class CellLayout {
        public:
            typedef LayoutGroup<CellType, GroupType> Group;
            typedef std::vector<Group> GroupList;
        private:
            float m_width;
            float m_cellMargin;
            float m_titleMargin;
            float m_rowMargin;
            float m_groupMargin;
            float m_outerMargin;
            size_t m_maxCellsPerRow;
            float m_maxUpScale;
            float m_minCellWidth;
            float m_maxCellWidth;
            float m_minCellHeight;
            float m_maxCellHeight;

            GroupList m_groups;
            bool m_valid;
            float m_height;

            void validate() {
                if (m_width <= 0.0f)
                    return;

                m_height = 2.0f * m_outerMargin;
                m_valid = true;
                if (!m_groups.empty()) {
                    GroupList copy = m_groups;
                    m_groups.clear();

                    for (size_t i = 0; i < copy.size(); ++i) {
                        Group& group = copy[i];
                        addGroup(group.item(), group.titleBounds().height());
                        for (size_t j = 0; j < group.size(); ++j) {
                            const typename Group::Row& row = group[j];
                            for (size_t k = 0; k < row.size(); k++) {
                                const typename Group::Row::Cell& cell = row[k];
                                const LayoutBounds& itemBounds = cell.itemBounds();
                                const LayoutBounds& titleBounds = cell.titleBounds();
                                float scale = cell.scale();
                                float itemWidth = itemBounds.width() / scale;
                                float itemHeight = itemBounds.height() / scale;
                                addItem(cell.item(), itemWidth, itemHeight, titleBounds.width(), titleBounds.height());
                            }
                        }
                    }
                }
            }
        public:
            const Group& operator[] (const size_t index) {
                assert(index >= 0 && index < m_groups.size());
                if (!m_valid)
                    validate();
                    return m_groups[index];
            }

            CellLayout(const size_t maxCellsPerRow = 0) :
            m_width(1.0f),
            m_cellMargin(0.0f),
            m_titleMargin(0.0f),
            m_rowMargin(0.0f),
            m_groupMargin(0.0f),
            m_outerMargin(0.0f),
            m_maxCellsPerRow(maxCellsPerRow),
            m_maxUpScale(1.0f),
            m_minCellWidth(100.0f),
            m_maxCellWidth(100.0f),
            m_minCellHeight(100.0f),
            m_maxCellHeight(100.0f),
            m_groups(),
            m_valid(false),
            m_height(0.0f) {
                invalidate();
            }

            void setCellMargin(const float cellMargin) {
                if (m_cellMargin == cellMargin)
                    return;
                m_cellMargin = cellMargin;
                invalidate();
            }
            
            void setTitleMargin(const float titleMargin) {
                if (m_titleMargin == titleMargin)
                    return;
                m_titleMargin = titleMargin;
                invalidate();
            }

            void setRowMargin(const float rowMargin) {
                if (m_rowMargin == rowMargin)
                    return;
                m_rowMargin = rowMargin;
                invalidate();
            }

            void setGroupMargin(const float groupMargin) {
                if (m_groupMargin == groupMargin)
                    return;
                m_groupMargin = groupMargin;
                invalidate();
            }

            void setOuterMargin(const float outerMargin) {
                if (m_outerMargin == outerMargin)
                    return;
                m_outerMargin = outerMargin;
                invalidate();
            }

            void addGroup(const GroupType groupItem, const float titleHeight) {
                if (!m_valid)
                    validate();

                float y = 0.0f;
                if (!m_groups.empty()) {
                    y = m_groups.back().bounds().bottom() + m_groupMargin;
                    m_height += m_groupMargin;
                }

                m_groups.push_back(Group(groupItem, m_outerMargin, y, m_cellMargin, m_titleMargin, m_rowMargin, titleHeight, m_width - 2.0f * m_outerMargin, m_maxCellsPerRow, m_maxUpScale, m_minCellWidth, m_maxCellWidth, m_minCellHeight, m_maxCellHeight));
                m_height += m_groups.back().bounds().height();
            }

            void addItem(const CellType item,
                         const float itemWidth, const float itemHeight,
                         const float titleWidth, const float titleHeight) {
                if (!m_valid)
                    validate();

                if (m_groups.empty()) {
                    m_groups.push_back(Group(m_outerMargin, m_outerMargin, m_cellMargin, m_titleMargin, m_rowMargin, m_width - 2.0f * m_outerMargin, m_maxCellsPerRow, m_maxUpScale, m_minCellWidth, m_maxCellWidth, m_minCellHeight, m_maxCellHeight));
                    m_height += titleHeight;
                    if (titleHeight > 0.0f)
                        m_height += m_rowMargin;
                }

                const float oldGroupHeight = m_groups.back().bounds().height();
                m_groups.back().addItem(item, itemWidth, itemHeight, titleWidth, titleHeight);
                const float newGroupHeight = m_groups.back().bounds().height();

                m_height += (newGroupHeight - oldGroupHeight);
            }

            void clear() {
                m_groups.clear();
                invalidate();
            }

            bool cellAt(const float x, const float y, const typename Group::Row::Cell** result) {
                if (!m_valid)
                    validate();

                for (size_t i = 0; i < m_groups.size(); ++i) {
                    const Group& group = m_groups[i];
                    const LayoutBounds groupBounds = group.bounds();
                    if (y > groupBounds.bottom())
                        continue;
                    else if (y < groupBounds.top())
                        break;
                    if (group.cellAt(x, y, result))
                        return true;
                }

                return false;
            }

            bool groupAt(const float x, const float y, Group* result) {
                if (!m_valid)
                    validate();

                for (size_t i = 0; i < m_groups.size(); ++i) {
                    Group* group = m_groups[i];
                    const LayoutBounds groupBounds = group->bounds();
                    if (y > groupBounds.bottom())
                        continue;
                    else if (y < groupBounds.top())
                        break;
                    if (group->hitTest(x, y)) {
                        result = group;
                        return true;
                    }
                }

                return false;
            }

            const LayoutBounds titleBoundsForVisibleRect(const Group& group, const float y, const float height) const {
                return group.titleBoundsForVisibleRect(y, height, m_groupMargin);
            }

            float rowPosition(const float y, const int offset) {
                if (!m_valid)
                    validate();

                size_t groupIndex = m_groups.size();
                for (size_t i = 0; i < m_groups.size(); ++i) {
                    Group* candidate = &m_groups[i];
                    const LayoutBounds groupBounds = candidate->bounds();
                    if (y + m_rowMargin > groupBounds.bottom())
                        continue;
                    groupIndex = i;
                    break;
                }

                if (groupIndex == m_groups.size())
                    return y;
                
                size_t rowIndex = m_groups[groupIndex].indexOfRowAt(y);
                if (rowIndex == m_groups[groupIndex].size())
                    return y;

                if (offset == 0)
                    return y;
                
                int newIndex = static_cast<int>(rowIndex) + offset;
                if (newIndex < 0) {
                    while (newIndex < 0 && groupIndex > 0)
                        newIndex += m_groups[--groupIndex].size();
                } else if (newIndex >= static_cast<int>(m_groups[groupIndex].size())) {
                    while (newIndex >= static_cast<int>(m_groups[groupIndex].size()) && groupIndex < m_groups.size() - 1)
                        newIndex -= m_groups[groupIndex++].size();
                }
                
                if (groupIndex < m_groups.size()) {
                    if (newIndex >= 0) {
                        rowIndex = static_cast<size_t>(newIndex);
                        if (rowIndex < m_groups[groupIndex].size()) {
                            return m_groups[groupIndex][rowIndex].bounds().top();
                        }
                    }
                }
                
                
                return y;
            }
            
            size_t size() {
                if (!m_valid)
                    validate();
                return m_groups.size();
            }

            void invalidate() {
                m_valid = false;
            }

            void setWidth(const float width) {
                if (m_width == width)
                    return;
                m_width = width;
                invalidate();
            }

            float minCellWidth() const {
                return m_minCellWidth;
            }
            
            float maxCellWidth() const {
                return m_maxCellWidth;
            }
            
            void setCellWidth(const float minCellWidth, const float maxCellWidth) {
                assert(0.0f < minCellWidth);
                assert(minCellWidth <= maxCellWidth);
                
                if (m_minCellWidth == minCellWidth && m_maxCellWidth == maxCellWidth)
                    return;
                m_minCellWidth = minCellWidth;
                m_maxCellWidth = maxCellWidth;
                invalidate();
            }
            
            float minCellHeight() const {
                return m_minCellHeight;
            }
            
            float maxCellHeight() const {
                return m_maxCellHeight;
            }
            
            void setCellHeight(const float minCellHeight, const float maxCellHeight) {
                assert(0.0f < minCellHeight);
                assert(minCellHeight <= maxCellHeight);
                
                if (m_minCellHeight == minCellHeight && m_maxCellHeight == maxCellHeight)
                    return;
                m_minCellHeight = minCellHeight;
                m_maxCellHeight = maxCellHeight;
                invalidate();
            }
            
            void setMaxUpScale(const float maxUpScale) {
                if (m_maxUpScale == maxUpScale)
                    return;
                m_maxUpScale = maxUpScale;
                invalidate();
            }

            float width() const {
                return m_width;
            }

            float height() {
                if (!m_valid)
                    validate();
                return m_height;
            }
            
            float outerMargin() const {
                return m_outerMargin();
            }
            
            float groupMargin() const {
                return m_groupMargin;
            }
            
            float rowMargin() const {
                return m_rowMargin;
            }
            
            float cellMargin() const {
                return m_cellMargin();
            }
        };
    }
}

#endif
