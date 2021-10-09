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

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <vector>

#include <QVariant>

namespace TrenchBroom {
    namespace View {
        struct LayoutBounds {
            float x;
            float y;
            float width;
            float height;

            float left() const;
            float top() const;
            float right() const;
            float bottom() const;

            bool containsPoint(float pointX, float pointY) const;
            bool intersectsY(float rangeY, float rangeHeight) const;
        };

        class LayoutCell {
        private:
            QVariant m_item;
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
        public:
            LayoutCell(QVariant item,
                       float x, float y,
                       float itemWidth, float itemHeight,
                       float titleWidth, float titleHeight,
                       float titleMargin,
                       float maxUpScale,
                       float minWidth, float maxWidth,
                       float minHeight, float maxHeight);

            const QVariant& item() const;

            float scale() const;

            const LayoutBounds& bounds() const;
            const LayoutBounds& cellBounds() const;
            const LayoutBounds& titleBounds() const;
            const LayoutBounds& itemBounds() const;

            bool hitTest(float x, float y) const;

            void updateLayout(float maxUpScale,
                              float minWidth, float maxWidth,
                              float minHeight, float maxHeight);
        private:
            void doLayout(float maxUpScale,
                          float minWidth, float maxWidth,
                          float minHeight, float maxHeight);
        };

        class LayoutRow {
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

            std::vector<LayoutCell> m_cells;
        public:
            LayoutRow(float x, float y,
                      float cellMargin,
                      float titleMargin,
                      float maxWidth,
                      size_t maxCells,
                      float maxUpScale,
                      float minCellWidth, float maxCellWidth,
                      float minCellHeight, float maxCellHeight);

            const LayoutBounds& bounds() const;

            const std::vector<LayoutCell>& cells() const;
            const LayoutCell* cellAt(float x, float y) const;

            bool intersectsY(float y, float height) const;

            bool addItem(QVariant item,
                         float itemWidth, float itemHeight,
                         float titleWidth, float titleHeight);
        private:
            void readjustItems();
        };

        class LayoutGroup {
        private:
            using GroupType = std::string;
            using CellType = QVariant;
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

            std::vector<LayoutRow> m_rows;
        public:
            LayoutGroup(const GroupType& item,
                        float x, float y,
                        float cellMargin, float titleMargin, float rowMargin,
                        float titleHeight,
                        float width,
                        size_t maxCellsPerRow,
                        float maxUpScale,
                        float minCellWidth, float maxCellWidth,
                        float minCellHeight, float maxCellHeight);

            LayoutGroup(float x, float y,
                        float cellMargin, float titleMargin, float rowMargin,
                        float width,
                        size_t maxCellsPerRow,
                        float maxUpScale,
                        float minCellWidth, float maxCellWidth,
                        float minCellHeight, float maxCellHeight);

            const GroupType& item() const;

            const LayoutBounds& titleBounds() const;
            LayoutBounds titleBoundsForVisibleRect(float y, float height, float groupMargin) const;
            const LayoutBounds& contentBounds() const;
            LayoutBounds bounds() const;

            const std::vector<LayoutRow>& rows() const;
            size_t indexOfRowAt(float y) const;
            const LayoutCell* cellAt(float x, float y) const;

            bool hitTest(float x, float y) const;
            bool intersectsY(float y, float height) const;

            void addItem(CellType item,
                         float itemWidth, float itemHeight,
                         float titleWidth, float titleHeight);
        };

        class CellLayout {
        private:
            using GroupType = std::string;
            using CellType = QVariant;
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

            std::vector<LayoutGroup> m_groups;
            bool m_valid;
            float m_height;
        public:
            CellLayout(size_t maxCellsPerRow = 0);

            float titleMargin() const;
            void setTitleMargin(float titleMargin);

            float cellMargin() const;
            void setCellMargin(float cellMargin);

            float rowMargin() const;
            void setRowMargin(float rowMargin);

            float groupMargin() const;
            void setGroupMargin(float groupMargin);

            float outerMargin() const;
            void setOuterMargin(float outerMargin);

            float minCellWidth() const;
            float maxCellWidth() const;
            void setCellWidth(float minCellWidth, float maxCellWidth);

            float minCellHeight() const;
            float maxCellHeight() const;
            void setCellHeight(float minCellHeight, float maxCellHeight);

            float maxUpScale() const;
            void setMaxUpScale(float maxUpScale);

            float width() const;
            float height();

            LayoutBounds titleBoundsForVisibleRect(const LayoutGroup& group, float y, float height) const;
            float rowPosition(float y, int offset);

            void invalidate();

            void setWidth(float width);

            const std::vector<LayoutGroup>& groups();
            const LayoutCell* cellAt(float x, float y);

            void addGroup(GroupType groupItem, float titleHeight);
            void addItem(CellType item,
                         float itemWidth, float itemHeight,
                         float titleWidth, float titleHeight);

            void clear();
        private:
            void validate();
        };
    }
}
