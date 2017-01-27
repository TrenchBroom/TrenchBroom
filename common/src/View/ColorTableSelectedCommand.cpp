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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ColorTableSelectedCommand.h"

wxDEFINE_EVENT(COLOR_TABLE_SELECTED_EVENT, TrenchBroom::View::ColorTableSelectedCommand);

namespace TrenchBroom {
    namespace View {
        wxIMPLEMENT_DYNAMIC_CLASS(ColorTableSelectedCommand, wxNotifyEvent)
        ColorTableSelectedCommand::ColorTableSelectedCommand() :
        wxNotifyEvent(COLOR_TABLE_SELECTED_EVENT, wxID_ANY) {}
        
        const wxColor& ColorTableSelectedCommand::color() const {
            return m_color;
        }
        
        void ColorTableSelectedCommand::setColor(const wxColor& color) {
            m_color = color;
        }

        wxEvent* ColorTableSelectedCommand::Clone() const {
            return new ColorTableSelectedCommand(*this);
        }
        
    }
}
