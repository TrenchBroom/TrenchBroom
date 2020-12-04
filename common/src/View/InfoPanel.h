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

#include <memory>

#include <QWidget>

namespace TrenchBroom {
    class Logger;

    namespace View {
        class Console;
        class IssueBrowser;
        class MapDocument;
        class TabBook;

        class InfoPanel : public QWidget {
            Q_OBJECT
        private:
            TabBook* m_tabBook;
            Console* m_console;
            IssueBrowser* m_issueBrowser;
        public:
            explicit InfoPanel(std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
            Console* console() const;
        };
    }
}


