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

#ifndef LaunchGameEngineDialog_h
#define LaunchGameEngineDialog_h

#include "IO/Path.h"
#include "View/CompilationVariables.h"
#include "View/ViewTypes.h"

#include <QDialog>

class QPushButton;

namespace TrenchBroom {
    namespace Model {
        class GameEngineProfile;
    }

    namespace View {
        class MultiCompletionLineEdit;
        class GameEngineProfileListBox;

        class LaunchGameEngineDialog : public QDialog {
        private:
            MapDocumentWPtr m_document;
            GameEngineProfileListBox* m_gameEngineList;
            MultiCompletionLineEdit* m_parameterText;
            QPushButton* m_launchButton;
            Model::GameEngineProfile* m_lastProfile;
        public:
            explicit LaunchGameEngineDialog(MapDocumentWPtr document, QWidget* parent = nullptr);
        private:
            void createGui();
            LaunchGameEngineVariables variables() const;
        private slots:
            void gameEngineProfileChanged();
            void parametersChanged(const QString& text);
            void editGameEngines();
            void launchEngine();
        };
    }
}

#endif /* LaunchGameEngineDialog_h */
