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
        template <typename CellType>
        class Cell {
        private:
            float m_x;
            float m_y;
            float m_itemWidth;
            float m_itemHeight;
            float m_titleWidth;
            float m_titleHeight;
            float m_fixedCellWidth;
            CellType m_item;
        public:
            Cell(CellType item, float x, float y, float itemWidth, float itemHeight, float titleWidth, float titleHeight, float fixedCellWidth) : m_item(item), m_x(x), m_y(y), m_itemWidth(itemWidth), m_itemHeight(itemHeight), m_titleWidth(titleWidth), m_titleHeight(titleHeight), m_fixedCellWidth(fixedCellWidth) {
                if (m_fixedCellWidth > 0) {
                    if (m_itemWidth >= m_fixedCellWidth) {
                        m_itemHeight *= m_fixedCellWidth / m_itemWidth;
                        m_itemWidth = m_fixedCellWidth;
                    }
                }
            }
            
            bool hitTest(float x, float y) {
                return (x >= itemX()  && x <= itemX()  + itemWidth()  && y >= itemY()  && y <= itemY()  + itemHeight()) ||
                       (x >= titleX() && x <= titleX() + titleWidth() && y >= titleY() && y <= titleY() + titleHeight());
                    
            }
            
            float x() const {
                return m_x;
            }
            
            float y() const {
                return m_y;
            }
            
            float width() const {
                if (m_fixedCellWidth > 0)
                    return m_fixedCellWidth;
                return Math::fmax(m_itemWidth, m_titleWidth);
            }
            
            float height() const {
                return m_itemHeight + m_titleHeight;
            }
            
            float itemX() const {
                return m_x + (width() - m_itemWidth) / 2.0f;
            }
            
            float itemY() const {
                return m_y;
            }
            
            float itemWidth() const {
                return m_itemWidth;
            }
            
            float itemHeight() const {
                return m_itemHeight;
            }
            
            float titleX() const {
                return m_x + (width() - m_titleWidth) / 2.0f;
            }
            
            float titleY() const {
                return m_y + m_itemHeight;
            }
            
            float titleWidth() const {
                return m_titleWidth;
            }
            
            float titleHeight() const {
                return m_titleHeight;
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
            float m_rowWidth;
            unsigned int m_maxCells;
            float m_fixedCellWidth;
            float m_y;
            float m_width;
            float m_height;
            float m_cellMargin;
        public:
            const CellPtr operator[] (const unsigned int index) const {
                assert(index >= 0 && index < m_cells.size());
                return m_cells[index];
            }

            CellRow(float y, float cellMargin, float rowWidth, unsigned int maxCells, float fixedCellWidth) : m_y(y), m_cellMargin(cellMargin), m_rowWidth(rowWidth), m_maxCells(maxCells), m_fixedCellWidth(fixedCellWidth), m_width(0), m_height(0) {}
            
            bool addItem(CellType item, float itemWidth, float itemHeight, float titleWidth, float titleHeight) {
                float x = m_width;
                if (!m_cells.empty())
                    x += m_cellMargin;
                Cell<CellType>* cell = new Cell<CellType>(item, x, m_y, itemWidth, itemHeight, titleWidth, titleHeight, m_fixedCellWidth);
                CellPtr cellPtr(cell);

                if (m_maxCells == 0 && m_width + cellPtr->width() + 2 * m_cellMargin > m_rowWidth && !m_cells.empty())
                    return false;
                if (m_maxCells > 0 && m_cells.size() >= m_maxCells - 1)
                    return false;
                
                m_width += cellPtr->width();
                if (!m_cells.empty())
                    m_width += m_cellMargin;
                m_height = Math::fmax(m_height, cellPtr->height());
                
                m_cells.push_back(cellPtr);
                return true;
            }

            const std::vector<CellPtr>& cells() {
                return m_cells;
            }
            
            bool cellAt(float x, float y, CellPtr& result) {
                for (unsigned int i = 0; i < m_cells.size(); i++) {
                    CellPtr cell = m_cells[i];
                    if (x > cell->x() + cell->width())
                        continue;
                    else if (x < cell->x())
                        break;
                    if (cell->hitTest(x, y)) {
                        result = cell;
                        return true;
                    }
                }
                return false;
            }

            float y() const {
                return m_y;
            }
            
            float height() const {
                return m_height;
            }
            
            bool intersects(float y, float height) {
                return m_y + m_height >= y && m_y <= y + height;
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
            float m_y;
            float m_titleHeight;
            float m_width;
            float m_height;
            unsigned int m_maxCellsPerRow;
            float m_fixedCellWidth;
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
                m_y(y), 
                m_cellMargin(cellMargin), 
                m_rowMargin(rowMargin), 
                m_titleHeight(titleHeight), 
                m_width(width), 
                m_height(m_titleHeight), 
                m_maxCellsPerRow(maxCellsPerRow),
                m_fixedCellWidth(fixedCellWidth) {}
            
            CellGroup(float y, float cellMargin, float rowMargin, float width, int maxCellsPerRow, float fixedCellWidth) : 
                m_y(y), 
                m_cellMargin(cellMargin), 
                m_rowMargin(rowMargin), 
                m_titleHeight(0), 
                m_width(width), 
                m_height(m_titleHeight), 
                m_maxCellsPerRow(maxCellsPerRow),
                m_fixedCellWidth(fixedCellWidth) {}
            
            void addItem(CellType item, float itemWidth, float itemHeight, float titleWidth, float titleHeight) {
                CellRowPtr rowPtr;
                if (m_rows.empty()) {
                    float y = m_y;
                    if (m_titleHeight > 0)
                        y += m_titleHeight + m_rowMargin;
                    CellRow<CellType>* row = new CellRow<CellType>(y, m_cellMargin, m_width, m_maxCellsPerRow, m_fixedCellWidth);
                    rowPtr = CellRowPtr(row);
                    m_rows.push_back(rowPtr);
                    m_height += m_rowMargin;
                } else {
                    rowPtr = m_rows.back();
                }
                
                float oldHeight = rowPtr->height();
                if (!rowPtr->addItem(item, itemWidth, itemHeight, titleWidth, titleHeight)) {
                    float y = rowPtr->y() + rowPtr->height() + m_rowMargin;
                    CellRow<CellType>* row = new CellRow<CellType>(y, m_cellMargin, m_width, m_maxCellsPerRow, m_fixedCellWidth);
                    rowPtr = CellRowPtr(row);
                    m_rows.push_back(rowPtr);
                    assert(rowPtr->addItem(item, itemWidth, itemHeight, titleWidth, titleHeight));
                    m_height += row->height() + m_rowMargin;
                } else {
                    m_height += (rowPtr->height() - oldHeight);
                }
            }
            
            bool cellAt(float x, float y, typename CellRow<CellType>::CellPtr& result) {
                for (unsigned int i = 0; i < m_rows.size(); i++) {
                    CellRowPtr row = m_rows[i];
                    if (y > row->y() + row->height())
                        continue;
                    else if (y < row->y())
                        break;
                    typename CellRow<CellType>::CellPtr cell;
                    if (row->cellAt(x, y, cell)) {
                        result = cell;
                        return true;
                    }
                }
                
                return false;
            }

            float y() const {
                return m_y;
            }
            
            float titleHeight() const {
                return m_titleHeight;
            }
            
            float height() const {
                return m_height;
            }
            
            bool intersects(float y, float height) {
                return m_y + m_height >= y && m_y <= y + height;
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
                        addGroup(group->item(), group->titleHeight());
                        for (unsigned int j = 0; j < group->size(); j++) {
                            const typename CellGroup<CellType, GroupType>::CellRowPtr row = (*group)[j];
                            for (unsigned int k = 0; k < row->size(); k++) {
                                const typename CellRow<CellType>::CellPtr cell = (*row)[k];
                                addItem(cell->item(), cell->itemWidth(), cell->itemHeight(), cell->titleWidth(), cell->titleHeight());
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
                    y += m_groups.back()->y() + m_groups.back()->height() + m_groupMargin;
                
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
                
                float oldHeight = groupPtr->height();
                groupPtr->addItem(item, itemWidth, itemHeight, titleWidth, titleHeight);
                m_height += (groupPtr->height() - oldHeight);
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
                    if (y > group->y() + group->height())
                        continue;
                    else if (y < group->y())
                        break;
                    typename CellRow<CellType>::CellPtr cell;
                    if (group->cellAt(x, y, cell)) {
                        result = cell;
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
