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

#include "RecentDocumentListBox.h"

#include "StringUtils.h"
#include "TrenchBroomApp.h"
#include "IO/Path.h"
#include "IO/ResourceUtils.h"
#include "Model/GameConfig.h"
#include "Model/GameFactory.h"
// FIXME:
// #include "View/RecentDocumentSelectedCommand.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        RecentDocumentListBox::RecentDocumentListBox(QWidget* parent) :
        ImageListBox("No Recent Documents", parent),
        m_documentIcon(IO::loadPixmapResource("DocIcon.png")) {
            assert(!m_documentIcon.isNull());
            TrenchBroomApp& app = View::TrenchBroomApp::instance();
            app.recentDocumentsDidChangeNotifier.addObserver(this, &RecentDocumentListBox::recentDocumentsDidChange);

            refresh();
        }

        RecentDocumentListBox::~RecentDocumentListBox() {
            TrenchBroomApp& app = View::TrenchBroomApp::instance();
            app.recentDocumentsDidChangeNotifier.removeObserver(this, &RecentDocumentListBox::recentDocumentsDidChange);
        }

        void RecentDocumentListBox::recentDocumentsDidChange() {
            refresh();
        }

        size_t RecentDocumentListBox::itemCount() const {
            const TrenchBroomApp& app = View::TrenchBroomApp::instance();
            const IO::Path::List& recentDocuments = app.recentDocuments();
            return recentDocuments.size();
        }

        QPixmap RecentDocumentListBox::image(const size_t /* index */) const {
            return m_documentIcon;
        }

        QString RecentDocumentListBox::title(const size_t index) const {
            const auto& app = View::TrenchBroomApp::instance();
            const IO::Path::List& recentDocuments = app.recentDocuments();
            ensure(index < recentDocuments.size(), "index out of range");
            return QString::fromStdString(recentDocuments[index].lastComponent().asString());
        }

        QString RecentDocumentListBox::subtitle(const size_t index) const {
            const auto& app = View::TrenchBroomApp::instance();
            const IO::Path::List& recentDocuments = app.recentDocuments();
            ensure(index < recentDocuments.size(), "index out of range");
            return QString::fromStdString(recentDocuments[index].asString());
        }

        void RecentDocumentListBox::doubleClicked(size_t index) {
            auto& app = View::TrenchBroomApp::instance();
            const IO::Path::List& recentDocuments = app.recentDocuments();

            if (index < recentDocuments.size()) {
                const IO::Path& documentPath = recentDocuments[index];
                emit loadRecentDocument(documentPath);
            }
        }
    }
}
