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
        void Selection::push() {
            m_state.push_back(current());
        }
        
        void Selection::pop() {
            assert(m_state.size() > 1);
            
            removeAll();
            m_state.pop_back();

            SelectionState state = current();
            current().clear();
            
            addEntities(state.entities);
            addBrushes(state.brushes);
            addFaces(state.faces);
            current().mruTextures = state.mruTextures;
        }

        const FaceList Selection::brushFaces() const {
            FaceList faces;
            for (unsigned int i = 0; i < current().brushes.size(); i++) {
                FaceList brushFaces = current().brushes[i]->faces;
                for (unsigned int j = 0; j < brushFaces.size(); j++)
                    faces.push_back(brushFaces[j]);
            }
            return faces;
        }

        const FaceList Selection::allFaces() const {
            FaceList allFaces = brushFaces();
            allFaces.insert(allFaces.begin(), current().faces.begin(), current().faces.end());
            return allFaces;
        }

        const Entity* Selection::brushSelectionEntity() const {
            if (current().mode != TB_SM_BRUSHES)
                return NULL;

            Entity* entity = current().brushes[0]->entity;
            for (unsigned int i = 1; i < current().brushes.size(); i++) {
                if (current().brushes[i]->entity != entity)
                    return NULL;
            }

            return entity;
        }

        Vec3f Selection::center() const {
            Vec3f center;
            switch (current().mode) {
                case TB_SM_FACES:
                    center = current().faces[0]->center();
                    for (unsigned int i = 1; i < current().faces.size(); i++)
                        center += current().faces[i]->center();
                    center /= static_cast<float>(current().faces.size());
                    break;
                case TB_SM_BRUSHES:
                    center = current().brushes[0]->center();
                    for (unsigned int i = 1; i < current().brushes.size(); i++)
                        center += current().brushes[i]->center();
                    center /= static_cast<float>(current().brushes.size());
                    break;
                case TB_SM_ENTITIES:
                    center = current().entities[0]->center();
                    for (unsigned int i = 1; i < current().entities.size(); i++)
                        center += current().entities[i]->center();
                    center /= static_cast<float>(current().entities.size());
                    break;
                case TB_SM_BRUSHES_ENTITIES:
                    center = current().brushes[0]->center();
                    for (unsigned int i = 1; i < current().brushes.size(); i++)
                        center += current().brushes[i]->center();
                    for (unsigned int i = 0; i < current().entities.size(); i++)
                        center += current().entities[i]->center();
                    center /= static_cast<float>(current().brushes.size() + current().entities.size());
                    break;
                default:
                    center = Vec3f::NaN;
                    break;
            }

            return center;
        }

        BBox Selection::bounds() const {
            BBox bounds;
            switch (current().mode) {
                case TB_SM_FACES:
                    bounds = current().faces[0]->brush->bounds();
                    for (unsigned int i = 1; i < current().faces.size(); i++)
                        bounds += current().faces[i]->brush->bounds();
                    break;
                case TB_SM_BRUSHES:
                    bounds = current().brushes[0]->bounds();
                    for (unsigned int i = 1; i < current().brushes.size(); i++)
                        bounds += current().brushes[i]->bounds();
                    break;
                case TB_SM_ENTITIES:
                    bounds = current().entities[0]->bounds();
                    for (unsigned int i = 1; i < current().entities.size(); i++)
                        bounds += current().entities[i]->bounds();
                    break;
                case TB_SM_BRUSHES_ENTITIES:
                    bounds = current().brushes[0]->bounds();
                    for (unsigned int i = 1; i < current().brushes.size(); i++)
                        bounds += current().brushes[i]->bounds();
                    for (unsigned int i = 0; i < current().entities.size(); i++)
                        bounds += current().entities[i]->bounds();
                    break;
                default:
                    bounds.min = bounds.max = Vec3f::NaN;
                    break;
            }
            return bounds;
        }

        void Selection::addTexture(Assets::Texture& texture) {
            std::vector<Assets::Texture*>::iterator it = find(current().mruTextures.begin(), current().mruTextures.end(), &texture);
            if (it != current().mruTextures.end())
                current().mruTextures.erase(it);
            current().mruTextures.push_back(&texture);
        }

        void Selection::addFace(Face& face) {
            if (current().mode != TB_SM_FACES) removeAll();

            current().faces.push_back(&face);
            face.selected = true;
            face.brush->partiallySelected = true;

            if (find(current().partialBrushes.begin(), current().partialBrushes.end(), face.brush) == current().partialBrushes.end())
                current().partialBrushes.push_back(face.brush);

            if (face.texture != NULL)
                addTexture(*face.texture);
            current().mode = TB_SM_FACES;

            SelectionEventData data(face);
            selectionAdded(data);
        }

        void Selection::addFaces(const FaceList& faces) {
            if (faces.empty()) return;
            if (current().mode != TB_SM_FACES) removeAll();

            for (unsigned int i = 0; i < faces.size(); i++) {
                Face* face = faces[i];
                current().faces.push_back(face);
                face->selected = true;
                face->brush->partiallySelected = true;
                if (find(current().partialBrushes.begin(), current().partialBrushes.end(), face->brush) == current().partialBrushes.end())
                    current().partialBrushes.push_back(face->brush);
            }

            if (faces.back()->texture != NULL)
                addTexture(*faces.back()->texture);
            current().mode = TB_SM_FACES;

            SelectionEventData data(faces);
            selectionAdded(data);
        }

        void Selection::addBrush(Brush& brush) {
            if (current().mode == TB_SM_FACES) removeAll();

            current().brushes.push_back(&brush);
            brush.selected = true;

            if (current().mode == TB_SM_ENTITIES) current().mode = TB_SM_BRUSHES_ENTITIES;
            else current().mode = TB_SM_BRUSHES;

            SelectionEventData data(brush);
            selectionAdded(data);
        }

        void Selection::addBrushes(const BrushList& brushes) {
            if (brushes.empty()) return;
            if (current().mode == TB_SM_FACES) removeAll();

            for (unsigned int i = 0; i < brushes.size(); i++) {
                Brush* brush = brushes[i];
                current().brushes.push_back(brush);
                brush->selected = true;
            }

            if (current().mode == TB_SM_ENTITIES) current().mode = TB_SM_BRUSHES_ENTITIES;
            else current().mode = TB_SM_BRUSHES;

            SelectionEventData data(brushes);
            selectionAdded(data);
        }

        void Selection::addEntity(Entity& entity) {
            if (current().mode == TB_SM_FACES) removeAll();

            current().entities.push_back(&entity);
            entity.setSelected(true);

            if (current().mode == TB_SM_BRUSHES) current().mode = TB_SM_BRUSHES_ENTITIES;
            else current().mode = TB_SM_ENTITIES;

            SelectionEventData data(entity);
            selectionAdded(data);
        }

        void Selection::addEntities(const EntityList& entities) {
            if (entities.empty()) return;
            if (current().mode == TB_SM_FACES) removeAll();

            for (unsigned int i = 0; i < entities.size(); i++) {
                Entity* entity = entities[i];
                current().entities.push_back(entity);
                entity->setSelected(true);
            }

            if (current().mode == TB_SM_BRUSHES) current().mode = TB_SM_BRUSHES_ENTITIES;
            else current().mode = TB_SM_ENTITIES;

            SelectionEventData data(entities);
            selectionAdded(data);
        }

        void Selection::removeFace(Face& face) {
            FaceList::iterator it = find(current().faces.begin(), current().faces.end(), &face);
            if (it == current().faces.end()) return;

            current().faces.erase(it);
            face.selected = false;

            if (current().faces.size() == 0) {
                current().mode = TB_SM_NONE;
                current().partialBrushes.clear();
            } else {
                const FaceList siblings = face.brush->faces;
                face.brush->partiallySelected = false;
                for (unsigned int i = 0; i < siblings.size() && !face.brush->partiallySelected; i++)
                    face.brush->partiallySelected = siblings[i]->selected;
                if (!face.brush->partiallySelected)
                    current().partialBrushes.erase(find(current().partialBrushes.begin(), current().partialBrushes.end(), face.brush));
            }

            SelectionEventData data(face);
            selectionRemoved(data);
        }

        void Selection::removeFaces(const FaceList& faces) {
            if (faces.empty()) return;

            FaceList removedFaces;
            for (unsigned int i = 0; i < faces.size(); i++) {
                Face* face = faces[i];
                FaceList::iterator it = find(current().faces.begin(), current().faces.end(), face);
                if (it != current().faces.end()) {
                    current().faces.erase(it);
                    face->selected = false;
                    removedFaces.push_back(face);

                    const FaceList siblings = face->brush->faces;
                    face->brush->partiallySelected = false;
                    for (unsigned int j = 0; j < siblings.size() && !face->brush->partiallySelected; j++)
                        face->brush->partiallySelected = siblings[j]->selected;
                    if (!face->brush->partiallySelected)
                        current().partialBrushes.erase(find(current().partialBrushes.begin(), current().partialBrushes.end(), face->brush));
                }
            }

            if (current().faces.size() == 0)
                current().mode = TB_SM_NONE;

            SelectionEventData data(faces);
            selectionRemoved(data);
        }

        void Selection::removeBrush(Brush& brush) {
            BrushList::iterator it = find(current().brushes.begin(), current().brushes.end(), &brush);
            if (it == current().brushes.end()) return;

            current().brushes.erase(it);
            brush.selected = false;

            if (current().brushes.empty()) {
                if (current().entities.empty()) current().mode = TB_SM_NONE;
                else current().mode = TB_SM_ENTITIES;
            }

            SelectionEventData data(brush);
            selectionRemoved(data);
        }

        void Selection::removeBrushes(const BrushList& brushes) {
            if (brushes.empty()) return;

            BrushList removedBrushes;
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Brush* brush = brushes[i];
                BrushList::iterator it = find(current().brushes.begin(), current().brushes.end(), brush);
                if (it != current().brushes.end()) {
                    current().brushes.erase(it);
                    brush->selected = false;
                    removedBrushes.push_back(brush);
                }
            }

            if (current().brushes.empty()) {
                if (current().entities.empty()) current().mode = TB_SM_NONE;
                else current().mode = TB_SM_ENTITIES;
            }

            SelectionEventData data(removedBrushes);
            selectionRemoved(data);
        }

        void Selection::removeEntity(Entity& entity) {
            EntityList::iterator it = find(current().entities.begin(), current().entities.end(), &entity);
            if (it == current().entities.end()) return;

            current().entities.erase(it);
            entity.setSelected(false);

            if (current().entities.empty()) {
                if (current().brushes.empty()) current().mode = TB_SM_NONE;
                else current().mode = TB_SM_BRUSHES;
            }

            SelectionEventData data(entity);
            selectionRemoved(data);
        }

        void Selection::removeEntities(const EntityList& entities) {
            if (entities.empty()) return;

            EntityList removedEntities;
            for (unsigned int i = 0; i < entities.size(); i++) {
                Entity* entity = entities[i];
                EntityList::iterator it = find(current().entities.begin(), current().entities.end(), entity);
                if (it != current().entities.end()) {
                    current().entities.erase(it);
                    entity->setSelected(false);
                    removedEntities.push_back(entity);
                }
            }

            if (current().entities.empty()) {
                if (current().brushes.empty()) current().mode = TB_SM_NONE;
                else current().mode = TB_SM_BRUSHES;
            }

            SelectionEventData data(removedEntities);
            selectionRemoved(data);
        }

        void Selection::removeAll() {
            if (current().faces.empty() && current().brushes.empty() && current().entities.empty()) return;

            SelectionEventData data;

            if (!current().faces.empty()) {
                data.faces = current().faces;
                for (unsigned int i = 0; i < current().faces.size(); i++)
                    current().faces[i]->selected = false;
                current().faces.clear();
                for (unsigned int i = 0; i < current().partialBrushes.size(); i++)
                    current().partialBrushes[i]->partiallySelected = false;
                current().partialBrushes.clear();
                current().mode = TB_SM_NONE;
            }

            if (!current().brushes.empty()) {
                data.brushes = current().brushes;
                for (unsigned int i = 0; i < current().brushes.size(); i++)
                    current().brushes[i]->selected = false;
                current().brushes.clear();
                current().mode = TB_SM_NONE;
            }

            if (!current().entities.empty()) {
                data.entities = current().entities;
                for (unsigned int i = 0; i < current().entities.size(); i++)
                    current().entities[i]->setSelected(false);
                current().entities.clear();
                current().mode = TB_SM_NONE;
            }

            selectionRemoved(data);
        }
    }
}
