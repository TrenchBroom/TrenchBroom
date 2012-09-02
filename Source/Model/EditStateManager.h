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
            void setDefaultAndClear(EditState::Type state, EditStateChangeSet& changeSet);
        public:
            EditStateManager();
            
            inline SelectionMode selectionMode() const {
                return current().selectionMode();
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
            mutable EntityStateChange m_entityStateChanges;
            mutable BrushStateChange m_brushStateChanges;
            FaceList m_selectedFaces;
            FaceList m_deselectedFaces;
            bool m_empty;
            bool m_entitySelectionChanged;
            bool m_brushSelectionChanged;
            bool m_faceSelectionChanged;
            
            friend class EditStateManager;
            inline void addEntity(EditState::Type previousState, Entity& entity) {
                m_entityStateChanges[previousState].push_back(&entity);
                m_empty = false;
                m_entitySelectionChanged = previousState == EditState::Selected || entity.selected();
            }
            
            inline void addBrush(EditState::Type previousState, Brush& brush) {
                m_brushStateChanges[previousState].push_back(&brush);
                m_empty = false;
                m_brushSelectionChanged = previousState == EditState::Selected || brush.selected();
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
            m_entitySelectionChanged(false),
            m_brushSelectionChanged(false),
            m_faceSelectionChanged(false) {}
            
            inline const EntityList& entities(EditState::Type previousState) const {
                return m_entityStateChanges[previousState];
            }
            
            inline const BrushList& brushes(EditState::Type previousState) const {
                return m_brushStateChanges[previousState];
            }
            
            inline const FaceList& faces(bool previouslySelected) const {
                if (previouslySelected)
                    return m_deselectedFaces;
                return m_selectedFaces;
            }
            
            inline bool entitySelectionChanged() const {
                return m_entitySelectionChanged;
            }
            
            inline bool brushSelectionChanged() const {
                return m_brushSelectionChanged;
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
