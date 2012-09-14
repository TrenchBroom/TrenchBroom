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

#ifndef __TrenchBroom__EditStateManager__
#define __TrenchBroom__EditStateManager__

#include "Model/EditState.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/FaceTypes.h"

#include <cassert>
#include <map>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class EditStateChangeSet;
        
        class EditStateManager {
        public:
            typedef enum {
                None,
                Entities,
                Brushes,
                EntitiesAndBrushes,
                Faces
            } SelectionMode;

            class State {
            public:
                EntityList selectedEntities;
                EntityList hiddenEntities;
                EntityList lockedEntities;
                BrushList selectedBrushes;
                BrushList hiddenBrushes;
                BrushList lockedBrushes;
                FaceList selectedFaces;
                
                inline SelectionMode selectionMode() const {
                    if (!selectedEntities.empty()) {
                        assert(selectedFaces.empty());
                        if (!selectedBrushes.empty())
                            return EntitiesAndBrushes;
                        return Entities;
                    }
                    
                    if (!selectedBrushes.empty()) {
                        assert(selectedFaces.empty());
                        return Brushes;
                    }
                    
                    if (!selectedFaces.empty())
                        return Faces;
                    return None;
                }
                
                inline void clear() {
                    selectedEntities.clear();
                    hiddenEntities.clear();
                    lockedEntities.clear();
                    selectedBrushes.clear();
                    hiddenBrushes.clear();
                    lockedBrushes.clear();
                    selectedFaces.clear();
                }
            };
            
            typedef std::vector<State> StateStack;
            StateStack m_states;
            
            inline State& current() {
                return m_states.back();
            }

            inline const State& current() const {
                return m_states.back();
            }
            
            bool doSetEditState(const EntityList& entities, EditState::Type newState, EditStateChangeSet& changeSet);
            bool doSetEditState(const BrushList& brushes, EditState::Type newState, EditStateChangeSet& changeSet);
            void setDefaultAndClear(EntityList& entities, EditStateChangeSet& changeSet);
            void setDefaultAndClear(BrushList& brushes, EditStateChangeSet& changeSet);
            void deselectAndClear(FaceList& faces, EditStateChangeSet& changeSet);
            void setDefaultAndClear(EditState::Type previousState, EditStateChangeSet& changeSet);
        public:
            EditStateManager();
            
            inline SelectionMode selectionMode() const {
                return current().selectionMode();
            }
            
            inline bool hasHiddenObjects() const {
                return !hiddenEntities().empty() || !hiddenBrushes().empty();
            }
            
            inline bool hasLockedObjects() const {
                return !lockedEntities().empty() || !lockedBrushes().empty();
            }

            inline const EntityList& selectedEntities() const {
                return current().selectedEntities;
            }
            
            inline const EntityList& hiddenEntities() const {
                return current().hiddenEntities;
            }
            
            inline const EntityList& lockedEntities() const {
                return current().lockedEntities;
            }
            
            inline const BrushList& selectedBrushes() const {
                return current().selectedBrushes;
            }
            
            inline const BrushList& hiddenBrushes() const {
                return current().hiddenBrushes;
            }
            
            inline const BrushList& lockedBrushes() const {
                return current().lockedBrushes;
            }
            
            inline const FaceList& selectedFaces() const {
                return current().selectedFaces;
            }
            
            EditStateChangeSet setEditState(const EntityList& entities, EditState::Type newState, bool replace = false);
            EditStateChangeSet setEditState(const BrushList& brushes, EditState::Type newState, bool replace = false);
            EditStateChangeSet setEditState(const EntityList& entities, const BrushList& brushes, EditState::Type newState, bool replace = false);
            EditStateChangeSet setSelected(const FaceList& faces, bool select, bool replace = false);
            
            EditStateChangeSet deselectAll();
            EditStateChangeSet unhideAll();
            EditStateChangeSet unlockAll();
            
            inline void clear() {
                current().clear();
            }
        };
        
        class EditStateChangeSet {
        public:
            typedef std::map<EditState::Type, EntityList> EntityStateChange;
            typedef std::map<EditState::Type, BrushList> BrushStateChange;
        private:
            mutable EntityStateChange m_entityStateChangesFrom;
            mutable EntityStateChange m_entityStateChangesTo;
            mutable BrushStateChange m_brushStateChangesFrom;
            mutable BrushStateChange m_brushStateChangesTo;
            FaceList m_selectedFaces;
            FaceList m_deselectedFaces;
            bool m_empty;
            
            bool m_entityStateTransitions[EditState::Count][EditState::Count];
            bool m_brushStateTransitions[EditState::Count][EditState::Count];
            bool m_faceSelectionChanged;
            
            friend class EditStateManager;
            inline void addEntity(EditState::Type previousState, Entity& entity) {
                m_entityStateChangesFrom[previousState].push_back(&entity);
                m_entityStateChangesTo[entity.editState()].push_back(&entity);
                m_entityStateTransitions[previousState][entity.editState()] = true;
                m_empty = false;
            }
            
            inline void addBrush(EditState::Type previousState, Brush& brush) {
                m_brushStateChangesFrom[previousState].push_back(&brush);
                m_brushStateChangesTo[brush.editState()].push_back(&brush);
                m_brushStateTransitions[previousState][brush.editState()] = true;
                m_empty = false;
            }
            
            inline void addFace(bool previouslySelected, Face& face) {
                if (previouslySelected)
                    m_deselectedFaces.push_back(&face);
                else
                    m_selectedFaces.push_back(&face);
                m_empty = false;
                m_faceSelectionChanged = true;
            }
        public:
            EditStateChangeSet() :
            m_empty(true),
            m_faceSelectionChanged(false) {
                for (unsigned int i = 0; i < EditState::Count; i++) {
                    for (unsigned int j = 0; j < EditState::Count; j++) {
                        m_entityStateTransitions[i][j] = false;
                        m_brushStateTransitions[i][j] = false;
                    }
                }
            }
            
            inline const EntityList& entitiesFrom(EditState::Type previousState) const {
                return m_entityStateChangesFrom[previousState];
            }
            
            inline const EntityList& entitiesTo(EditState::Type newState) const {
                return m_entityStateChangesTo[newState];
            }
            
            inline const BrushList& brushesFrom(EditState::Type previousState) const {
                return m_brushStateChangesFrom[previousState];
            }
            
            inline const BrushList& brushesTo(EditState::Type newState) const {
                return m_brushStateChangesTo[newState];
            }
            
            inline const FaceList& faces(bool previouslySelected) const {
                if (previouslySelected)
                    return m_deselectedFaces;
                return m_selectedFaces;
            }
            
            inline bool entityStateChanged(EditState::Type previousState, EditState::Type newState) const {
                return m_entityStateTransitions[previousState][newState];
            }
            
            inline bool entityStateChangedFrom(EditState::Type previousState) const {
                for (unsigned int i = 0; i < EditState::Count; i++)
                    if (m_entityStateTransitions[previousState][i])
                        return true;
                return false;
            }
            
            inline bool entityStateChangedTo(EditState::Type newState) const {
                for (unsigned int i = 0; i < EditState::Count; i++)
                    if (m_entityStateTransitions[i][newState])
                        return true;
                return false;
            }

            inline bool brushStateChanged(EditState::Type previousState, EditState::Type newState) const {
                return m_brushStateTransitions[previousState][newState];
            }
            
            inline bool brushStateChangedFrom(EditState::Type previousState) const {
                for (unsigned int i = 0; i < EditState::Count; i++)
                    if (m_brushStateTransitions[previousState][i])
                        return true;
                return false;
            }
            
            inline bool brushStateChangedTo(EditState::Type newState) const {
                for (unsigned int i = 0; i < EditState::Count; i++)
                    if (m_brushStateTransitions[i][newState])
                        return true;
                return false;
            }

            inline bool faceSelectionChanged() const {
                return m_faceSelectionChanged;
            }
            
            inline bool empty() const {
                return m_empty;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__EditStateManager__) */
