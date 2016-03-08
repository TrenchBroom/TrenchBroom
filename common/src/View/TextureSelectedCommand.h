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

#ifndef TrenchBroom_TextureSelectedCommand
#define TrenchBroom_TextureSelectedCommand

#include <wx/event.h>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace View {
        class TextureSelectedCommand : public wxNotifyEvent {
        protected:
            Assets::Texture* m_texture;
        public:
            TextureSelectedCommand();
            
            Assets::Texture* texture() const;
            void setTexture(Assets::Texture* texture);
            
            virtual wxEvent* Clone() const;
            
            DECLARE_DYNAMIC_CLASS(TextureSelectedCommand)
        };
    }
}

typedef void (wxEvtHandler::*TextureSelectedCommandFunction)(TrenchBroom::View::TextureSelectedCommand &);

wxDECLARE_EVENT(TEXTURE_SELECTED_EVENT, TrenchBroom::View::TextureSelectedCommand);
#define TextureSelectedHandler(func) wxEVENT_HANDLER_CAST(TextureSelectedCommandFunction, func)

#endif /* defined(TrenchBroom_TextureSelectedCommand) */
