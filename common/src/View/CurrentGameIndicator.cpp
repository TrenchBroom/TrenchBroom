/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "CurrentGameIndicator.h"

#include "IO/ResourceUtils.h"
#include "Model/GameFactory.h"
#include "View/ViewConstants.h"

#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        CurrentGameIndicator::CurrentGameIndicator(wxWindow* parent, const String& gameName) :
        wxPanel(parent) {
            SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));

            Model::GameFactory& gameFactory = Model::GameFactory::instance();

            const IO::Path gamePath = gameFactory.gamePath(gameName);
            IO::Path iconPath = gameFactory.iconPath(gameName);
            if (iconPath.isEmpty())
                iconPath = IO::Path("DefaultGameIcon.png");
            
            const wxBitmap gameIcon = IO::loadImageResource(iconPath);
            wxStaticBitmap* gameIconImg = new wxStaticBitmap(this, wxID_ANY, gameIcon);
            wxStaticText* gameNameText = new wxStaticText(this, wxID_ANY, gameName);
            gameNameText->SetFont(gameNameText->GetFont().Larger().Larger().Bold());
            
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->AddSpacer(LayoutConstants::WideHMargin);
            sizer->Add(gameIconImg, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::WideVMargin));
            sizer->AddSpacer(LayoutConstants::NarrowHMargin);
            sizer->Add(gameNameText, wxSizerFlags().CenterVertical().Border(wxTOP | wxBOTTOM, LayoutConstants::WideVMargin));
            SetSizer(sizer);
        }
    }
}
