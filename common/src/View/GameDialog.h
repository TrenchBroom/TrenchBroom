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

#ifndef TrenchBroom_GameDialog
#define TrenchBroom_GameDialog

#include "Model/MapFormat.h"

#include <string>

#include <QDialog>

class QComboBox;
class QPushButton;
class QWidget;

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    namespace View {
        class GameListBox;

        class GameDialog : public QDialog {
            Q_OBJECT
        protected:
            GameListBox* m_gameListBox;
            QComboBox* m_mapFormatComboBox;
            QPushButton* m_openPreferencesButton;
            QPushButton* m_okButton;
        public:
            ~GameDialog() override;

            // FIXME: return a tuple instead of taking in/out parameters
            static bool showNewDocumentDialog(QWidget* parent, std::string& gameName, Model::MapFormat& mapFormat);
            static bool showOpenDocumentDialog(QWidget* parent, std::string& gameName, Model::MapFormat& mapFormat);

            std::string currentGameName() const;
            Model::MapFormat currentMapFormat() const;
        private slots:
            void currentGameChanged(const QString& gameName);
            void gameSelected(const QString& gameName);
            void openPreferencesClicked();
        protected:
            GameDialog(const QString& title, const QString& infoText, QWidget* parent = nullptr);

            void createGui(const QString& title, const QString& infoText);
            QWidget* createInfoPanel(QWidget* parent, const QString& title, const QString& infoText);
            QWidget* createSelectionPanel(QWidget* parent);
        private:
            void updateMapFormats(const std::string& gameName);

            void bindObservers();
            void unbindObservers();
            void preferenceDidChange(const IO::Path& path);
        };
    }
}

#endif /* defined(TrenchBroom_GameDialog) */
