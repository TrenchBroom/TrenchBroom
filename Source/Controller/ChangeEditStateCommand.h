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

#ifndef __TrenchBroom__ChangeEditStateCommand__
#define __TrenchBroom__ChangeEditStateCommand__

#include "Controller/Command.h"
#include "Model/BrushTypes.h"
#include "Model/EditStateManager.h"
#include "Model/EntityTypes.h"
#include "Model/FaceTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class EditStateChangeSet;
        class Entity;
        class Face;
        class MapDocument;
    }
    
    namespace Controller {
        class ChangeEditStateCommand : public Command {
        protected:
            Model::EditState::Type m_state;
            bool m_affectAll;
            bool m_replace;
            
            Model::EntityList m_entities;
            Model::BrushList m_brushes;
            Model::FaceList m_faces;
            
            Model::EditStateChangeSet m_changeSet;
            
            ChangeEditStateCommand(Model::MapDocument& document, const wxString& name, Model::EditState::Type previousState);
            ChangeEditStateCommand(Model::MapDocument& document, const wxString& name, Model::EditState::Type newState, const Model::EntityList& entities, bool replace);
            ChangeEditStateCommand(Model::MapDocument& document, const wxString& name, Model::EditState::Type newState, const Model::BrushList& brushes, bool replace);
            ChangeEditStateCommand(Model::MapDocument& document, const wxString& name, Model::EditState::Type newState, const Model::EntityList& entities, const Model::BrushList& brushes, bool replace);
            ChangeEditStateCommand(Model::MapDocument& document, const wxString& name, Model::EditState::Type newState, const Model::FaceList& faces, bool replace);
        public:
            static ChangeEditStateCommand* select(Model::MapDocument& document, Model::Entity& entity);
            static ChangeEditStateCommand* select(Model::MapDocument& document, Model::Brush& brush);
            static ChangeEditStateCommand* select(Model::MapDocument& document, Model::Face& face);
            static ChangeEditStateCommand* select(Model::MapDocument& document, const Model::EntityList& entities);
            static ChangeEditStateCommand* select(Model::MapDocument& document, const Model::BrushList& brushes);
            static ChangeEditStateCommand* select(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes);
            static ChangeEditStateCommand* select(Model::MapDocument& document, const Model::FaceList& faces);
            
            static ChangeEditStateCommand* deselect(Model::MapDocument& document, Model::Entity& entity);
            static ChangeEditStateCommand* deselect(Model::MapDocument& document, Model::Brush& brush);
            static ChangeEditStateCommand* deselect(Model::MapDocument& document, Model::Face& face);
            static ChangeEditStateCommand* deselect(Model::MapDocument& document, const Model::EntityList& entities);
            static ChangeEditStateCommand* deselect(Model::MapDocument& document, const Model::BrushList& brushes);
            static ChangeEditStateCommand* deselect(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes);
            static ChangeEditStateCommand* deselect(Model::MapDocument& document, const Model::FaceList& faces);

            static ChangeEditStateCommand* replace(Model::MapDocument& document, Model::Entity& entity);
            static ChangeEditStateCommand* replace(Model::MapDocument& document, Model::Brush& brush);
            static ChangeEditStateCommand* replace(Model::MapDocument& document, Model::Face& face);
            static ChangeEditStateCommand* replace(Model::MapDocument& document, const Model::EntityList& entities);
            static ChangeEditStateCommand* replace(Model::MapDocument& document, const Model::BrushList& brushes);
            static ChangeEditStateCommand* replace(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes);
            static ChangeEditStateCommand* replace(Model::MapDocument& document, const Model::FaceList& faces);

            static ChangeEditStateCommand* deselectAll(Model::MapDocument& document);
            
            bool Do();
            bool Undo();
            
            inline const Model::EditStateChangeSet& changeSet() const {
                return m_changeSet;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__ChangeEditStateCommand__) */
