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

#ifndef TrenchBroom_GameListBox
#define TrenchBroom_GameListBox

#include "StringType.h"
#include "View/ImageListBox.h"

class QPixmap;

#include <vector>

namespace TrenchBroom {
    namespace View {
        class GameListBox : public ImageListBox {
            Q_OBJECT
        private:
            struct Info {
                String name;
                QPixmap image;
                QString title;
                QString subtitle;
            };

            using InfoList = std::vector<Info>;

            InfoList m_gameInfos;
        public:
            explicit GameListBox(QWidget* parent = nullptr);
            String selectedGameName() const;
            void selectGame(size_t index);
            void reloadGameInfos();
            void updateGameInfos();
        private:
            Info makeGameInfo(const String& gameName) const;
        private:
            size_t itemCount() const override;
            QPixmap image(size_t index) const override;
            QString title(size_t index) const override;
            QString subtitle(size_t index) const override;

            void selectedRowChanged(int index) override;
            void doubleClicked(size_t index) override;
        signals:
            void currentGameChanged(const QString& gameName);
            void selectCurrentGame(const QString& gameName);
        };
    }
}


#endif /* defined(TrenchBroom_GameListBox) */
