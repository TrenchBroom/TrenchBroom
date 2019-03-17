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

#include "TextureSelectedCommand.h"

wxDEFINE_EVENT(TEXTURE_SELECTED_EVENT, TrenchBroom::View::TextureSelectedCommand);

namespace TrenchBroom {
    namespace View {
        wxIMPLEMENT_DYNAMIC_CLASS(TextureSelectedCommand, wxNotifyEvent)
        TextureSelectedCommand::TextureSelectedCommand() :
        wxNotifyEvent(TEXTURE_SELECTED_EVENT, wxID_ANY),
        m_texture(nullptr) {}

        Assets::Texture* TextureSelectedCommand::texture() const {
            return m_texture;
        }

        void TextureSelectedCommand::setTexture(Assets::Texture* texture) {
            m_texture = texture;
        }

        wxEvent* TextureSelectedCommand::Clone() const {
            return new TextureSelectedCommand(*this);
        }
    }
}
