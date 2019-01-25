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

#include "TitleBar.h"

#include <wx/settings.h>
#include <wx/sizer.h>
#include <QLabel>

#include "View/ViewConstants.h"

namespace TrenchBroom {
    namespace View {
        TitleBar::TitleBar(QWidget* parent, const QString& title, const int hMargin, const int vMargin, const bool boldTitle) :
        QWidget(parent, wxID_ANY),
        m_titleText(new QLabel(this, wxID_ANY, title)) {
            SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_FRAMEBK));
            
            if (boldTitle)
                m_titleText->SetFont(m_titleText->GetFont().Bold());
            
            auto* sizer = new QHBoxLayout();
            sizer->addSpacing(hMargin);
            sizer->addWidget(m_titleText, 0, wxTOP | wxBOTTOM, vMargin);
            sizer->AddStretchSpacer();
            sizer->addSpacing(hMargin);

            SetSizer(sizer);
        }

        bool TitleBar::AcceptsFocus() const {
            return false;
        }
    }
}
