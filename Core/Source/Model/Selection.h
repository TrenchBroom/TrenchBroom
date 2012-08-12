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

#ifndef TrenchBroom_Selection_h
#define TrenchBroom_Selection_h

#include <vector>
#include "Model/Assets/Texture.h"
#include "Model/Map/Brush.h"
#include "Model/Map/Face.h"
#include "Model/Map/Entity.h"
#include "Utilities/Event.h"
#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Model {
        typedef enum {
            TB_SM_NONE,
            TB_SM_FACES,
            TB_SM_BRUSHES,
            TB_SM_ENTITIES,
            TB_SM_BRUSHES_ENTITIES
        } ESelectionMode;
        
        class Texture;
        class Entity;
        class Brush;
        class Face;
        
        class SelectionEventData {
        public:
            EntityList entities;
            BrushList brushes;
            FaceList faces;
            SelectionEventData() {};
            SelectionEventData(const EntityList& entities) : entities(entities) {}
            SelectionEventData(const EntityList& entities, const BrushList& brushes) : entities(entities), brushes(brushes) {}
            SelectionEventData(const BrushList& brushes) : brushes(brushes) {}
            SelectionEventData(const FaceList& faces) : faces(faces) {}
            SelectionEventData(Entity& entity) { entities.push_back(&entity); }
            SelectionEventData(Brush& brush) { brushes.push_back(&brush); }
            SelectionEventData(Face& face) { faces.push_back(&face); }
        };
        
        class Selection {
        private:
            class State {
            public:
                EntityList hiddenEntities;
                BrushList hiddenBrushes;
                EntityList selectedEntities;
                BrushList selectedBrushes;
                FaceList selectedFaces;
                std::vector<Assets::Texture*> mruTextures;
                ESelectionMode selectionMode;
                
                State() : selectionMode(TB_SM_NONE) {}
                State(const State& state) :
                    selectedEntities(state.selectedEntities),
                    selectedBrushes(state.selectedBrushes),
                    selectedFaces(state.selectedFaces),
                    mruTextures(state.mruTextures),
                    selectionMode(state.selectionMode) {}
                
                void clear() {
                    selectedEntities.clear();
                    selectedBrushes.clear();
                    selectedFaces.clear();
                    mruTextures.clear();
                    selectionMode = TB_SM_NONE;
                }
            };
            
            mutable std::vector<State> m_state;
            
            inline State& current() const {
                return m_state.back();
            }
        protected:
            void doSelectEntities(const EntityList& entities);
            void doSelectBrushes(const BrushList& brushes);
            void doSelectFaces(const FaceList& faces);
            EntityList doDeselectEntities(const EntityList& entities);
            BrushList doDeselectBrushes(const BrushList& brushes);
            FaceList doDeselectFaces(const FaceList& faces);
        public:
            typedef Event<const SelectionEventData&> SelectionEvent;
            SelectionEvent selectionAdded;
            SelectionEvent selectionRemoved;
            
            inline Selection() {
                m_state.resize(1);
            }
            
            void push();
            void pop();
            
            inline ESelectionMode selectionMode() const {
                return current().selectionMode;
            }
            
            inline bool empty() const {
                return current().selectionMode == TB_SM_NONE;
            }
            
            inline const std::vector<Assets::Texture*>& mruTextures() const {
                return current().mruTextures;
            }
            
            inline Assets::Texture* texture() const {
                if (current().mruTextures.empty())
                    return NULL;
                return current().mruTextures.back();
            }
            
            inline const FaceList& selectedFaces() const {
                return current().selectedFaces;
            }
            
            const FaceList selectedBrushFaces() const;
            const FaceList allSelectedFaces() const;
            
            const BrushList& selectedBrushes() const {
                return current().selectedBrushes;
            }
            
            const EntityList& selectedEntities() const {
                return current().selectedEntities;
            }
            const EntityList allSelectedEntities() const;
            
            const Entity* brushSelectionEntity() const;
            Vec3f center() const;
            BBox bounds() const;
            
            void selectTexture(Assets::Texture& texture);
            void selectFace(Face& face);
            void selectFaces(const FaceList& faces);
            void selectBrush(Brush& brush);
            void selectBrushes(const BrushList& brushes);
            void selectEntity(Entity& entity);
            void selectEntities(const EntityList& entities);
            
            void replaceSelection(const EntityList& entities, const BrushList& brushes);
            void replaceSelection(const EntityList& entities);
            void replaceSelection(const BrushList& brushes);
            void replaceSelection(Entity& entity);
            void replaceSelection(Brush& brush);
            void replaceSelection(const FaceList& faces);
            void replaceSelection(Face& face);
            
            void deselectFace(Face& face);
            void deselectFaces(const FaceList& faces);
            void deselectBrush(Brush& brush);
            void deselectBrushes(const BrushList& brushes);
            void deselectEntity(Entity& entity);
            void deselectEntities(const EntityList& entities);
            void deselectAll();
        };
    }
}

#endif
