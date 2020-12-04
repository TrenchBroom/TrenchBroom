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

#include "Model/GameEngineConfig.h"
#include "View/CompilationVariables.h"

#include <memory>

#include <QDialog>

class QPushButton;

namespace TrenchBroom {
    namespace Model {
        class GameEngineProfile;
    }

    namespace View {
        class GameEngineProfileListBox;
        class MultiCompletionLineEdit;

        /**
         * Dialog for launching engine (Run -> Launch Engine); only lets you edit the
         * parameters of a GameEngineProfile, not edit the profile list/name/path.
         *
         * A "Configure Engines..." button opens GameEngineDialog for editing the
         * name/path of engines.
         */
        class LaunchGameEngineDialog : public QDialog {
        private:
            std::weak_ptr<MapDocument> m_document;
            GameEngineProfileListBox* m_gameEngineList;
            MultiCompletionLineEdit* m_parameterText;
            QPushButton* m_launchButton;
            Model::GameEngineProfile* m_lastProfile;
            Model::GameEngineConfig m_config;
        public:
            explicit LaunchGameEngineDialog(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
        private:
            void createGui();
            void reloadConfig();
            LaunchGameEngineVariables variables() const;
        private slots:
            void gameEngineProfileChanged();
            void parametersChanged(const QString& text);
            void editGameEngines();
            void launchEngine();
        public slots: // QDialog overrides
            void done(int r) override;
        private:
            void saveConfig();
        };
    }
}


