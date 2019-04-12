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

#include "MapViewBar.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"
#include "View/ViewEditor.h"
#include "View/wxUtils.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedLayout>

namespace TrenchBroom {
    namespace View {
        MapViewBar::MapViewBar(QWidget* parent, MapDocumentWPtr document) :
        ContainerBar(parent, Sides::BottomSide),
        m_document(document),
        m_toolBook(nullptr),
        m_viewEditor(nullptr) {
            createGui(document);
        }

        QStackedLayout* MapViewBar::toolBook() {
            return m_toolBook;
        }

        void MapViewBar::OnSearchPatternChanged() {

        }

        void MapViewBar::createGui(MapDocumentWPtr document) {
            m_toolBook = new QStackedLayout();
            m_viewEditor = new ViewPopupEditor(this, document);

            auto* hSizer = new QHBoxLayout();
            hSizer->addSpacing(LayoutConstants::NarrowHMargin);
            hSizer->addLayout(m_toolBook, 1);
            hSizer->addSpacing(LayoutConstants::MediumHMargin);
            hSizer->addWidget(m_viewEditor, 0, Qt::AlignVCenter);
            hSizer->addSpacing(LayoutConstants::NarrowHMargin);

            auto* vSizer = new QVBoxLayout();
            vSizer->addSpacing(LayoutConstants::NarrowVMargin);
            vSizer->addLayout(hSizer, 1);
            vSizer->addSpacing(LayoutConstants::NarrowVMargin);

            setLayout(vSizer);
        }
    }
}
