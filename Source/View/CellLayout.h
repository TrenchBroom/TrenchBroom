/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_CellLayout_h
#define TrenchBroom_CellLayout_h

#include "Utility/VecMath.h"

#include <algorithm>
#include <cassert>
#include <memory>
#include <vector>

using namespace TrenchBroom::Math;

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
            
            LayoutBounds(float x, float y, float width, float height) :
            m_x(x),
            m_y(y),
            m_width(width),
            m_height(height) {}

            inline float left() const {
                return m_x;
            }

            inline float top() const {
                return m_y;
            }
            
            inline float right() const {
                return m_x + m_width;
            }
            
            inline float bottom() const {
                return m_y + m_height;
            }
            
            inline float midX() const {
                return m_x + m_width / 2.0f;
            }
            
            inline float midY() const {
                return m_y + m_height / 2.0f;
            }
            
            inline float width() const {
                return m_width;
            }
            
            inline float height() const {
                return m_height;
            }
            
            inline bool containsPoint(float x, float y) const {
                return x >= left() && x <= right() && y >= top() && y <= bottom();
            }
            
            inline bool intersectsY(float y, float height) const {
                return bottom() >= y || top() <= y + height;
            }
        };
        
        template <typename CellType>
        class LayoutCell {
        private:
            LayoutBounds m_cellBounds;
            LayoutBounds m_itemBounds;
            LayoutBounds m_titleBounds;
            CellType m_item;
        public:
            LayoutCell(CellType item, float x, float y, float itemWidth, float itemHeight, float titleWidth, float titleHeight, float fixedCellWidth) :
            m_item(item) {
                if (fixedCellWidth > 0) {
                    float scaledItemWidth = itemWidth;
                    float scaledItemHeight = itemHeight;
                    if (scaledItemWidth >= fixedCellWidth) {
                        scaledItemWidth *= fixedCellWidth / scaledItemWidth;
                        scaledItemHeight = fixedCellWidth;
                    }

                    float clippedTitleWidth = (std::min)(fixedCellWidth, titleWidth);
                    
                    m_cellBounds = LayoutBounds(x, y, fixedCellWidth, scaledItemHeight + titleHeight);
                    m_itemBounds = LayoutBounds(x + (m_cellBounds.width() - scaledItemWidth) / 2.0f, y, scaledItemWidth, scaledItemHeight);
                    m_titleBounds = LayoutBounds(x + (m_cellBounds.width() - clippedTitleWidth) / 2.0f, m_itemBounds.bottom(), clippedTitleWidth, titleHeight);
                } else {
                    m_cellBounds = LayoutBounds(x, y, (std::max)(itemWidth, titleWidth), itemHeight + titleHeight);
                    m_itemBounds = LayoutBounds(x + (m_cellBounds.width() - itemWidth) / 2.0f, y, itemWidth, itemHeight);
                    m_titleBounds = LayoutBounds(x + (m_cellBounds.width() - titleWidth) / 2.0f, m_itemBounds.bottom(), titleWidth, titleHeight);
                }
            }
            
            inline bool hitTest(float x, float y) const {
                return m_cellBounds.containsPoint(x, y) || m_titleBounds.containsPoint(x, y);
            }
            
            inline const LayoutBounds& cellBounds() const {
                return m_cellBounds;
            }
            
            inline const LayoutBounds& titleBounds() const {
                return m_titleBounds;
            }
            
            inline const LayoutBounds& itemBounds() const {
                return m_itemBounds;
            }
            
            inline CellType& item() const {
                return m_item;
            }
            
        };
        
        template <typename CellType>
        class LayoutRow {
        public:
            typedef LayoutCell<CellType> Cell;
            typedef std::auto_ptr<Cell> CellPtr;
            typedef std::vector<CellPtr> CellList;
        private:
            CellList m_cells;
            unsigned int m_maxCells;
            float m_maxWidth;
            float m_fixedCellWidth;
            float m_cellMargin;
            LayoutBounds m_bounds;
        public:
            LayoutRow(float y, float cellMargin, float maxWidth, unsigned int maxCells, float fixedCellWidth) :
            m_cellMargin(cellMargin),
            m_maxWidth(maxWidth),
            m_maxCells(maxCells),
            m_fixedCellWidth(fixedCellWidth),
            m_bounds(0.0f, y, 0.0f, 0.0f) {}
            
            inline const Cell* operator[] (const unsigned int index) const {
                assert(index >= 0 && index < m_cells.size());
                return m_cells[index].get();
            }
            
            inline bool addItem(CellType item, float itemWidth, float itemHeight, float titleWidth, float titleHeight) {
                float x = m_bounds.right();
                if (!m_cells.empty())
                    x += m_cellMargin;
                
                CellPtr cell = CellPtr(new Cell(item, x, m_bounds.top(), itemWidth, itemHeight, titleWidth, titleHeight, m_fixedCellWidth));

                if (m_maxCells == 0 && m_bounds.right() + cell->cellBounds().width() + 2 * m_cellMargin > m_maxWidth && !m_cells.empty())
                    return false;
                if (m_maxCells > 0 && m_cells.size() >= m_maxCells - 1)
                    return false;
                
                float width = x + cell->cellBounds().width();
                float height = (std::max)(m_bounds.height(), cell->cellBounds().height());
                m_bounds = LayoutBounds(m_bounds.left(), m_bounds.top(), width, height);
                
                m_cells.push_back(cell);
                return true;
            }

            
            inline const CellList& cells() const {
                return m_cells;
            }
            
            inline bool cellAt(float x, float y, Cell* result) const {
                for (unsigned int i = 0; i < m_cells.size(); i++) {
                    Cell* cell = m_cells[i].get();
                    const LayoutBounds& cellBounds = cell->cellBounds();
                    if (x > cellBounds.right())
                        continue;
                    else if (x < cellBounds.left())
                        break;
                    if (cell->hitTest(x, y)) {
                        result = cell;
                        return true;
                    }
                }
                return false;
            }

            const LayoutBounds& bounds() const {
                return m_bounds;
            }

            bool intersectsY(float y, float height) const {
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
            typedef std::auto_ptr<Row> RowPtr;
            typedef std::vector<RowPtr> RowList;
        private:
            RowList m_rows;
            LayoutBounds m_titleBounds;
            LayoutBounds m_contentBounds;
            unsigned int m_maxCellsPerRow;
            float m_fixedCellWidth;
            float m_maxWidth;
            float m_cellMargin;
            float m_rowMargin;
            GroupType m_item;
        public:
            inline const Row* operator[] (const unsigned int index) const {
                assert(index >= 0 && index < m_rows.size());
                return m_rows[index].get();
            }

            LayoutGroup(GroupType item, float y, float cellMargin, float rowMargin, float titleHeight, float width, int maxCellsPerRow, float fixedCellWidth) : 
                m_item(item), 
                m_cellMargin(cellMargin), 
                m_rowMargin(rowMargin), 
                m_maxCellsPerRow(maxCellsPerRow),
                m_fixedCellWidth(fixedCellWidth),
                m_titleBounds(0.0f, y, width, titleHeight),
                m_contentBounds(0.0f, y + titleHeight, width, 0.0f) {}
            
            LayoutGroup(float y, float cellMargin, float rowMargin, float width, int maxCellsPerRow, float fixedCellWidth) : 
                m_cellMargin(cellMargin), 
                m_rowMargin(rowMargin), 
                m_maxCellsPerRow(maxCellsPerRow),
                m_fixedCellWidth(fixedCellWidth),
                m_titleBounds(0.0f, y, width, 0.0f),
                m_contentBounds(0.0f, y, width, 0.0f) {}
            
            void addItem(CellType item, float itemWidth, float itemHeight, float titleWidth, float titleHeight) {
                Row* row;
                if (m_rows.empty()) {
                    float y = m_contentBounds.top();
                    if (m_titleBounds.height() > 0)
                        y += m_rowMargin;
                    
                    row = new Row(y, m_cellMargin, m_contentBounds.width(), m_maxCellsPerRow, m_fixedCellWidth);
                    m_rows.push_back(RowPtr(row));
                    m_contentBounds = LayoutBounds(m_contentBounds.left(), m_contentBounds.top(), m_contentBounds.width(), m_contentBounds.height() + m_rowMargin);
                } else {
                    row = m_rows.back().get();
                }
                
                const LayoutBounds& oldBounds = row->bounds();
                if (!row->addItem(item, itemWidth, itemHeight, titleWidth, titleHeight)) {
                    float y = oldBounds.bottom() + m_rowMargin;
                    row = new Row(y, m_cellMargin, m_contentBounds.width(), m_maxCellsPerRow, m_fixedCellWidth);
                    m_rows.push_back(RowPtr(row));

                    bool added = (row->addItem(item, itemWidth, itemHeight, titleWidth, titleHeight));
                    assert(added);

                    m_contentBounds = LayoutBounds(m_contentBounds.left(), m_contentBounds.top(), m_contentBounds.width(), m_contentBounds.height() + row->bounds().height() + m_rowMargin);
                } else {
                    m_contentBounds = LayoutBounds(m_contentBounds.left(), m_contentBounds.top(), m_contentBounds.width(), m_contentBounds.height() + (row->bounds().height() - oldBounds.height()));
                }
            }
            
            bool cellAt(float x, float y, typename Row::Cell* result) {
                for (unsigned int i = 0; i < m_rows.size(); i++) {
                    Row* row = m_rows[i].get();
                    const LayoutBounds& rowBounds = row->bounds();
                    if (y > rowBounds.bottom())
                        continue;
                    else if (y < rowBounds.top())
                        break;
                    typename Row::Cell* cell;
                    if (row->cellAt(x, y, cell)) {
                        result = cell;
                        return true;
                    }
                }
                
                return false;
            }

            bool hitTest(float x, float y) const {
                return bounds().containsPoint(x, y);
            }
            
            const LayoutBounds& titleBounds() const {
                return m_titleBounds;
            }
            
            const LayoutBounds titleBoundsForVisibleRect(float y, float height) {
                if (intersectsY(y, height) && m_titleBounds.top() < y) {
                    if (y > m_contentBounds.bottom() - m_titleBounds.height())
                        return LayoutBounds(m_titleBounds.left(), m_contentBounds.bottom() - m_titleBounds.height(), m_titleBounds.width(), m_titleBounds.height());
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
            
            bool intersectsY(float y, float height) const {
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
            typedef std::auto_ptr<Group> GroupPtr;
            typedef std::vector<GroupPtr> GroupList;
        private:
            GroupList m_groups;
            bool m_valid;
            unsigned int m_maxCellsPerRow;
            float m_fixedCellWidth;
            float m_width;
            float m_height;
            float m_cellMargin;
            float m_rowMargin;
            float m_groupMargin;

            void validate() {
                if (m_width <= 0)
                    return;
                
                m_height = 0;
                m_valid = true;
                if (!m_groups.empty()) {
                    GroupList copy = m_groups;
                    m_groups.clear();
                    
                    for (unsigned int i = 0; i < copy.size(); i++) {
                        Group* group = copy[i].get();
                        addGroup(group->item(), group->titleBounds().height());
                        for (unsigned int j = 0; j < group->size(); j++) {
                            const typename Group::Row* row = group[j];
                            for (unsigned int k = 0; k < row->size(); k++) {
                                const typename Group::Row::Cell* cell = (*row)[k];
                                const LayoutBounds& itemBounds = cell->itemBounds();
                                const LayoutBounds& titleBounds = cell->titleBounds();
                                addItem(cell->item(), itemBounds.width(), itemBounds.height(), titleBounds.width(), titleBounds.height());
                            }
                        }
                    }
                }
            }
        public:
            const Group* operator[] (const unsigned int index) {
                assert(index >= 0 && index < m_groups.size());
                if (!m_valid)
                    validate();
                return m_groups[index].get();
            }

            CellLayout(float fixedCellWidth) : m_width(1), m_cellMargin(0), m_rowMargin(0), m_groupMargin(0), m_maxCellsPerRow(-1), m_fixedCellWidth(fixedCellWidth) {
                invalidate();
            }
            
            CellLayout(int maxCellsPerRow = 0) : m_width(1), m_cellMargin(0), m_rowMargin(0), m_groupMargin(0), m_maxCellsPerRow(maxCellsPerRow), m_fixedCellWidth(0) {
                invalidate();
            }
            
            void setCellMargin(float cellMargin) {
                if (m_cellMargin == cellMargin)
                    return;
                m_cellMargin = cellMargin;
                invalidate();
            }
            
            void setRowMargin(float rowMargin) {
                if (m_rowMargin == rowMargin)
                    return;
                m_rowMargin = rowMargin;
                invalidate();
            }
            
            void setGroupMargin(float groupMargin) {
                if (m_groupMargin == groupMargin)
                    return;
                m_groupMargin = groupMargin;
                invalidate();
            }
            
            void addGroup(const GroupType groupItem, float titleHeight) {
                if (!m_valid)
                    validate();
                
                float y = 0;
                if (!m_groups.empty())
                    y += m_groups.back()->bounds().bottom() + m_groupMargin;
                
                m_height += titleHeight;
                if (!m_groups.empty())
                    m_height += m_groupMargin;
                
                GroupPtr group = GroupPtr(new Group(groupItem, y, m_cellMargin, m_rowMargin, titleHeight, m_width, m_maxCellsPerRow, m_fixedCellWidth));
                m_groups.push_back(group);
            }
            
            void addItem(const CellType item, float itemWidth, float itemHeight, float titleWidth, float titleHeight) {
                if (!m_valid)
                    validate();
                
                Group* group;
                if (m_groups.empty()) {
                    group = new Group(0.0f, m_cellMargin, m_rowMargin, m_width, m_maxCellsPerRow, m_fixedCellWidth);
                    m_groups.push_back(GroupPtr(group));
                    m_height += titleHeight;
                    if (titleHeight > 0.0f)
                        m_height += m_rowMargin;
                } else {
                    group = m_groups.back().get();
                }
                
                const LayoutBounds oldBounds = group->bounds();
                group->addItem(item, itemWidth, itemHeight, titleWidth, titleHeight);
                m_height += (group->bounds().height() - oldBounds.height());
            }
            
            void clear() {
                m_groups.clear();
                invalidate();
            }
            
            bool cellAt(float x, float y, typename Group::Row::Cell* result) {
                if (!m_valid)
                    validate();
                
                for (unsigned int i = 0; i < m_groups.size(); i++) {
                    Group group = m_groups[i].get();
                    const LayoutBounds groupBounds = group->bounds();
                    if (y > groupBounds.bottom())
                        continue;
                    else if (y < groupBounds.top())
                        break;
                    typename Group::Row::Cell* cell;
                    if (group->cellAt(x, y, cell)) {
                        result = cell;
                        return true;
                    }
                }
                
                return false;
            }
            
            bool groupAt(float x, float y, Group* result) {
                if (!m_valid)
                    validate();
                
                for (unsigned int i = 0; i < m_groups.size(); i++) {
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
            
            size_t size() {
                if (!m_valid)
                    validate();
                return m_groups.size();
            }
            
            void invalidate() {
                m_valid = false;
            }
            
            void setWidth(float width) {
                if (m_width == width)
                    return;
                m_width = width;
                invalidate();
            }
            
            void setFixedCellWidth(float fixedCellWidth) {
                if (m_fixedCellWidth == fixedCellWidth)
                    return;
                m_fixedCellWidth = fixedCellWidth;
                invalidate();
            }
            
            float fixedCellWidth() const {
                return m_fixedCellWidth;
            }

            float width() const {
                return m_width;
            }
            
            float height() {
                if (!m_valid)
                    validate();
                return m_height;
            }
        };
    }
}

#endif
