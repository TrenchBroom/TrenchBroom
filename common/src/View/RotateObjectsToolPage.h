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

#include "FloatType.h"

#include <vecmath/forward.h>
#include <vecmath/util.h>

#include <memory>

#include <QWidget>

class QAbstractButton;
class QComboBox;

namespace TrenchBroom {
    namespace View {
        class MapDocument;
        class RotateObjectsTool;
        class Selection;
        class SpinControl;

        class RotateObjectsToolPage : public QWidget {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;
            RotateObjectsTool* m_tool;

            QComboBox* m_recentlyUsedCentersList;
            QAbstractButton* m_resetCenterButton;

            SpinControl* m_angle;
            QComboBox* m_axis;
            QAbstractButton* m_rotateButton;
        public:
            RotateObjectsToolPage(std::weak_ptr<MapDocument> document, RotateObjectsTool* tool, QWidget* parent = nullptr);
            ~RotateObjectsToolPage() override;

            void setAxis(vm::axis::type axis);
            void setRecentlyUsedCenters(const std::vector<vm::vec3>& centers);
            void setCurrentCenter(const vm::vec3& center);
        private:
            void bindObservers();
            void unbindObservers();

            void createGui();
            void updateGui();

            void selectionDidChange(const Selection& selection);

            void centerChanged();
            void resetCenterClicked();
            void angleChanged(double value);
            void rotateClicked();
            vm::vec3 getAxis() const;
        };
    }
}


