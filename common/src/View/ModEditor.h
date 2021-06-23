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

#include "NotifierConnection.h"

#include <memory>
#include <string>
#include <vector>

#include <QWidget>

class QLineEdit;
class QListWidget;
class QWidget;
class QAbstractButton;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    namespace View {
        class MapDocument;

        class ModEditor : public QWidget {
            Q_OBJECT
        private:
            std::weak_ptr<MapDocument> m_document;

            QListWidget* m_availableModList;
            QListWidget* m_enabledModList;
            QLineEdit* m_filterBox;
            QAbstractButton* m_addModsButton;
            QAbstractButton* m_removeModsButton;
            QAbstractButton* m_moveModUpButton;
            QAbstractButton* m_moveModDownButton;

            std::vector<std::string> m_availableMods;

            NotifierConnection m_notifierConnection;
        public:
            explicit ModEditor(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
        private:
            void createGui();
        private slots:
            void updateButtons();
        private:
            void connectObservers();

            void documentWasNewed(MapDocument* document);
            void documentWasLoaded(MapDocument* document);
            void modsDidChange();
            void preferenceDidChange(const IO::Path& path);

            void updateAvailableMods();
            void updateMods();

            void addModClicked();
            void removeModClicked();
            void moveModUpClicked();
            void moveModDownClicked();
            bool canEnableAddButton() const;
            bool canEnableRemoveButton() const;
            bool canEnableMoveUpButton() const;
            bool canEnableMoveDownButton() const;
            void filterBoxChanged();
        };
    }
}

