/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_ColorTableSelectedCommand
#define TrenchBroom_ColorTableSelectedCommand

#include <wx/colour.h>
#include <wx/event.h>

namespace TrenchBroom {
    namespace View {
        class ColorTableSelectedCommand : public wxNotifyEvent {
        protected:
            wxColor m_color;
        public:
            ColorTableSelectedCommand();
            
            const wxColor& color() const;
            void setColor(const wxColor& color);
            
            virtual wxEvent* Clone() const;
            
            DECLARE_DYNAMIC_CLASS(ColorTableSelectedCommand)
        };
    }
}

typedef void (wxEvtHandler::*ColorTableSelectedCommandFunction)(TrenchBroom::View::ColorTableSelectedCommand &);

wxDECLARE_EVENT(COLOR_TABLE_SELECTED_EVENT, TrenchBroom::View::ColorTableSelectedCommand);
#define ColorTableSelectedHandler(func) wxEVENT_HANDLER_CAST(ColorTableSelectedCommandFunction, func)

#endif /* defined(TrenchBroom_ColorTableSelectedCommand) */
