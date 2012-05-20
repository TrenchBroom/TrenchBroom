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

#include "SingleTextureControl.h"
#include "Model/Assets/Texture.h"
#include "Gwen/Skin.h"

namespace TrenchBroom {
    namespace Gui {
        SingleTextureControl::SingleTextureControl(Gwen::Controls::Base* parent) : Base(parent), m_texture(NULL) {
        }
        
        SingleTextureControl::~SingleTextureControl() {
        }

        void SingleTextureControl::setTexture(Model::Assets::Texture* texture) {
            m_texture = texture;
        }

        void SingleTextureControl::Render(Gwen::Skin::Base* skin) {
            skin->DrawBox(this);
        }

        void SingleTextureControl::RenderOver(Gwen::Skin::Base* skin) {
            if (m_texture != NULL) {
                skin->GetRender()->Flush();
                
                glMatrixMode(GL_MODELVIEW);
                glPushMatrix();
                
                const Gwen::Padding& padding = GetPadding();
                const Gwen::Point& offset = skin->GetRender()->GetRenderOffset();
                const Gwen::Rect& bounds = GetRenderBounds();
                glTranslatef(offset.x, offset.y, 0);

                glEnable(GL_TEXTURE_2D);
                m_texture->activate();
                glColor4f(1, 1, 1, 1);
                glBegin(GL_QUADS);
                
                glTexCoord2f(0, 0);
                glVertex3f(bounds.x + padding.left, bounds.y + padding.top, 0);
                glTexCoord2f(1, 0);
                glVertex3f(bounds.x + bounds.w - padding.right, bounds.y + padding.top, 0);
                glTexCoord2f(1, 1);
                glVertex3f(bounds.x + bounds.w - padding.right, bounds.y + bounds.h - padding.bottom, 0);
                glTexCoord2f(0, 1);
                glVertex3f(bounds.x + padding.left, bounds.y + bounds.h - padding.bottom, 0);
                glEnd();
                m_texture->deactivate();
                
                glPopMatrix();
            }
        }
    }
}
