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

#ifndef TrenchBroom_FlagChangedCommand
#define TrenchBroom_FlagChangedCommand

#include <wx/event.h>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace View {
        class FlagChangedCommand : public wxNotifyEvent {
        protected:
            size_t m_index;
            
            int m_flagSetValue;
            int m_flagMixedValue;
        public:
            FlagChangedCommand();

            void setValues(size_t index, int flagSetValue, int flagMixedValue = 0);
            
            int flagSetValue() const;
            int flagMixedValue() const;
            
            size_t index() const;
            bool flagSet() const;
            
            virtual wxEvent* Clone() const;
            
            DECLARE_DYNAMIC_CLASS(FlagChangedCommand)
        };
    }
}

typedef void (wxEvtHandler::*FlagChangedCommandFunction)(TrenchBroom::View::FlagChangedCommand &);

wxDECLARE_EVENT(FLAG_CHANGED_EVENT, TrenchBroom::View::FlagChangedCommand);
#define FlagChangedHandler(func) wxEVENT_HANDLER_CAST(FlagChangedCommandFunction, func)

#endif /* defined(TrenchBroom_FlagChangedCommand) */
