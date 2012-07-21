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

#ifndef TrenchBroom_EntityPropertyTableControl_h
#define TrenchBroom_EntityPropertyTableControl_h

#include "Gwen/Controls/Base.h"
#include <vector>

namespace Gwen {
    namespace Skin {
        class Base;
    }
    
    namespace Controls {
        class Properties;
        class PropertyRow;
        class ScrollControl;
    }
}

namespace TrenchBroom {
    namespace Controller {
        class Editor;
    }
    
    namespace Model {
        class Entity;
    }
    
    namespace Gui {
        class EntityPropertyTableControl : public Gwen::Controls::Base {
        protected:
            Controller::Editor& m_editor;
            Gwen::Controls::Properties* m_properties;
            std::vector<Gwen::Controls::PropertyRow*> m_propertyRows;
            Gwen::Controls::ScrollControl* m_scroller;
            std::vector<Model::Entity*> m_entities;
            
            virtual void updateProperties();
            void propertyKeyChanged(Gwen::Controls::Base* control);
            void propertyValueChanged(Gwen::Controls::Base* control);
            void propertyRowAdded(Gwen::Controls::Base* control);
        public:
            EntityPropertyTableControl(Gwen::Controls::Base* parent, Controller::Editor& editor);
            virtual ~EntityPropertyTableControl();
            virtual void Render(Gwen::Skin::Base* skin);
            
            void setEntities(const std::vector<Model::Entity*>& entities);
        };
    }
}

#endif
