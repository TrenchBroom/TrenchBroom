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

#include "ReparentBrushesCommand.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Utility/String.h"

namespace TrenchBroom {
    namespace Controller {
        bool ReparentBrushesCommand::performDo() {
            m_oldParents.clear();

            Model::EntityList entities;
            if (!m_newParent.worldspawn())
                entities.push_back(&m_newParent);
            
            Model::BrushList::const_iterator it, end;
            for (it = m_brushes.begin(), end = m_brushes.end(); it != end; ++it) {
                Model::Brush* brush = *it;
                Model::Entity* oldParent = brush->entity();
                if (oldParent != NULL) {
                    m_oldParents[brush] = oldParent;
                    if (!oldParent->worldspawn() &&
                        std::find(entities.begin(), entities.end(), oldParent) == entities.end())
                        entities.push_back(oldParent);
                }
            }
            
            document().entitiesWillChange(entities);
            for (it = m_brushes.begin(), end = m_brushes.end(); it != end; ++it) {
                Model::Brush& brush = **it;
                Model::Entity& oldParent = *brush.entity();
                oldParent.removeBrush(brush);
                m_newParent.addBrush(brush);
            }
            document().entitiesDidChange(entities);
            
            return true;
        }
        
        bool ReparentBrushesCommand::performUndo() {
            Model::EntityList entities;
            if (!m_newParent.worldspawn())
                entities.push_back(&m_newParent);
            
            Model::BrushList::const_iterator it, end;
            for (it = m_brushes.begin(), end = m_brushes.end(); it != end; ++it) {
                Model::Brush* brush = *it;
                Model::Entity* oldParent = m_oldParents[brush];
                if (oldParent != NULL &&
                    !oldParent->worldspawn() &&
                    std::find(entities.begin(), entities.end(), oldParent) == entities.end())
                    entities.push_back(oldParent);
            }
            
            document().entitiesWillChange(entities);
            for (it = m_brushes.begin(), end = m_brushes.end(); it != end; ++it) {
                Model::Brush& brush = **it;
                Model::Entity* oldParent = m_oldParents[&brush];
                m_newParent.removeBrush(brush);
                if (oldParent != NULL)
                    oldParent->addBrush(brush);
            }
            document().entitiesDidChange(entities);
            
            return true;
        }
        
        ReparentBrushesCommand::ReparentBrushesCommand(Model::MapDocument& document, const wxString& name, const Model::BrushList& brushes, Model::Entity& newParent) :
        DocumentCommand(ReparentBrushes, document, true, name, true),
        m_brushes(brushes),
        m_newParent(newParent) {}

        ReparentBrushesCommand* ReparentBrushesCommand::reparent(Model::MapDocument& document, const Model::BrushList& brushes, Model::Entity& newParent) {
            StringStream name;
            name << (brushes.size() == 1 ? "Move Brush to " : "Move Brushes to ");
            name << newParent.classname();
            return new ReparentBrushesCommand(document, name.str(), brushes, newParent);
        }

        const Model::EntityList ReparentBrushesCommand::emptyParents() const {
            Model::EntitySet result;
            Model::BrushParentMap::const_iterator it, end;
            for (it = m_oldParents.begin(), end = m_oldParents.end(); it != end; ++it) {
                Model::Entity& entity = *it->second;
                if (!entity.worldspawn() && entity.brushes().empty())
                    result.insert(&entity);
            }
            return Model::makeList(result);
        }

    }
}
