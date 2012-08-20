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

#include "MapStructureControl.h"

#include "Controller/Editor.h"
#include "Model/Map/Brush.h"
#include "Model/Map/Entity.h"
#include "Model/Map/Map.h"

#include "Gwen/Controls/TreeControl.h"
#include "Gwen/Controls/TreeNode.h"

namespace TrenchBroom {
    namespace Gui {
        MapStructureControl::MapStructureControl(Gwen::Controls::Base* parent, Controller::Editor& editor) : Base(parent), m_editor(editor) {
            m_treeControl = new Gwen::Controls::TreeControl(this);
            m_treeControl->Dock(Gwen::Pos::Fill);
            m_treeControl->SetCacheToTexture();
        }
        
        MapStructureControl::~MapStructureControl() {
        }

        void MapStructureControl::refresh() {
            m_treeControl->Clear();

            Model::Map& map = m_editor.map();

            /*
            const Model::PropertyValue* message = map.worldspawn(true)->propertyForKey("msg");
            std::stringstream mapName;
            mapName << "Map";
            if (message != NULL)
                mapName << " (" << message << ")";
            Gwen::Controls::TreeNode* mapNode = m_treeControl->AddNode(mapName.str());
            */
             
            const Model::EntityList& entities = map.entities();
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity* entity = entities[i];
                const Model::BrushList& brushes = entity->brushes();
                
                std::string entityName = entity->classname() != NULL ? *entity->classname() : "unknown class";
                Gwen::Controls::TreeNode* entityNode = m_treeControl->AddNode(entityName);
                for (unsigned int j = 0; j < brushes.size(); j++) {
                    std::stringstream brushName;
                    brushName << "Brush " << (j + 1);
                    entityNode->AddNode(brushName.str());
                }
            }
            
//            m_treeControl->ExpandAll();
        }
    }
}
