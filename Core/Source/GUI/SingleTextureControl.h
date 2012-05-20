/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef TrenchBroom_SingleTextureControl_h
#define TrenchBroom_SingleTextureControl_h

#include "Gwen/Controls/Base.h"

namespace Gwen {
    namespace Skin {
        class Base;
    }
}

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            class Texture;
        }
    }
    
    namespace Gui {
        class SingleTextureControl : public Gwen::Controls::Base {
        private:
            Model::Assets::Texture* m_texture;
        public:
            SingleTextureControl(Gwen::Controls::Base* parent);
            virtual ~SingleTextureControl();
            void setTexture(Model::Assets::Texture* texture);
            virtual void Render(Gwen::Skin::Base* skin);
            virtual void RenderOver(Gwen::Skin::Base* skin);
        };
    }
}

#endif
