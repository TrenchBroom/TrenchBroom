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

#include "Utilities/SharedPointer.h"
#include "Utilities/VecMath.h"
#include <vector>
#include <cassert>

namespace TrenchBroom {
    namespace Gui {
        class LayoutBounds {
        private:
            float m_x;
            float m_y;
            float m_width;
            float m_height;
        public:
            LayoutBounds() : m_x(0), m_y(0), m_width(0), m_height(0) {}
            LayoutBounds(float x, float y, float width, float height) : m_x(x), m_y(y), m_width(width), m_height(height) {}

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
            
            float width() const {
                return m_width;
            }
            
            float height() const {
                return m_height;
            }
            
            bool containsPoint(float x, float y) const {
                return x >= left() && x <= right() && y >= top() && y <= bottom();
            }
            
            bool intersectsY(float y, float height) const {
                return bottom() >= y || top() <= y + height;
            }
        };
        
        template <typename CellType>
        class Cell {
        private:
            LayoutBounds m_cellBounds;
            LayoutBounds m_itemBounds;
            LayoutBounds m_titleBounds;
            CellType m_item;
        public:
            Cell(CellType item, float x, float y, float itemWidth, float itemHeight, float titleWidth, float titleHeight, float fixedCellWidth) : m_item(item) {
                if (fixedCellWidth > 0) {
                    float scaledItemWidth = itemWidth;
                    float scaledItemHeight = itemHeight;
                    if (scaledItemWidth >= fixedCellWidth) {
                        scaledItemWidth *= fixedCellWidth / scaledItemWidth;
                        scaledItemHeight = fixedCellWidth;
                    }

                    float clippedTitleWidth = Math::fmin(fixedCellWidth, titleWidth);
                    
                    m_cellBounds = LayoutBounds(x, y, fixedCellWidth, scaledItemHeight + titleHeight);
                    m_itemBounds = LayoutBounds(x + (m_cellBounds.width() - scaledItemWidth) / 2.0f, y, scaledItemWidth, scaledItemHeight);
                    m_titleBounds = LayoutBounds(x + (m_cellBounds.width() - clippedTitleWidth) / 2.0f, m_itemBounds.bottom(), clippedTitleWidth, titleHeight);
                } else {
                    m_cellBounds = LayoutBounds(x, y, Math::fmax(itemWidth, titleWidth), itemHeight + titleHeight);
                    m_itemBounds = LayoutBounds(x + (m_cellBounds.width() - itemWidth) / 2.0f, y, itemWidth, itemHeight);
                    m_titleBounds = LayoutBounds(x + (m_cellBounds.width() - titleWidth) / 2.0f, m_itemBounds.bottom(), titleWidth, titleHeight);
                }
            }
            
            bool hitTest(float x, float y) const {
                return m_cellBounds.containsPoint(x, y) || m_titleBounds.containsPoint(x, y);
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
            
            CellType item() const {
                return m_item;
            }
            
        };
        
        template <typename CellType>
        class CellRow {
        public:
            typedef std::tr1::shared_ptr<Cell<CellType> > CellPtr;
        private:
            std::vector<CellPtr> m_cells;
            unsigned int m_maxCells;
            float m_maxWidth;
            float m_fixedCellWidth;
            float m_cellMargin;
            LayoutBounds m_bounds;
        public:
            const CellPtr operator[] (const unsigned int index) const {
                assert(index >= 0 && index < m_cells.size());
                return m_cells[index];
            }

            CellRow(float y, float cellMargin, float maxWidth, unsigned int maxCells, float fixedCellWidth) : m_cellMargin(cellMargin), m_maxWidth(maxWidth), m_maxCells(maxCells), m_fixedCellWidth(fixedCellWidth), m_bounds(0, y, 0, 0) { }
            
            bool addItem(CellType item, float itemWidth, float itemHeight, float titleWidth, float titleHeight) {
                float x = m_bounds.right();
                if (!m_cells.empty())
                    x += m_cellMargin;
                Cell<CellType>* cell = new Cell<CellType>(item, x, m_bounds.top(), itemWidth, itemHeight, titleWidth, titleHeight, m_fixedCellWidth);
                CellPtr cellPtr(cell);

                if (m_maxCells == 0 && m_bounds.right() + cellPtr->cellBounds().width() + 2 * m_cellMargin > m_maxWidth && !m_cells.empty())
                    return false;
                if (m_maxCells > 0 && m_cells.size() >= m_maxCells - 1)
                    return false;
                
                float width = x + cellPtr->cellBounds().width();
                float height = Math::fmax(m_bounds.height(), cellPtr->cellBounds().height());
                m_bounds = LayoutBounds(m_bounds.left(), m_bounds.top(), width, height);
                
                m_cells.push_back(cellPtr);
                return true;
            }

            const std::vector<CellPtr>& cells() const {
                return m_cells;
            }
            
            bool cellAt(float x, float y, CellPtr& result) const {
                for (unsigned int i = 0; i < m_cells.size(); i++) {
                    CellPtr cell = m_cells[i];
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
        class CellGroup {
        public:
            typedef std::tr1::shared_ptr<CellRow<CellType> > CellRowPtr;
        private:
            std::vector<CellRowPtr> m_rows;
            LayoutBounds m_titleBounds;
            LayoutBounds m_contentBounds;
            unsigned int m_maxCellsPerRow;
            float m_fixedCellWidth;
            float m_maxWidth;
            float m_cellMargin;
            float m_rowMargin;
            GroupType m_item;
        public:
            const CellRowPtr operator[] (const unsigned int index) const {
                assert(index >= 0 && index < m_rows.size());
                return m_rows[index];
            }

            CellGroup(GroupType item, float y, float cellMargin, float rowMargin, float titleHeight, float width, int maxCellsPerRow, float fixedCellWidth) : 
                m_item(item), 
                m_cellMargin(cellMargin), 
                m_rowMargin(rowMargin), 
                m_maxCellsPerRow(maxCellsPerRow),
                m_fixedCellWidth(fixedCellWidth),
                m_titleBounds(0, y, width, titleHeight),
                m_contentBounds(0, y + titleHeight, width, 0) {
                }
            
            CellGroup(float y, float cellMargin, float rowMargin, float width, int maxCellsPerRow, float fixedCellWidth) : 
                m_cellMargin(cellMargin), 
                m_rowMargin(rowMargin), 
                m_maxCellsPerRow(maxCellsPerRow),
                m_fixedCellWidth(fixedCellWidth),
                m_titleBounds(0, y, width, 0),
                m_contentBounds(0, y, width, 0) {}
            
            void addItem(CellType item, float itemWidth, float itemHeight, float titleWidth, float titleHeight) {
                CellRowPtr rowPtr;
                if (m_rows.empty()) {
                    float y = m_contentBounds.top();
                    if (m_titleBounds.height() > 0)
                        y += m_rowMargin;
                    CellRow<CellType>* row = new CellRow<CellType>(y, m_cellMargin, m_contentBounds.width(), m_maxCellsPerRow, m_fixedCellWidth);
                    rowPtr = CellRowPtr(row);
                    m_rows.push_back(rowPtr);
                    m_contentBounds = LayoutBounds(m_contentBounds.left(), m_contentBounds.top(), m_contentBounds.width(), m_contentBounds.height() + m_rowMargin);
                } else {
                    rowPtr = m_rows.back();
                }
                
                const LayoutBounds oldBounds = rowPtr->bounds();
                if (!rowPtr->addItem(item, itemWidth, itemHeight, titleWidth, titleHeight)) {
                    float y = oldBounds.bottom() + m_rowMargin;
                    CellRow<CellType>* row = new CellRow<CellType>(y, m_cellMargin, m_contentBounds.width(), m_maxCellsPerRow, m_fixedCellWidth);
                    rowPtr = CellRowPtr(row);
                    m_rows.push_back(rowPtr);
                    assert(rowPtr->addItem(item, itemWidth, itemHeight, titleWidth, titleHeight));
                    m_contentBounds = LayoutBounds(m_contentBounds.left(), m_contentBounds.top(), m_contentBounds.width(), m_contentBounds.height() + rowPtr->bounds().height() + m_rowMargin);
                } else {
                    m_contentBounds = LayoutBounds(m_contentBounds.left(), m_contentBounds.top(), m_contentBounds.width(), m_contentBounds.height() + (rowPtr->bounds().height() - oldBounds.height()));
                }
            }
            
            bool cellAt(float x, float y, typename CellRow<CellType>::CellPtr& result) {
                for (unsigned int i = 0; i < m_rows.size(); i++) {
                    CellRowPtr row = m_rows[i];
                    const LayoutBounds& rowBounds = row->bounds();
                    if (y > rowBounds.bottom())
                        continue;
                    else if (y < rowBounds.top())
                        break;
                    typename CellRow<CellType>::CellPtr cell;
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
            typedef std::tr1::shared_ptr<CellGroup<CellType, GroupType> > CellGroupPtr;
        private:
            std::vector<CellGroupPtr> m_groups;
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
                    std::vector<CellGroupPtr> copy = m_groups;
                    m_groups.clear();
                    
                    for (unsigned int i = 0; i < copy.size(); i++) {
                        CellGroupPtr group = copy[i];
                        addGroup(group->item(), group->titleBounds().height());
                        for (unsigned int j = 0; j < group->size(); j++) {
                            const typename CellGroup<CellType, GroupType>::CellRowPtr row = (*group)[j];
                            for (unsigned int k = 0; k < row->size(); k++) {
                                const typename CellRow<CellType>::CellPtr cell = (*row)[k];
                                const LayoutBounds& itemBounds = cell->itemBounds();
                                const LayoutBounds& titleBounds = cell->titleBounds();
                                addItem(cell->item(), itemBounds.width(), itemBounds.height(), titleBounds.width(), titleBounds.height());
                            }
                        }
                    }
                }
            }
        public:
            const CellGroupPtr operator[] (const unsigned int index) {
                assert(index >= 0 && index < m_groups.size());
                if (!m_valid)
                    validate();
                return m_groups[index];
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
                
                CellGroup<CellType, GroupType>* group = new CellGroup<CellType, GroupType>(groupItem, y, m_cellMargin, m_rowMargin, titleHeight, m_width, m_maxCellsPerRow, m_fixedCellWidth);
                CellGroupPtr groupPtr = CellGroupPtr(group);
                m_groups.push_back(groupPtr);
            }
            
            void addItem(const CellType item, float itemWidth, float itemHeight, float titleWidth, float titleHeight) {
                if (!m_valid)
                    validate();
                
                CellGroupPtr groupPtr;
                if (m_groups.empty()) {
                    CellGroup<CellType, GroupType>* group = new CellGroup<CellType, GroupType>(0, m_cellMargin, m_rowMargin, m_width, m_maxCellsPerRow, m_fixedCellWidth);
                    groupPtr = CellGroupPtr(group);
                    m_groups.push_back(groupPtr);
                    m_height += titleHeight;
                    if (titleHeight > 0)
                        m_height += m_rowMargin;
                } else {
                    groupPtr = m_groups.back();
                }
                
                const LayoutBounds oldBounds = groupPtr->bounds();
                groupPtr->addItem(item, itemWidth, itemHeight, titleWidth, titleHeight);
                m_height += (groupPtr->bounds().height() - oldBounds.height());
            }
            
            void clear() {
                m_groups.clear();
                invalidate();
            }
            
            bool cellAt(float x, float y, typename CellRow<CellType>::CellPtr& result) {
                if (!m_valid)
                    validate();
                
                for (unsigned int i = 0; i < m_groups.size(); i++) {
                    CellGroupPtr group = m_groups[i];
                    const LayoutBounds groupBounds = group->bounds();
                    if (y > groupBounds.bottom())
                        continue;
                    else if (y < groupBounds.top())
                        break;
                    typename CellRow<CellType>::CellPtr cell;
                    if (group->cellAt(x, y, cell)) {
                        result = cell;
                        return true;
                    }
                }
                
                return false;
            }
            
            bool groupAt(float x, float y, CellGroupPtr& result) {
                if (!m_valid)
                    validate();
                
                for (unsigned int i = 0; i < m_groups.size(); i++) {
                    CellGroupPtr group = m_groups[i];
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
