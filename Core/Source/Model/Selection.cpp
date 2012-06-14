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
#include <cmath>
#include <algorithm>
#include "Model/Map/BrushGeometry.h"

namespace TrenchBroom {
    namespace Model {
        Selection::Selection() : m_mode(TB_SM_NONE) {}

        ESelectionMode Selection::mode() const {
            return m_mode;
        }

        bool Selection::empty() const {
            return m_entities.empty() && m_brushes.empty() && m_faces.empty();
        }

        const std::vector<Assets::Texture*>& Selection::mruTextures() const {
            return m_mruTextures;
        }

        const FaceList& Selection::faces() const {
            return m_faces;
        }

        const FaceList Selection::brushFaces() const {
            FaceList faces;
            for (unsigned int i = 0; i < m_brushes.size(); i++) {
                FaceList brushFaces = m_brushes[i]->faces;
                for (unsigned int j = 0; j < brushFaces.size(); j++)
                    faces.push_back(brushFaces[j]);
            }
            return faces;
        }

        const FaceList Selection::allFaces() const {
            FaceList allFaces = brushFaces();
            allFaces.insert(allFaces.begin(), m_faces.begin(), m_faces.end());
            return allFaces;
        }

        const BrushList& Selection::brushes() const {
            return m_brushes;
        }

        const BrushList& Selection::partialBrushes() const {
            return m_partialBrushes;
        }

        const EntityList& Selection::entities() const {
            return m_entities;
        }

        const Entity* Selection::brushSelectionEntity() const {
            if (m_mode != TB_SM_BRUSHES)
                return NULL;

            Entity* entity = m_brushes[0]->entity;
            for (unsigned int i = 1; i < m_brushes.size(); i++) {
                if (m_brushes[i]->entity != entity)
                    return NULL;
            }

            return entity;
        }

        Vec3f Selection::center() const {
            Vec3f center;
            switch (m_mode) {
                case TB_SM_FACES:
                    center = m_faces[0]->center();
                    for (unsigned int i = 1; i < m_faces.size(); i++)
                        center += m_faces[i]->center();
                    center /= static_cast<float>(m_faces.size());
                    break;
                case TB_SM_BRUSHES:
                    center = m_brushes[0]->center();
                    for (unsigned int i = 1; i < m_brushes.size(); i++)
                        center += m_brushes[i]->center();
                    center /= static_cast<float>(m_brushes.size());
                    break;
                case TB_SM_ENTITIES:
                    center = m_entities[0]->center();
                    for (unsigned int i = 1; i < m_entities.size(); i++)
                        center += m_entities[i]->center();
                    center /= static_cast<float>(m_entities.size());
                    break;
                case TB_SM_BRUSHES_ENTITIES:
                    center = m_brushes[0]->center();
                    for (unsigned int i = 1; i < m_brushes.size(); i++)
                        center += m_brushes[i]->center();
                    for (unsigned int i = 0; i < m_entities.size(); i++)
                        center += m_entities[i]->center();
                    center /= static_cast<float>(m_brushes.size() + m_entities.size());
                    break;
                default:
                    center = Nan3f;
                    break;
            }

            return center;
        }

        BBox Selection::bounds() const {
            BBox bounds;
            switch (m_mode) {
                case TB_SM_FACES:
                    bounds = m_faces[0]->brush->bounds();
                    for (unsigned int i = 1; i < m_faces.size(); i++)
                        bounds += m_faces[i]->brush->bounds();
                    break;
                case TB_SM_BRUSHES:
                    bounds = m_brushes[0]->bounds();
                    for (unsigned int i = 1; i < m_brushes.size(); i++)
                        bounds += m_brushes[i]->bounds();
                    break;
                case TB_SM_ENTITIES:
                    bounds = m_entities[0]->bounds();
                    for (unsigned int i = 1; i < m_entities.size(); i++)
                        bounds += m_entities[i]->bounds();
                    break;
                case TB_SM_BRUSHES_ENTITIES:
                    bounds = m_brushes[0]->bounds();
                    for (unsigned int i = 1; i < m_brushes.size(); i++)
                        bounds += m_brushes[i]->bounds();
                    for (unsigned int i = 0; i < m_entities.size(); i++)
                        bounds += m_entities[i]->bounds();
                    break;
                default:
                    bounds.min = Nan3f;
                    bounds.max = Nan3f;
                    break;
            }
            return bounds;
        }

        void Selection::addTexture(Assets::Texture& texture) {
            std::vector<Assets::Texture*>::iterator it = find(m_mruTextures.begin(), m_mruTextures.end(), &texture);
            if (it != m_mruTextures.end())
                m_mruTextures.erase(it);
            m_mruTextures.push_back(&texture);
        }

        void Selection::addFace(Face& face) {
            if (m_mode != TB_SM_FACES) removeAll();

            m_faces.push_back(&face);
            face.selected = true;
            face.brush->partiallySelected = true;

            if (find(m_partialBrushes.begin(), m_partialBrushes.end(), face.brush) == m_partialBrushes.end())
                m_partialBrushes.push_back(face.brush);

            if (face.texture != NULL)
                addTexture(*face.texture);
            m_mode = TB_SM_FACES;

            SelectionEventData data(face);
            selectionAdded(data);
        }

        void Selection::addFaces(const FaceList& faces) {
            if (faces.empty()) return;
            if (m_mode != TB_SM_FACES) removeAll();

            for (unsigned int i = 0; i < faces.size(); i++) {
                Face* face = faces[i];
                m_faces.push_back(face);
                face->selected = true;
                face->brush->partiallySelected = true;
                if (find(m_partialBrushes.begin(), m_partialBrushes.end(), face->brush) == m_partialBrushes.end())
                    m_partialBrushes.push_back(face->brush);
            }

            if (faces.back()->texture != NULL)
                addTexture(*faces.back()->texture);
            m_mode = TB_SM_FACES;

            SelectionEventData data(faces);
            selectionAdded(data);
        }

        void Selection::addBrush(Brush& brush) {
            if (m_mode == TB_SM_FACES) removeAll();

            m_brushes.push_back(&brush);
            brush.selected = true;

            if (m_mode == TB_SM_ENTITIES) m_mode = TB_SM_BRUSHES_ENTITIES;
            else m_mode = TB_SM_BRUSHES;

            SelectionEventData data(brush);
            selectionAdded(data);
        }

        void Selection::addBrushes(const BrushList& brushes) {
            if (brushes.empty()) return;
            if (m_mode == TB_SM_FACES) removeAll();

            for (unsigned int i = 0; i < brushes.size(); i++) {
                Brush* brush = brushes[i];
                m_brushes.push_back(brush);
                brush->selected = true;
            }

            if (m_mode == TB_SM_ENTITIES) m_mode = TB_SM_BRUSHES_ENTITIES;
            else m_mode = TB_SM_BRUSHES;

            SelectionEventData data(brushes);
            selectionAdded(data);
        }

        void Selection::addEntity(Entity& entity) {
            if (m_mode == TB_SM_FACES) removeAll();

            m_entities.push_back(&entity);
            entity.setSelected(true);

            if (m_mode == TB_SM_BRUSHES) m_mode = TB_SM_BRUSHES_ENTITIES;
            else m_mode = TB_SM_ENTITIES;

            SelectionEventData data(entity);
            selectionAdded(data);
        }

        void Selection::addEntities(const EntityList& entities) {
            if (entities.empty()) return;
            if (m_mode == TB_SM_FACES) removeAll();

            for (unsigned int i = 0; i < entities.size(); i++) {
                Entity* entity = entities[i];
                m_entities.push_back(entity);
                entity->setSelected(true);
            }

            if (m_mode == TB_SM_BRUSHES) m_mode = TB_SM_BRUSHES_ENTITIES;
            else m_mode = TB_SM_ENTITIES;

            SelectionEventData data(entities);
            selectionAdded(data);
        }

        void Selection::removeFace(Face& face) {
            FaceList::iterator it = find(m_faces.begin(), m_faces.end(), &face);
            if (it == m_faces.end()) return;

            m_faces.erase(it);
            face.selected = false;

            if (m_faces.size() == 0) {
                m_mode = TB_SM_NONE;
                m_partialBrushes.clear();
            } else {
                const FaceList siblings = face.brush->faces;
                face.brush->partiallySelected = false;
                for (unsigned int i = 0; i < siblings.size() && !face.brush->partiallySelected; i++)
                    face.brush->partiallySelected = siblings[i]->selected;
                if (!face.brush->partiallySelected)
                    m_partialBrushes.erase(find(m_partialBrushes.begin(), m_partialBrushes.end(), face.brush));
            }

            SelectionEventData data(face);
            selectionRemoved(data);
        }

        void Selection::removeFaces(const FaceList& faces) {
            if (faces.empty()) return;

            FaceList removedFaces;
            for (unsigned int i = 0; i < faces.size(); i++) {
                Face* face = faces[i];
                FaceList::iterator it = find(m_faces.begin(), m_faces.end(), face);
                if (it != m_faces.end()) {
                    m_faces.erase(it);
                    face->selected = false;
                    removedFaces.push_back(face);

                    const FaceList siblings = face->brush->faces;
                    face->brush->partiallySelected = false;
                    for (unsigned int j = 0; j < siblings.size() && !face->brush->partiallySelected; j++)
                        face->brush->partiallySelected = siblings[j]->selected;
                    if (!face->brush->partiallySelected)
                        m_partialBrushes.erase(find(m_partialBrushes.begin(), m_partialBrushes.end(), face->brush));
                }
            }

            if (m_faces.size() == 0)
                m_mode = TB_SM_NONE;

            SelectionEventData data(faces);
            selectionRemoved(data);
        }

        void Selection::removeBrush(Brush& brush) {
            BrushList::iterator it = find(m_brushes.begin(), m_brushes.end(), &brush);
            if (it == m_brushes.end()) return;

            m_brushes.erase(it);
            brush.selected = false;

            if (m_brushes.empty()) {
                if (m_entities.empty()) m_mode = TB_SM_NONE;
                else m_mode = TB_SM_ENTITIES;
            }

            SelectionEventData data(brush);
            selectionRemoved(data);
        }

        void Selection::removeBrushes(const BrushList& brushes) {
            if (brushes.empty()) return;

            BrushList removedBrushes;
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Brush* brush = brushes[i];
                BrushList::iterator it = find(m_brushes.begin(), m_brushes.end(), brush);
                if (it != m_brushes.end()) {
                    m_brushes.erase(it);
                    brush->selected = false;
                    removedBrushes.push_back(brush);
                }
            }

            if (m_brushes.empty()) {
                if (m_entities.empty()) m_mode = TB_SM_NONE;
                else m_mode = TB_SM_ENTITIES;
            }

            SelectionEventData data(removedBrushes);
            selectionRemoved(data);
        }

        void Selection::removeEntity(Entity& entity) {
            EntityList::iterator it = find(m_entities.begin(), m_entities.end(), &entity);
            if (it == m_entities.end()) return;

            m_entities.erase(it);
            entity.setSelected(false);

            if (m_entities.empty()) {
                if (m_brushes.empty()) m_mode = TB_SM_NONE;
                else m_mode = TB_SM_BRUSHES;
            }

            SelectionEventData data(entity);
            selectionRemoved(data);
        }

        void Selection::removeEntities(const EntityList& entities) {
            if (entities.empty()) return;

            EntityList removedEntities;
            for (unsigned int i = 0; i < entities.size(); i++) {
                Entity* entity = entities[i];
                EntityList::iterator it = find(m_entities.begin(), m_entities.end(), entity);
                if (it != m_entities.end()) {
                    m_entities.erase(it);
                    entity->setSelected(false);
                    removedEntities.push_back(entity);
                }
            }

            if (m_entities.empty()) {
                if (m_brushes.empty()) m_mode = TB_SM_NONE;
                else m_mode = TB_SM_BRUSHES;
            }

            SelectionEventData data(removedEntities);
            selectionRemoved(data);
        }

        void Selection::removeAll() {
            if (m_faces.empty() && m_brushes.empty() && m_entities.empty()) return;

            SelectionEventData data;

            if (!m_faces.empty()) {
                data.faces = m_faces;
                for (unsigned int i = 0; i < m_faces.size(); i++)
                    m_faces[i]->selected = false;
                m_faces.clear();
                for (unsigned int i = 0; i < m_partialBrushes.size(); i++)
                    m_partialBrushes[i]->partiallySelected = false;
                m_partialBrushes.clear();
                m_mode = TB_SM_NONE;
            }

            if (!m_brushes.empty()) {
                data.brushes = m_brushes;
                for (unsigned int i = 0; i < m_brushes.size(); i++)
                    m_brushes[i]->selected = false;
                m_brushes.clear();
                m_mode = TB_SM_NONE;
            }

            if (!m_entities.empty()) {
                data.entities = m_entities;
                for (unsigned int i = 0; i < m_entities.size(); i++)
                    m_entities[i]->setSelected(false);
                m_entities.clear();
                m_mode = TB_SM_NONE;
            }

            selectionRemoved(data);
        }
    }
}
