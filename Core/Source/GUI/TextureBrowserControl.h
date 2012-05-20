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

#ifndef TrenchBroom_TextureBrowserControl_h
#define TrenchBroom_TextureBrowserControl_h

#include "Gwen/Controls/Base.h"
#include "GUI/CellLayout.h"
#include "Model/Assets/Texture.h"

namespace Gwen {
    namespace Controls {
        class ScrollControl;
    }

    namespace Skin {
        class Base;
    }
}

namespace TrenchBroom {
    namespace Controller {
        class Editor;
    }
    
    namespace Model {
        namespace Assets {
            class Texture;
            class TextureCollection;
        }
    }
    
    namespace Gui {
        class TextureBrowserPanel : public Gwen::Controls::Base {
        protected:
            Controller::Editor& m_editor;
            bool m_group;
            bool m_hideUnused;
            Model::Assets::ETextureSortCriterion m_sortCriterion;
            Gwen::Font* m_font;
            CellLayout<Model::Assets::Texture*, Model::Assets::TextureCollection*> m_layout;
            
            void textureManagerChanged(Model::Assets::TextureManager& textureManager);
            void reloadTextures();
            void renderTextureBorder(CellRow<Model::Assets::Texture*>::CellPtr cell);
        public:
            TextureBrowserPanel(Gwen::Controls::Base* parent, Controller::Editor& editor);
            virtual ~TextureBrowserPanel();
            virtual void SetFont(Gwen::Font* font);
            virtual Gwen::Font* GetFont();
            virtual void SetPadding(const Gwen::Padding& padding);
            virtual void OnBoundsChanged(Gwen::Rect oldBounds);
            virtual void RenderOver(Gwen::Skin::Base* skin);
            void setHideUnused(bool hideUnused);
            void setGroup(bool group);
            void setSortCriterion(Model::Assets::ETextureSortCriterion criterion);
            void setFixedCellWidth(float fixedCellWidth);
        };
        
        class TextureBrowserControl : public Gwen::Controls::Base {
        protected:
            Controller::Editor& m_editor;
            TextureBrowserPanel* m_textureBrowserPanel;
            Gwen::Controls::ScrollControl* m_textureBrowserScroller;
        public:
            TextureBrowserControl(Gwen::Controls::Base* parent, Controller::Editor& editor);
            virtual ~TextureBrowserControl() {}
            virtual void Render(Gwen::Skin::Base* skin);
            void setHideUnused(bool hideUnused);
            void setGroup(bool group);
            void setSortCriterion(Model::Assets::ETextureSortCriterion criterion);
            void setFixedCellWidth(float fixedCellWidth);
        };
    }
}

#endif
