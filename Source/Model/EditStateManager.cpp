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

#include "EditStateManager.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Face.h"
#include "Utility/List.h"

namespace TrenchBroom {
    namespace Model {
        bool EditStateManager::doSetEditState(const EntityList& entities, EditState::Type newState, EditStateChangeSet& changeSet) {
            bool changed = false;
            
            for (unsigned int i = 0; i < entities.size(); i++) {
                Entity& entity = *entities[i];
                if (entity.editState() != newState) {
                    EditState::Type previousState = entity.setEditState(newState);
                    changeSet.addEntity(previousState, entity);
                    
                    if (previousState == EditState::Selected)
                        Utility::erase(current().selectedEntities, &entity);
                    else if (previousState == EditState::Hidden)
                        Utility::erase(current().hiddenEntities, &entity);
                    else if (previousState == EditState::Locked)
                        Utility::erase(current().lockedEntities, &entity);
                    
                    if (newState == EditState::Selected)
                        current().selectedEntities.push_back(&entity);
                    else if (newState == EditState::Hidden)
                        current().hiddenEntities.push_back(&entity);
                    else if (newState == EditState::Locked)
                        current().lockedEntities.push_back(&entity);
                    changed = true;
                }
            }

            return changed;
        }

        bool EditStateManager::doSetEditState(const BrushList& brushes, EditState::Type newState, EditStateChangeSet& changeSet) {
            bool changed = false;
            
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Brush& brush = *brushes[i];
                if (brush.editState() != newState) {
                    EditState::Type previousState = brush.setEditState(newState);
                    changeSet.addBrush(previousState, brush);
                    
                    if (previousState == EditState::Selected)
                        Utility::erase(current().selectedBrushes, &brush);
                    else if (previousState == EditState::Hidden)
                        Utility::erase(current().hiddenBrushes, &brush);
                    else if (previousState == EditState::Locked)
                        Utility::erase(current().lockedBrushes, &brush);
                    
                    if (newState == EditState::Selected)
                        current().selectedBrushes.push_back(&brush);
                    else if (newState == EditState::Hidden)
                        current().hiddenBrushes.push_back(&brush);
                    else if (newState == EditState::Locked)
                        current().lockedBrushes.push_back(&brush);
                    changed = true;
                }
            }
            
            return changed;
        }
        
        bool EditStateManager::doSetSelected(const FaceList& faces, bool newState, EditStateChangeSet& changeSet) {
            bool changed = false;
            for (unsigned int i = 0; i < faces.size(); i++) {
                Face& face = *faces[i];
                if (face.selected() != newState) {
                    if (newState)
                        current().selectedFaces.push_back(&face);
                    else
                        Utility::erase(current().selectedFaces, &face);
                    face.setSelected(newState);
                    changeSet.addFace(!newState, face);
                    changed = true;
                }
            }
            return changed;
        }

        void EditStateManager::setDefaultAndClear(EntityList& entities, EditStateChangeSet& changeSet) {
            for (unsigned int i = 0; i < entities.size(); i++) {
                Entity& entity = *entities[i];
                EditState::Type previousState = entity.setEditState(EditState::Default);
                changeSet.addEntity(previousState, entity);
            }
            entities.clear();
        }
        
        void EditStateManager::setDefaultAndClear(BrushList& brushes, EditStateChangeSet& changeSet) {
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Brush& brush = *brushes[i];
                EditState::Type previousState = brush.setEditState(EditState::Default);
                changeSet.addBrush(previousState, brush);
            }
            brushes.clear();
        }
        
        void EditStateManager::deselectAndClear(FaceList& faces, EditStateChangeSet& changeSet) {
            for (unsigned int i = 0; i < faces.size(); i++) {
                Face& face = *faces[i];
                face.setSelected(false);
                changeSet.addFace(true, face);
            }
            faces.clear();
        }

        void EditStateManager::setDefaultAndClear(EditState::Type previousState, EditStateChangeSet& changeSet) {
            if (previousState == EditState::Selected) {
                setDefaultAndClear(current().selectedEntities, changeSet);
                setDefaultAndClear(current().selectedBrushes, changeSet);
                deselectAndClear(current().selectedFaces, changeSet);
            } else if (previousState == EditState::Hidden) {
                setDefaultAndClear(current().hiddenEntities, changeSet);
                setDefaultAndClear(current().hiddenBrushes, changeSet);
            } else if (previousState == EditState::Locked) {
                setDefaultAndClear(current().lockedEntities, changeSet);
                setDefaultAndClear(current().lockedBrushes, changeSet);
            }
        }

        EditStateManager::EditStateManager() {
            m_states.push_back(State());
        }
        
        EditStateChangeSet EditStateManager::setEditState(const EntityList& entities, EditState::Type newState, bool replace) {
            EditStateChangeSet changeSet;
            
            if (entities.empty())
                return changeSet;
            
            if (replace)
                setDefaultAndClear(newState, changeSet);
            
            FaceList& selectedFaces = current().selectedFaces;
            if (doSetEditState(entities, newState, changeSet) &&
                newState == EditState::Selected &&
                !selectedFaces.empty()) {
                deselectAndClear(selectedFaces, changeSet);
            }
            
            return changeSet;
        }
        
        EditStateChangeSet EditStateManager::setEditState(const BrushList& brushes, EditState::Type newState, bool replace) {
            EditStateChangeSet changeSet;
            if (brushes.empty())
                return changeSet;
            
            if (replace)
                setDefaultAndClear(newState, changeSet);
            
            FaceList& selectedFaces = current().selectedFaces;
            if (doSetEditState(brushes, newState, changeSet) &&
                newState == EditState::Selected &&
                !selectedFaces.empty()) {
                deselectAndClear(selectedFaces, changeSet);
            }
            
            return changeSet;
        }

        EditStateChangeSet EditStateManager::setEditState(const EntityList& entities, const BrushList& brushes, EditState::Type newState, bool replace) {
            EditStateChangeSet changeSet;
            if (entities.empty() && brushes.empty())
                return changeSet;
            
            if (replace)
                setDefaultAndClear(newState, changeSet);
            
            bool deselectFaces = doSetEditState(entities, newState, changeSet);
            deselectFaces |= doSetEditState(brushes, newState, changeSet);

            FaceList& selectedFaces = current().selectedFaces;
            if (deselectFaces && newState == EditState::Selected && !selectedFaces.empty())
                deselectAndClear(selectedFaces, changeSet);
            
            return changeSet;
        }

        EditStateChangeSet EditStateManager::setSelected(const FaceList& faces, bool select, bool replace) {
            EditStateChangeSet changeSet;

            if (faces.empty())
                return changeSet;

            if (select && replace)
                setDefaultAndClear(EditState::Selected, changeSet);
            
            bool changed = doSetSelected(faces, select, changeSet);
            if (select && changed) {
                EntityList& entities = current().selectedEntities;
                BrushList& brushes = current().selectedBrushes;

                if (!entities.empty())
                    setDefaultAndClear(entities, changeSet);
                if (!brushes.empty())
                    setDefaultAndClear(brushes, changeSet);
            }
            
            return changeSet;
        }

        EditStateChangeSet EditStateManager::deselectAll() {
            EditStateChangeSet changeSet;
            setDefaultAndClear(EditState::Selected, changeSet);
            return changeSet;
        }
        
        EditStateChangeSet EditStateManager::unhideAll() {
            EditStateChangeSet changeSet;
            setDefaultAndClear(EditState::Hidden, changeSet);
            return changeSet;
        }
        
        EditStateChangeSet EditStateManager::unlockAll() {
            EditStateChangeSet changeSet;
            setDefaultAndClear(EditState::Locked, changeSet);
            return changeSet;
        }
 
        EditStateChangeSet EditStateManager::undoChangeSet(const EditStateChangeSet& undoChangeSet) {
            EditStateChangeSet changeSet;

            doSetEditState(undoChangeSet.entitiesFrom(EditState::Default), EditState::Default, changeSet);
            doSetEditState(undoChangeSet.entitiesFrom(EditState::Selected), EditState::Selected, changeSet);
            doSetEditState(undoChangeSet.entitiesFrom(EditState::Hidden), EditState::Hidden, changeSet);
            doSetEditState(undoChangeSet.entitiesFrom(EditState::Locked), EditState::Locked, changeSet);

            doSetEditState(undoChangeSet.brushesFrom(EditState::Default), EditState::Default, changeSet);
            doSetEditState(undoChangeSet.brushesFrom(EditState::Selected), EditState::Selected, changeSet);
            doSetEditState(undoChangeSet.brushesFrom(EditState::Hidden), EditState::Hidden, changeSet);
            doSetEditState(undoChangeSet.brushesFrom(EditState::Locked), EditState::Locked, changeSet);
            
            doSetSelected(undoChangeSet.faces(true), true, changeSet);
            doSetSelected(undoChangeSet.faces(false), false, changeSet);
            
            return changeSet;
        }
    }
}