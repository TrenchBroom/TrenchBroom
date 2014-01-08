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

#ifndef __TrenchBroom__TextureSelectedCommand__
#define __TrenchBroom__TextureSelectedCommand__

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

#define WXDLLIMPEXP_CUSTOM_EVENT

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CUSTOM_EVENT, EVT_TEXTURE_SELECTED_EVENT, 1)
//DECLARE_EVENT_TYPE(EVT_TEXTURE_SELECTED_EVENT, 1)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*textureSelectedEventFunction)(TrenchBroom::View::TextureSelectedCommand&);

#define EVT_TEXTURE_SELECTED_HANDLER(func) \
    (wxObjectEventFunction) \
    (textureSelectedEventFunction) & func

#define EVT_TEXTURE_SELECTED(id,func) \
    DECLARE_EVENT_TABLE_ENTRY(EVT_TEXTURE_SELECTED_EVENT, \
        id, \
        wxID_ANY, \
        (wxObjectEventFunction) \
        (textureSelectedEventFunction) & func, \
        (wxObject*) NULL ),


#endif /* defined(__TrenchBroom__TextureSelectedCommand__) */
