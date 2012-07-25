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
#include "GUI/CellLayoutControl.h"
#include "GUI/CellLayout.h"
#include "Model/Map/EntityDefinition.h"
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
    
    namespace Renderer {
        class Vbo;
        class VboBlock;
    }
    
    namespace Gui {
        typedef std::tr1::shared_ptr<Gwen::Font> FontPtr;
        typedef std::pair<Model::EntityDefinitionPtr, FontPtr> EntityCellData;
        typedef std::string EntityGroupData;

        class EntityDragControl : public CellDragControl<EntityCellData> {
        protected:
            Controller::Editor& m_editor;
        public:
            EntityDragControl(Gwen::Controls::Base* parent, CellRow<EntityCellData>::CellPtr cell, Controller::Editor& editor) : CellDragControl(parent, cell), m_editor(editor) {}
            virtual void RenderOverlay(Gwen::Skin::Base* skin);
        };

        class EntityBrowserPanel : public CellLayoutControl<EntityCellData, EntityGroupData> {
        protected:
            Controller::Editor& m_editor;
            
            Renderer::Vbo* m_boundsVbo;
            Renderer::VboBlock* m_boundsBlock;
            
            bool m_group;
            std::string m_filterText;
            
            virtual void addEntityDefinitionToLayout(Model::EntityDefinitionPtr definition);
            virtual void doReloadLayout();
            virtual void SetDragAndDropPackage(CellRow<EntityCellData>::CellPtr cell);
            virtual CellDragControl<EntityCellData>* createDragControl(CellRow<EntityCellData>::CellPtr cell);
        public:
            EntityBrowserPanel(Gwen::Controls::Base* parent, Controller::Editor& editor);
            virtual ~EntityBrowserPanel();
            
            virtual void setGroup(bool group);
            virtual void setFilterText(const std::string filterText);
            
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
            
            virtual void setGroup(bool group);
            virtual void setFilterText(const std::string filterText);
            
            virtual void Render(Gwen::Skin::Base* skin);
        };
    }
}

#endif
