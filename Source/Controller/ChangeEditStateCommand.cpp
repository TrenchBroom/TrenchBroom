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

#include "ChangeEditStateCommand.h"

#include "Model/MapDocument.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        ChangeEditStateCommand::ChangeEditStateCommand(Model::MapDocument& document, const wxString& name, Model::EditState::Type previousState) :
        Command(Command::ChangeEditState, document, true,name),
        m_state(previousState),
        m_affectAll(true),
        m_replace(false) {}

        ChangeEditStateCommand::ChangeEditStateCommand(Model::MapDocument& document, const wxString& name, Model::EditState::Type newState, const Model::EntityList& entities, bool replace) :
        Command(Command::ChangeEditState, document, true, name),
        m_state(newState),
        m_entities(entities),
        m_affectAll(false),
        m_replace(replace) {}
        
        ChangeEditStateCommand::ChangeEditStateCommand(Model::MapDocument& document, const wxString& name, Model::EditState::Type newState, const Model::BrushList& brushes, bool replace) :
        Command(Command::ChangeEditState, document, true, name),
        m_state(newState),
        m_brushes(brushes),
        m_affectAll(false),
        m_replace(replace) {}
        
        ChangeEditStateCommand::ChangeEditStateCommand(Model::MapDocument& document, const wxString& name, Model::EditState::Type newState, const Model::EntityList& entities, const Model::BrushList& brushes, bool replace) :
        Command(Command::ChangeEditState, document, true, name),
        m_state(newState),
        m_entities(entities),
        m_brushes(brushes),
        m_affectAll(false),
        m_replace(replace) {}
        
        ChangeEditStateCommand::ChangeEditStateCommand(Model::MapDocument& document, const wxString& name, Model::EditState::Type newState, const Model::FaceList& faces, bool replace) :
        Command(Command::ChangeEditState, document, true, name),
        m_state(newState),
        m_faces(faces),
        m_affectAll(false),
        m_replace(replace) {
            assert(m_state == Model::EditState::Selected || m_state == Model::EditState::Default);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::select(Model::MapDocument& document, Model::Entity& entity) {
            Model::EntityList entities;
            entities.push_back(&entity);
            return select(document, entities);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::select(Model::MapDocument& document, Model::Brush& brush) {
            Model::BrushList brushes;
            brushes.push_back(&brush);
            return select(document, brushes);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::select(Model::MapDocument& document, Model::Face& face) {
            Model::FaceList faces;
            faces.push_back(&face);
            return select(document, faces);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::select(Model::MapDocument& document, const Model::EntityList& entities) {
            return new ChangeEditStateCommand(document, entities.size() == 1 ? "Select entity" : "Select entities", Model::EditState::Selected, entities, false);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::select(Model::MapDocument& document, const Model::BrushList& brushes) {
            return new ChangeEditStateCommand(document, brushes.size() == 1 ? "Select brush" : "Select brushes", Model::EditState::Selected, brushes, false);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::select(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes) {
            if (entities.empty())
                return select(document, brushes);
            if (brushes.empty())
                return select(document, entities);
            return new ChangeEditStateCommand(document, "Select objects", Model::EditState::Selected, entities, false);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::select(Model::MapDocument& document, const Model::FaceList& faces) {
            return new ChangeEditStateCommand(document, faces.size() == 1 ? "Select face" : "Select faces", Model::EditState::Selected, faces, false);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::deselect(Model::MapDocument& document, Model::Entity& entity) {
            Model::EntityList entities;
            entities.push_back(&entity);
            return deselect(document, entities);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::deselect(Model::MapDocument& document, Model::Brush& brush) {
            Model::BrushList brushes;
            brushes.push_back(&brush);
            return deselect(document, brushes);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::deselect(Model::MapDocument& document, Model::Face& face) {
            Model::FaceList faces;
            faces.push_back(&face);
            return deselect(document, faces);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::deselect(Model::MapDocument& document, const Model::EntityList& entities) {
            return new ChangeEditStateCommand(document, entities.size() == 1 ? "Deselect entity" : "Deselect entities", Model::EditState::Default, entities, false);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::deselect(Model::MapDocument& document, const Model::BrushList& brushes) {
            return new ChangeEditStateCommand(document, brushes.size() == 1 ? "Deselect brush" : "Deselect brushes", Model::EditState::Default, brushes, false);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::deselect(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes) {
            if (entities.empty())
                return deselect(document, brushes);
            if (brushes.empty())
                return deselect(document, entities);
            return new ChangeEditStateCommand(document, "Deselect objects", Model::EditState::Default, entities, false);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::deselect(Model::MapDocument& document, const Model::FaceList& faces) {
            return new ChangeEditStateCommand(document, faces.size() == 1 ? "Deselect face" : "Deselect faces", Model::EditState::Default, faces, false);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::replace(Model::MapDocument& document, Model::Entity& entity) {
            Model::EntityList entities;
            entities.push_back(&entity);
            return replace(document, entities);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::replace(Model::MapDocument& document, Model::Brush& brush) {
            Model::BrushList brushes;
            brushes.push_back(&brush);
            return replace(document, brushes);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::replace(Model::MapDocument& document, Model::Face& face) {
            Model::FaceList faces;
            faces.push_back(&face);
            return replace(document, faces);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::replace(Model::MapDocument& document, const Model::EntityList& entities) {
            return new ChangeEditStateCommand(document, entities.size() == 1 ? "Select entity" : "Select entities", Model::EditState::Selected, entities, true);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::replace(Model::MapDocument& document, const Model::BrushList& brushes) {
            return new ChangeEditStateCommand(document, brushes.size() == 1 ? "Select brush" : "Select brushes", Model::EditState::Selected, brushes, true);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::replace(Model::MapDocument& document, const Model::EntityList& entities, const Model::BrushList& brushes) {
            if (entities.empty())
                return replace(document, brushes);
            if (brushes.empty())
                return replace(document, entities);
            return new ChangeEditStateCommand(document, "Select objects", Model::EditState::Selected, entities, brushes, true);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::replace(Model::MapDocument& document, const Model::FaceList& faces) {
            return new ChangeEditStateCommand(document, faces.size() == 1 ? "Select face" : "Select faces", Model::EditState::Selected, faces, true);
        }
        
        ChangeEditStateCommand* ChangeEditStateCommand::deselectAll(Model::MapDocument& document) {
            return new ChangeEditStateCommand(document, "Deselect all", Model::EditState::Selected);
        }

        bool ChangeEditStateCommand::Do() {
            if (m_affectAll) {
                if (m_state == Model::EditState::Selected)
                    m_changeSet = document().EditStateManager().deselectAll();
                else if (m_state == Model::EditState::Hidden)
                    m_changeSet = document().EditStateManager().unhideAll();
                else if (m_state == Model::EditState::Locked)
                    m_changeSet = document().EditStateManager().unlockAll();
            } else {
                if (!m_faces.empty()) {
                    m_changeSet = document().EditStateManager().setSelected(m_faces, m_state == Model::EditState::Selected, m_replace);
                } else {
                    if (!m_entities.empty()) {
                        if (!m_brushes.empty())
                            m_changeSet = document().EditStateManager().setEditState(m_entities, m_brushes, m_state, m_replace);
                        else
                            m_changeSet = document().EditStateManager().setEditState(m_entities, m_state, m_replace);
                    } else if (!m_brushes.empty()) {
                        m_changeSet = document().EditStateManager().setEditState(m_brushes, m_state, m_replace);
                    }
                }
            }
            
            if (!m_changeSet.empty()) {
                document().UpdateAllViews(NULL, this);
            }
            
            return true;
        }
        
        bool ChangeEditStateCommand::Undo() {
            return true;
        }
    }
}