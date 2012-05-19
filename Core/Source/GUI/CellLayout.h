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
            CellType m_item;
        public:
            Cell(CellType item, float x, float y, float itemWidth, float itemHeight, float titleWidth, float titleHeight) : m_item(item), m_x(x), m_y(y), m_itemWidth(itemWidth), m_itemHeight(itemHeight), m_titleWidth(titleWidth), m_titleHeight(titleHeight) {}
            
            float x() const {
                return m_x;
            }
            
            float y() const {
                return m_y;
            }
            
            float width() const {
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
            int m_maxCells;
            float m_y;
            float m_width;
            float m_height;
            float m_cellMargin;
        public:
            const CellPtr operator[] (const int index) const {
                assert(index >= 0 && index < m_cells.size());
                return m_cells[index];
            }

            CellRow(float y, float cellMargin, float rowWidth, int maxCells = -1) : m_y(y), m_cellMargin(cellMargin), m_rowWidth(rowWidth), m_maxCells(maxCells), m_width(0), m_height(0) {}
            
            bool addItem(CellType item, float itemWidth, float itemHeight, float titleWidth, float titleHeight) {
                float cellWidth = Math::fmax(itemWidth, titleWidth);
                float cellHeight = itemHeight + titleHeight;
                if (m_maxCells == -1 && m_width + cellWidth + 2 * m_cellMargin > m_rowWidth && !m_cells.empty())
                    return false;
                if (m_maxCells != -1 && m_cells.size() >= m_maxCells - 1)
                    return false;
                
                float x = m_width;
                m_width += cellWidth;
                if (!m_cells.empty()) {
                    x += m_cellMargin;
                    m_width += m_cellMargin;
                }
                m_height = Math::fmax(m_height, cellHeight);
                
                Cell<CellType>* cell = new Cell<CellType>(item, x, m_y, itemWidth, itemHeight, titleWidth, titleHeight);
                CellPtr cellPtr(cell);
                m_cells.push_back(cellPtr);
                
                return true;
            }

            const std::vector<CellPtr>& cells() {
                return m_cells;
            }
            
            float y() const {
                return m_y;
            }
            
            float height() const {
                return m_height;
            }
            
            int size() const {
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
            int m_maxCellsPerRow;
            float m_cellMargin;
            float m_rowMargin;
            GroupType m_item;
        public:
            const CellRowPtr operator[] (const int index) const {
                assert(index >= 0 && index < m_rows.size());
                return m_rows[index];
            }

            CellGroup(GroupType item, float y, float cellMargin, float rowMargin, float titleHeight, float width, int maxCellsPerRow = -1) : 
                m_item(item), 
                m_y(y), 
                m_cellMargin(cellMargin), 
                m_rowMargin(rowMargin), 
                m_titleHeight(titleHeight), 
                m_width(width), 
                m_height(m_titleHeight), 
                m_maxCellsPerRow(maxCellsPerRow) {}
            
            CellGroup(float y, float cellMargin, float rowMargin, float width, int maxCellsPerRow = -1) : 
                m_y(y), 
                m_cellMargin(cellMargin), 
                m_rowMargin(rowMargin), 
                m_titleHeight(0), 
                m_width(width), 
                m_height(m_titleHeight), 
                m_maxCellsPerRow(maxCellsPerRow) {}
            
            void addItem(CellType item, float itemWidth, float itemHeight, float titleWidth, float titleHeight) {
                CellRowPtr rowPtr;
                if (m_rows.empty()) {
                    float y = m_y;
                    if (m_titleHeight > 0)
                        y += m_titleHeight + m_rowMargin;
                    CellRow<CellType>* row = new CellRow<CellType>(y, m_cellMargin, m_width, m_maxCellsPerRow);
                    rowPtr = CellRowPtr(row);
                    m_rows.push_back(rowPtr);
                } else {
                    rowPtr = m_rows.back();
                }
                
                float oldHeight = rowPtr->height();
                if (!rowPtr->addItem(item, itemWidth, itemHeight, titleWidth, titleHeight)) {
                    float y = rowPtr->y() + rowPtr->height() + m_rowMargin;
                    CellRow<CellType>* row = new CellRow<CellType>(y, m_cellMargin, m_width, m_maxCellsPerRow);
                    rowPtr = CellRowPtr(row);
                    m_rows.push_back(rowPtr);
                    assert(rowPtr->addItem(item, itemWidth, itemHeight, titleWidth, titleHeight));
                    m_height += row->height() + m_rowMargin;
                } else {
                    m_height += (rowPtr->height() - oldHeight);
                }
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
            
            GroupType item() const {
                return m_item;
            }
            
            int size() const {
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
            float m_maxCellsPerRow;
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
                    
                    for (int i = 0; i < copy.size(); i++) {
                        CellGroupPtr group = copy[i];
                        addGroup(group->item(), group->titleHeight());
                        for (int j = 0; j < group->size(); j++) {
                            const typename CellGroup<CellType, GroupType>::CellRowPtr row = (*group)[j];
                            for (int k = 0; k < row->size(); k++) {
                                const typename CellRow<CellType>::CellPtr cell = (*row)[k];
                                addItem(cell->item(), cell->itemWidth(), cell->itemHeight(), cell->titleWidth(), cell->titleHeight());
                            }
                        }
                    }
                }
            }
        public:
            const CellGroupPtr operator[] (const int index) {
                assert(index >= 0 && index < m_groups.size());
                if (m_valid)
                    validate();
                return m_groups[index];
            }

            CellLayout(int maxCellsPerRow = -1) : m_width(1), m_cellMargin(0), m_rowMargin(0), m_groupMargin(0), m_maxCellsPerRow(maxCellsPerRow) {
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
                
                float y = m_groups.empty() ? 0 : m_groups.back()->y() + m_groups.back()->titleHeight() + m_groups.back()->height() + m_groupMargin;
                CellGroup<CellType, GroupType>* group = new CellGroup<CellType, GroupType>(groupItem, y, m_cellMargin, m_rowMargin, titleHeight, m_width, m_maxCellsPerRow);
                CellGroupPtr groupPtr = CellGroupPtr(group);
                m_groups.push_back(groupPtr);
            }
            
            void addItem(const CellType item, float itemWidth, float itemHeight, float titleWidth, float titleHeight) {
                if (!m_valid)
                    validate();
                
                CellGroupPtr groupPtr;
                if (m_groups.empty()) {
                    CellGroup<CellType, GroupType>* group = new CellGroup<CellType, GroupType>(0, m_cellMargin, m_rowMargin, m_width, m_maxCellsPerRow);
                    groupPtr = CellGroupPtr(group);
                    m_groups.push_back(groupPtr);
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
            
            int size() {
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

            float width() {
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
