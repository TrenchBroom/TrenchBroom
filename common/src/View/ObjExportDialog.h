/*
 Copyright (C) 2010-2021 Amara M. Kilic

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

#include <QDialog>

class QPushButton;
class QCheckBox;
class QLineEdit;
class QPushButton;

namespace TrenchBroom {
    namespace View {
        class MapFrame;

        class ObjExportDialog : public QDialog {
            Q_OBJECT
        private:
            MapFrame* m_mapFrame;
            QPushButton* m_exportButton;
            QPushButton* m_closeButton;
            QCheckBox* m_relativeCheckBox;
            QLineEdit* m_pathEdit;
            QPushButton* m_browseButton;
        public:
            explicit ObjExportDialog(MapFrame* mapFrame);
        private:
            void createGui();
        };
    }
}

