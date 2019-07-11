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

#include "InfoPanel.h"

#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "View/Console.h"
#include "View/ContainerBar.h"
#include "View/IssueBrowser.h"
#include "View/TabBar.h"
#include "View/TabBook.h"

#include <QVBoxLayout>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        InfoPanel::InfoPanel(MapDocumentWPtr document, QWidget* parent) :
        QWidget(parent),
        m_tabBook(nullptr),
        m_console(nullptr),
        m_issueBrowser(nullptr) {
            m_tabBook = new TabBook(this);

            m_console = new Console();
            m_issueBrowser = new IssueBrowser(document);

            m_tabBook->addPage(m_console, tr("Console"));
            m_tabBook->addPage(m_issueBrowser, tr("Issues"));

            auto* sizer = new QVBoxLayout();
            sizer->setContentsMargins(0, 0, 0, 0);
            sizer->addWidget(m_tabBook);
            setLayout(sizer);
        }

        Console* InfoPanel::console() const {
            return m_console;
        }
    }
}
