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

#include "TitledPanel.h"

#include "View/BorderLine.h"
#include "View/TitleBar.h"
#include "View/ViewConstants.h"

#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        TitledPanel::TitledPanel(QWidget* parent, const QString& title, const bool showDivider, const bool boldTitle) :
        QWidget(parent),
        m_panel(nullptr) {
            const int hMargin = showDivider ? LayoutConstants::NarrowHMargin : 0;
            const int vMargin = showDivider ? LayoutConstants::NarrowVMargin : 0;

            m_panel = new QWidget(this);

            auto* sizer = new QVBoxLayout();
            sizer->addWidget(new TitleBar(this, title, hMargin, vMargin, boldTitle), wxSizerFlags().Expand());
            if (showDivider)
                sizer->addWidget(new BorderLine(this, BorderLine::Direction_Horizontal), wxSizerFlags().Expand());
            sizer->addWidget(m_panel, wxSizerFlags().Expand().Proportion(1));
            SetSizer(sizer);
        }

        QWidget* TitledPanel::getPanel() const {
            return m_panel;
        }
    }
}
