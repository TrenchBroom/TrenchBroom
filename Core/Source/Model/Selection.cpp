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

#include "Selection.h"

#include "Model/Map/BrushGeometry.h"

#include <cassert>
#include <cmath>
#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        void Selection::doSelectEntities(const EntityList& entities) {
            for (unsigned int i = 0; i < entities.size(); i++) {
                Entity* entity = entities[i];
                current().selectedEntities.push_back(entity);
                entity->setSelected(true);
            }
        }
        
        void Selection::doSelectBrushes(const BrushList& brushes) {
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Brush* brush = brushes[i];
                current().selectedBrushes.push_back(brush);
                brush->selected = true;
            }
        }

        void Selection::doSelectFaces(const FaceList& faces) {
            for (unsigned int i = 0; i < faces.size(); i++) {
                Face* face = faces[i];
                current().selectedFaces.push_back(face);
                face->selected = true;
                face->brush->partiallySelected = true;
                if (find(current().partiallySelectedBrushes.begin(), current().partiallySelectedBrushes.end(), face->brush) == current().partiallySelectedBrushes.end())
                    current().partiallySelectedBrushes.push_back(face->brush);
            }
            
            if (faces.back()->texture != NULL)
                selectTexture(*faces.back()->texture);
        }

        EntityList Selection::doDeselectEntities(const EntityList& entities) {
            EntityList deselectedEntities;
            for (unsigned int i = 0; i < entities.size(); i++) {
                Entity* entity = entities[i];
                EntityList::iterator it = find(current().selectedEntities.begin(), current().selectedEntities.end(), entity);
                if (it != current().selectedEntities.end()) {
                    current().selectedEntities.erase(it);
                    entity->setSelected(false);
                    deselectedEntities.push_back(entity);
                }
            }
            
            return deselectedEntities;
        }

        BrushList Selection::doDeselectBrushes(const BrushList& brushes) {
            BrushList deselectedBrushes;
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Brush* brush = brushes[i];
                BrushList::iterator it = find(current().selectedBrushes.begin(), current().selectedBrushes.end(), brush);
                if (it != current().selectedBrushes.end()) {
                    current().selectedBrushes.erase(it);
                    brush->selected = false;
                    deselectedBrushes.push_back(brush);
                }
            }
            
            return deselectedBrushes;
        }

        FaceList Selection::doDeselectFaces(const FaceList& faces) {
            FaceList deselectedFaces;
            for (unsigned int i = 0; i < faces.size(); i++) {
                Face* face = faces[i];
                FaceList::iterator it = find(current().selectedFaces.begin(), current().selectedFaces.end(), face);
                if (it != current().selectedFaces.end()) {
                    current().selectedFaces.erase(it);
                    face->selected = false;
                    deselectedFaces.push_back(face);
                    
                    const FaceList siblings = face->brush->faces;
                    face->brush->partiallySelected = false;
                    for (unsigned int j = 0; j < siblings.size() && !face->brush->partiallySelected; j++)
                        face->brush->partiallySelected = siblings[j]->selected;
                    if (!face->brush->partiallySelected)
                        current().partiallySelectedBrushes.erase(find(current().partiallySelectedBrushes.begin(), current().partiallySelectedBrushes.end(), face->brush));
                }
            }
            
            return deselectedFaces;
        }

        void Selection::push() {
            m_state.push_back(current());
        }
        
        void Selection::pop() {
            assert(m_state.size() > 1);
            
            deselectAll();
            m_state.pop_back();

            State state = current();
            current().clear();
            
            selectEntities(state.selectedEntities);
            selectBrushes(state.selectedBrushes);
            selectFaces(state.selectedFaces);
            current().mruTextures = state.mruTextures;
        }

        const FaceList Selection::selectedBrushFaces() const {
            FaceList faces;
            for (unsigned int i = 0; i < current().selectedBrushes.size(); i++) {
                FaceList brushFaces = current().selectedBrushes[i]->faces;
                for (unsigned int j = 0; j < brushFaces.size(); j++)
                    faces.push_back(brushFaces[j]);
            }
            return faces;
        }

        const FaceList Selection::allSelectedFaces() const {
            FaceList allFaces = selectedBrushFaces();
            allFaces.insert(allFaces.begin(), current().selectedFaces.begin(), current().selectedFaces.end());
            return allFaces;
        }

        const Entity* Selection::brushSelectionEntity() const {
            if (current().selectionMode != TB_SM_BRUSHES)
                return NULL;

            Entity* entity = current().selectedBrushes[0]->entity;
            for (unsigned int i = 1; i < current().selectedBrushes.size(); i++) {
                if (current().selectedBrushes[i]->entity != entity)
                    return NULL;
            }

            return entity;
        }

        Vec3f Selection::center() const {
            Vec3f center;
            switch (current().selectionMode) {
                case TB_SM_FACES:
                    center = current().selectedFaces[0]->center();
                    for (unsigned int i = 1; i < current().selectedFaces.size(); i++)
                        center += current().selectedFaces[i]->center();
                    center /= static_cast<float>(current().selectedFaces.size());
                    break;
                case TB_SM_BRUSHES:
                    center = current().selectedBrushes[0]->center();
                    for (unsigned int i = 1; i < current().selectedBrushes.size(); i++)
                        center += current().selectedBrushes[i]->center();
                    center /= static_cast<float>(current().selectedBrushes.size());
                    break;
                case TB_SM_ENTITIES:
                    center = current().selectedEntities[0]->center();
                    for (unsigned int i = 1; i < current().selectedEntities.size(); i++)
                        center += current().selectedEntities[i]->center();
                    center /= static_cast<float>(current().selectedEntities.size());
                    break;
                case TB_SM_BRUSHES_ENTITIES:
                    center = current().selectedBrushes[0]->center();
                    for (unsigned int i = 1; i < current().selectedBrushes.size(); i++)
                        center += current().selectedBrushes[i]->center();
                    for (unsigned int i = 0; i < current().selectedEntities.size(); i++)
                        center += current().selectedEntities[i]->center();
                    center /= static_cast<float>(current().selectedBrushes.size() + current().selectedEntities.size());
                    break;
                default:
                    center = Vec3f::NaN;
                    break;
            }

            return center;
        }

        BBox Selection::bounds() const {
            BBox bounds;
            switch (current().selectionMode) {
                case TB_SM_FACES:
                    bounds = current().selectedFaces[0]->brush->bounds();
                    for (unsigned int i = 1; i < current().selectedFaces.size(); i++)
                        bounds += current().selectedFaces[i]->brush->bounds();
                    break;
                case TB_SM_BRUSHES:
                    bounds = current().selectedBrushes[0]->bounds();
                    for (unsigned int i = 1; i < current().selectedBrushes.size(); i++)
                        bounds += current().selectedBrushes[i]->bounds();
                    break;
                case TB_SM_ENTITIES:
                    bounds = current().selectedEntities[0]->bounds();
                    for (unsigned int i = 1; i < current().selectedEntities.size(); i++)
                        bounds += current().selectedEntities[i]->bounds();
                    break;
                case TB_SM_BRUSHES_ENTITIES:
                    bounds = current().selectedBrushes[0]->bounds();
                    for (unsigned int i = 1; i < current().selectedBrushes.size(); i++)
                        bounds += current().selectedBrushes[i]->bounds();
                    for (unsigned int i = 0; i < current().selectedEntities.size(); i++)
                        bounds += current().selectedEntities[i]->bounds();
                    break;
                default:
                    bounds.min = bounds.max = Vec3f::NaN;
                    break;
            }
            return bounds;
        }

        void Selection::selectTexture(Assets::Texture& texture) {
            std::vector<Assets::Texture*>::iterator it = find(current().mruTextures.begin(), current().mruTextures.end(), &texture);
            if (it != current().mruTextures.end())
                current().mruTextures.erase(it);
            current().mruTextures.push_back(&texture);
        }

        void Selection::selectFace(Face& face) {
            if (current().selectionMode != TB_SM_FACES)
                deselectAll();

            current().selectedFaces.push_back(&face);
            face.selected = true;
            face.brush->partiallySelected = true;

            if (find(current().partiallySelectedBrushes.begin(), current().partiallySelectedBrushes.end(), face.brush) == current().partiallySelectedBrushes.end())
                current().partiallySelectedBrushes.push_back(face.brush);

            if (face.texture != NULL)
                selectTexture(*face.texture);
            current().selectionMode = TB_SM_FACES;

            SelectionEventData data(face);
            selectionAdded(data);
        }

        void Selection::selectFaces(const FaceList& faces) {
            if (faces.empty())
                return;
            if (current().selectionMode != TB_SM_FACES)
                deselectAll();

            doSelectFaces(faces);
            current().selectionMode = TB_SM_FACES;

            SelectionEventData data(faces);
            selectionAdded(data);
        }

        void Selection::selectBrush(Brush& brush) {
            if (current().selectionMode == TB_SM_FACES)
                deselectAll();

            current().selectedBrushes.push_back(&brush);
            brush.selected = true;

            if (current().selectionMode == TB_SM_ENTITIES)
                current().selectionMode = TB_SM_BRUSHES_ENTITIES;
            else
                current().selectionMode = TB_SM_BRUSHES;

            SelectionEventData data(brush);
            selectionAdded(data);
        }

        void Selection::selectBrushes(const BrushList& brushes) {
            if (brushes.empty())
                return;
            if (current().selectionMode == TB_SM_FACES)
                deselectAll();

            doSelectBrushes(brushes);

            if (current().selectionMode == TB_SM_ENTITIES)
                current().selectionMode = TB_SM_BRUSHES_ENTITIES;
            else
                current().selectionMode = TB_SM_BRUSHES;

            SelectionEventData data(brushes);
            selectionAdded(data);
        }

        void Selection::selectEntity(Entity& entity) {
            if (current().selectionMode == TB_SM_FACES)
                deselectAll();

            current().selectedEntities.push_back(&entity);
            entity.setSelected(true);

            if (current().selectionMode == TB_SM_BRUSHES)
                current().selectionMode = TB_SM_BRUSHES_ENTITIES;
            else
                current().selectionMode = TB_SM_ENTITIES;

            SelectionEventData data(entity);
            selectionAdded(data);
        }

        void Selection::selectEntities(const EntityList& entities) {
            if (entities.empty())
                return;
            if (current().selectionMode == TB_SM_FACES)
                deselectAll();

            doSelectEntities(entities);

            if (current().selectionMode == TB_SM_BRUSHES)
                current().selectionMode = TB_SM_BRUSHES_ENTITIES;
            else
                current().selectionMode = TB_SM_ENTITIES;

            SelectionEventData data(entities);
            selectionAdded(data);
        }

        void Selection::replaceSelection(const EntityList& entities, const BrushList& brushes) {
            if (entities.empty() && brushes.empty())
                return;
            
            switch (current().selectionMode) {
                case TB_SM_ENTITIES: {
                    EntityList selectedEntities = current().selectedEntities;
                    selectEntities(entities);
                    selectBrushes(brushes);
                    deselectEntities(selectedEntities);
                    break;
                }
                case TB_SM_BRUSHES: {
                    BrushList selectedBrushes = current().selectedBrushes;
                    selectEntities(entities);
                    selectBrushes(brushes);
                    deselectBrushes(selectedBrushes);
                    break;
                }
                case TB_SM_BRUSHES_ENTITIES: {
                    EntityList selectedEntities = current().selectedEntities;
                    BrushList selectedBrushes = current().selectedBrushes;
                    selectEntities(entities);
                    selectBrushes(brushes);
                    deselectEntities(selectedEntities);
                    deselectBrushes(selectedBrushes);
                    break;
                }
                case TB_SM_FACES: {
                    FaceList selectedFaces = current().selectedFaces;
                    doDeselectFaces(selectedFaces);
                    doSelectEntities(entities);
                    doSelectBrushes(brushes);
                    
                    if (!entities.empty() && !brushes.empty())
                        current().selectionMode = TB_SM_BRUSHES_ENTITIES;
                    else if (!entities.empty())
                        current().selectionMode = TB_SM_ENTITIES;
                    else
                        current().selectionMode = TB_SM_BRUSHES;
                    
                    selectionAdded(SelectionEventData(entities, brushes));
                    selectionRemoved(SelectionEventData(selectedFaces));
                    break;
                }
                default:
                    selectEntities(entities);
                    selectBrushes(brushes);
                    break;
            }
        }
        
        void Selection::replaceSelection(const EntityList& entities) {
            replaceSelection(entities, BrushList());
        }
        
        void Selection::replaceSelection(const BrushList& brushes) {
            replaceSelection(EntityList(), brushes);
        }
        
        void Selection::replaceSelection(Entity& entity) {
            EntityList entityList;
            entityList.push_back(&entity);
            replaceSelection(entityList, BrushList());
        }
        
        void Selection::replaceSelection(Brush& brush) {
            BrushList brushList;
            brushList.push_back(&brush);
            replaceSelection(EntityList(), brushList);
        }

        void Selection::replaceSelection(const FaceList& faces) {
            switch (current().selectionMode) {
                case TB_SM_ENTITIES: {
                    EntityList selectedEntities = current().selectedEntities;
                    doDeselectEntities(selectedEntities);
                    doSelectFaces(faces);
                    current().selectionMode = TB_SM_FACES;

                    selectionAdded(SelectionEventData(faces));
                    selectionRemoved(SelectionEventData(selectedEntities));
                    break;
                }
                case TB_SM_BRUSHES: {
                    BrushList selectedBrushes = current().selectedBrushes;
                    doDeselectBrushes(selectedBrushes);
                    doSelectFaces(faces);
                    current().selectionMode = TB_SM_FACES;
                    
                    selectionAdded(SelectionEventData(faces));
                    selectionRemoved(SelectionEventData(selectedBrushes));
                    break;
                }
                case TB_SM_BRUSHES_ENTITIES: {
                    EntityList selectedEntities = current().selectedEntities;
                    BrushList selectedBrushes = current().selectedBrushes;
                    doDeselectEntities(selectedEntities);
                    doDeselectBrushes(selectedBrushes);
                    doSelectFaces(faces);
                    current().selectionMode = TB_SM_FACES;

                    selectionAdded(SelectionEventData(faces));
                    selectionRemoved(SelectionEventData(selectedEntities, selectedBrushes));
                    break;
                }
                case TB_SM_FACES: {
                    FaceList selectedFaces = current().selectedFaces;
                    selectFaces(faces);
                    deselectFaces(selectedFaces);
                    break;
                }
                default:
                    selectFaces(faces);
                    break;
            }

        }

        void Selection::replaceSelection(Face& face) {
            FaceList faceList;
            faceList.push_back(&face);
            replaceSelection(faceList);
        }

        void Selection::deselectFace(Face& face) {
            FaceList::iterator it = find(current().selectedFaces.begin(), current().selectedFaces.end(), &face);
            if (it == current().selectedFaces.end())
                return;

            current().selectedFaces.erase(it);
            face.selected = false;

            if (current().selectedFaces.size() == 0) {
                current().selectionMode = TB_SM_NONE;
                current().partiallySelectedBrushes.clear();
            } else {
                const FaceList siblings = face.brush->faces;
                face.brush->partiallySelected = false;
                for (unsigned int i = 0; i < siblings.size() && !face.brush->partiallySelected; i++)
                    face.brush->partiallySelected = siblings[i]->selected;
                if (!face.brush->partiallySelected)
                    current().partiallySelectedBrushes.erase(find(current().partiallySelectedBrushes.begin(), current().partiallySelectedBrushes.end(), face.brush));
            }

            SelectionEventData data(face);
            selectionRemoved(data);
        }

        void Selection::deselectFaces(const FaceList& faces) {
            if (faces.empty())
                return;

            FaceList deselectedFaces = doDeselectFaces(faces);
            if (current().selectedFaces.size() == 0)
                current().selectionMode = TB_SM_NONE;

            SelectionEventData data(deselectedFaces);
            selectionRemoved(data);
        }

        void Selection::deselectBrush(Brush& brush) {
            BrushList::iterator it = find(current().selectedBrushes.begin(), current().selectedBrushes.end(), &brush);
            if (it == current().selectedBrushes.end())
                return;

            current().selectedBrushes.erase(it);
            brush.selected = false;

            if (current().selectedBrushes.empty()) {
                if (current().selectedEntities.empty())
                    current().selectionMode = TB_SM_NONE;
                else
                    current().selectionMode = TB_SM_ENTITIES;
            }

            SelectionEventData data(brush);
            selectionRemoved(data);
        }

        void Selection::deselectBrushes(const BrushList& brushes) {
            if (brushes.empty())
                return;

            BrushList deselectedBrushes = doDeselectBrushes(brushes);

            if (current().selectedBrushes.empty()) {
                if (current().selectedEntities.empty())
                    current().selectionMode = TB_SM_NONE;
                else
                    current().selectionMode = TB_SM_ENTITIES;
            }

            SelectionEventData data(deselectedBrushes);
            selectionRemoved(data);
        }

        void Selection::deselectEntity(Entity& entity) {
            EntityList::iterator it = find(current().selectedEntities.begin(), current().selectedEntities.end(), &entity);
            if (it == current().selectedEntities.end())
                return;

            current().selectedEntities.erase(it);
            entity.setSelected(false);

            if (current().selectedEntities.empty()) {
                if (current().selectedBrushes.empty())
                    current().selectionMode = TB_SM_NONE;
                else
                    current().selectionMode = TB_SM_BRUSHES;
            }

            SelectionEventData data(entity);
            selectionRemoved(data);
        }

        void Selection::deselectEntities(const EntityList& entities) {
            if (entities.empty())
                return;

            EntityList deselectedEntities = doDeselectEntities(entities);

            if (current().selectedEntities.empty()) {
                if (current().selectedBrushes.empty())
                    current().selectionMode = TB_SM_NONE;
                else
                    current().selectionMode = TB_SM_BRUSHES;
            }

            SelectionEventData data(deselectedEntities);
            selectionRemoved(data);
        }

        void Selection::deselectAll() {
            if (current().selectedFaces.empty() && current().selectedBrushes.empty() && current().selectedEntities.empty())
                return;

            SelectionEventData data;

            if (!current().selectedFaces.empty()) {
                data.faces = current().selectedFaces;
                for (unsigned int i = 0; i < current().selectedFaces.size(); i++)
                    current().selectedFaces[i]->selected = false;
                current().selectedFaces.clear();
                for (unsigned int i = 0; i < current().partiallySelectedBrushes.size(); i++)
                    current().partiallySelectedBrushes[i]->partiallySelected = false;
                current().partiallySelectedBrushes.clear();
                current().selectionMode = TB_SM_NONE;
            }

            if (!current().selectedBrushes.empty()) {
                data.brushes = current().selectedBrushes;
                for (unsigned int i = 0; i < current().selectedBrushes.size(); i++)
                    current().selectedBrushes[i]->selected = false;
                current().selectedBrushes.clear();
                current().selectionMode = TB_SM_NONE;
            }

            if (!current().selectedEntities.empty()) {
                data.entities = current().selectedEntities;
                for (unsigned int i = 0; i < current().selectedEntities.size(); i++)
                    current().selectedEntities[i]->setSelected(false);
                current().selectedEntities.clear();
                current().selectionMode = TB_SM_NONE;
            }

            selectionRemoved(data);
        }
    }
}
