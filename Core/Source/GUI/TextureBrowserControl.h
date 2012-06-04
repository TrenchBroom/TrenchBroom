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
#include "Gwen/Events.h"
#include "GUI/CellLayout.h"
#include "Model/Assets/Texture.h"
#include "Utilities/SharedPointer.h"

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
        class SelectionEventData;
    }
    
    namespace Gui {
        typedef std::tr1::shared_ptr<Gwen::Font> FontPtr;
        typedef std::pair<Model::Assets::Texture*, FontPtr> CellData;
        typedef Model::Assets::TextureCollection* GroupData;

        void renderTexture(CellRow<CellData>::CellPtr cell, bool override);
        void renderTextureBorder(CellRow<CellData>::CellPtr cell);

        class TextureDragControl : public Gwen::Controls::Base {
        protected:
            CellRow<CellData>::CellPtr m_cell;
        public:
            TextureDragControl(Gwen::Controls::Base* parent, CellRow<CellData>::CellPtr cell);
            virtual ~TextureDragControl();
            virtual void Render(Gwen::Skin::Base* skin);
        };
        
        class TextureBrowserPanel : public Gwen::Controls::Base {
        protected:
            Controller::Editor& m_editor;
            bool m_group;
            bool m_hideUnused;
            Model::Assets::ETextureSortCriterion m_sortCriterion;
            std::string m_filterText;
            Gwen::Font* m_font;
            CellLayout<CellData, GroupData> m_layout;
            Model::Assets::Texture* m_selectedTexture;
            TextureDragControl* m_dragControl;
            
            void selectionChanged(const Model::SelectionEventData& data);
            void textureManagerDidChange(Model::Assets::TextureManager& textureManager);
            void preferencesDidChange(const std::string& key);
            void addTexture(Model::Assets::Texture* texture);
            void reloadTextures();
            virtual void OnMouseClickLeft(int x, int y, bool down);
            virtual void OnTextureSelected();
        public:
            TextureBrowserPanel(Gwen::Controls::Base* parent, Controller::Editor& editor);
            virtual ~TextureBrowserPanel();
            virtual void SetFont(Gwen::Font* font);
            virtual Gwen::Font* GetFont();
            virtual void SetPadding(const Gwen::Padding& padding);
            virtual void OnBoundsChanged(Gwen::Rect oldBounds);
            virtual void RenderOver(Gwen::Skin::Base* skin);
            virtual void DragAndDrop_StartDragging( Gwen::DragAndDrop::Package* package, int x, int y);
            virtual void DragAndDrop_EndDragging(bool success, int x, int y);
            void setHideUnused(bool hideUnused);
            void setGroup(bool group);
            void setSortCriterion(Model::Assets::ETextureSortCriterion criterion);
            void setFixedCellWidth(float fixedCellWidth);
            void setFilterText(const std::string& filterText);

            Gwen::Event::Caller onTextureSelected;
            Model::Assets::Texture* selectedTexture();
        };
        
        class TextureBrowserControl : public Gwen::Controls::Base {
        protected:
            Controller::Editor& m_editor;
            TextureBrowserPanel* m_browserPanel;
            Gwen::Controls::ScrollControl* m_browserScroller;
            void onTextureSelectedInBrowserPanel(Gwen::Controls::Base* control);
        public:
            TextureBrowserControl(Gwen::Controls::Base* parent, Controller::Editor& editor);
            virtual ~TextureBrowserControl() {}
            virtual void Render(Gwen::Skin::Base* skin);
            void setHideUnused(bool hideUnused);
            void setGroup(bool group);
            void setSortCriterion(Model::Assets::ETextureSortCriterion criterion);
            void setFixedCellWidth(float fixedCellWidth);
            void setFilterText(const std::string& filterText);
            
            Gwen::Event::Caller onTextureSelected;
            Model::Assets::Texture* selectedTexture();
        };
    }
}

#endif
