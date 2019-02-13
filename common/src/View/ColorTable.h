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

#ifndef TrenchBroom_ColorTable
#define TrenchBroom_ColorTable

#include <QWidget>
#include <QColor>

#include <vector>

namespace TrenchBroom {
    namespace View {
        class ColorTable : public QWidget {
            Q_OBJECT
        public:
            using ColorList = std::vector<QColor>;
        private:
            int m_cellSize;
            int m_margin;
            ColorList m_colors;
            ColorList m_selectedColors;
        public:
            ColorTable(QWidget* parent, int cellSize);
            
            void setColors(const ColorList& colors);
            void setSelection(const ColorList& colors);

        signals:
            void colorTableSelected(QColor color);

        protected: // QWidget overrides
            void paintEvent(QPaintEvent* event) override;
            void mouseReleaseEvent(QMouseEvent* event) override;

        public: // QWidget overrides
            bool hasHeightForWidth() const override;
            int heightForWidth(int w) const override;

        private:
            int computeCols(int width) const;
            int computeRows(int cols) const;
            int computeHeight(int rows) const;
        };
    }
}

#endif /* defined(TrenchBroom_ColorTable) */
