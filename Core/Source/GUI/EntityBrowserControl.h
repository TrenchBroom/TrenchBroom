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

#ifndef TrenchBroom_EntityBrowserControl_h
#define TrenchBroom_EntityBrowserControl_h

#include "Gwen/Controls/Base.h"

#include "Model/Map/EntityDefinition.h"
#include "GUI/CellLayout.h"
#include "Utilities/SharedPointer.h"

namespace Gwen {
    namespace Controls {
        class ScrollControl;
    }
}

namespace TrenchBroom {
    namespace Controller {
        class Editor;
    }
    
    namespace Gui {
        class EntityBrowserPanel : public Gwen::Controls::Base {
        protected:
            typedef std::tr1::shared_ptr<Gwen::Font> FontPtr;
            typedef pair<Model::EntityDefinitionPtr, FontPtr> CellData;

            Controller::Editor& m_editor;
            CellLayout<CellData, void*> m_layout;
            Gwen::Font* m_font;
            
            void reloadEntityDefinitions();
        public:
            EntityBrowserPanel(Gwen::Controls::Base* parent, Controller::Editor& editor);
            virtual ~EntityBrowserPanel();

            virtual void SetFont(Gwen::Font* font);
            virtual Gwen::Font* GetFont();
            virtual void SetPadding(const Gwen::Padding& padding);
            virtual void OnBoundsChanged(Gwen::Rect oldBounds);
            virtual void RenderOver(Gwen::Skin::Base* skin);
        };
        
        class EntityBrowserControl : public Gwen::Controls::Base {
        protected:
            Controller::Editor& m_editor;
            EntityBrowserPanel* m_browserPanel;
            Gwen::Controls::ScrollControl* m_browserScroller;
        public:
            EntityBrowserControl(Gwen::Controls::Base* parent, Controller::Editor& editor);
            virtual ~EntityBrowserControl();
            
            virtual void Render(Gwen::Skin::Base* skin);
        };
    }
}

#endif
