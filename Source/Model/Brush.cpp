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

#include "Brush.h"

#include "Model/Entity.h"
#include "Model/Face.h"
#include "Model/Texture.h"

namespace TrenchBroom {
    namespace Model {
        void Brush::init() {
            m_entity = NULL;
            m_filePosition = 0;
            m_editState = EditState::Default;
            m_selectedFaceCount = 0;
            m_geometry = new BrushGeometry(m_worldBounds);
        }

        void Brush::validateGeometry() {
            FaceList droppedFaces;
            
            delete m_geometry;
            m_geometry = new BrushGeometry(m_worldBounds);
            m_geometry->addFaces(m_faces, droppedFaces);
            for (unsigned int i = 0; i < droppedFaces.size(); i++) {
                Face* droppedFace = droppedFaces[i];
                FaceList::iterator it = find(m_faces.begin(), m_faces.end(), droppedFace);
                delete *it;
                m_faces.erase(it);
            }
        }

        Brush::Brush(const BBox& worldBounds) : MapObject(), m_worldBounds(worldBounds) {
            init();
        }
        
        Brush::Brush(const BBox& worldBounds, const Brush& brushTemplate) : MapObject(), m_worldBounds(worldBounds) {
            init();
            restore(brushTemplate, false);
        }
        
        Brush::Brush(const BBox& worldBounds, const BBox& brushBounds, Texture* texture) : MapObject(), m_worldBounds(worldBounds) {
            init();
            
            Vec3f p1, p2, p3;
            String textureName = texture != NULL ? texture->name() : "";
            
            p1 = brushBounds.min;
            p2 = p1;
            p2.z = brushBounds.max.z;
            p3 = p1;
            p3.x = brushBounds.max.x;
            Face* front = new Face(worldBounds, p1, p2, p3, textureName);
            front->setTexture(texture);
            addFace(front);
            
            p2 = p1;
            p2.y = brushBounds.max.y;
            p3 = p1;
            p3.z = brushBounds.max.z;
            Face* left = new Face(worldBounds, p1, p2, p3, textureName);
            left->setTexture(texture);
            addFace(left);
            
            p2 = p1;
            p2.x = brushBounds.max.x;
            p3 = p1;
            p3.y = brushBounds.max.y;
            Face* bottom = new Face(worldBounds, p1, p2, p3, textureName);
            bottom->setTexture(texture);
            addFace(bottom);
            
            p1 = brushBounds.max;
            p2 = p1;
            p2.x = brushBounds.min.x;
            p3 = p1;
            p3.z = brushBounds.min.z;
            Face* back = new Face(worldBounds, p1, p2, p3, textureName);
            back->setTexture(texture);
            addFace(back);
            
            p2 = p1;
            p2.z = brushBounds.min.z;
            p3 = p1;
            p3.y = brushBounds.min.y;
            Face* right = new Face(worldBounds, p1, p2, p3, textureName);
            right->setTexture(texture);
            addFace(right);
            
            p2 = p1;
            p2.y = brushBounds.min.y;
            p3 = p1;
            p3.x = brushBounds.min.x;
            Face* top = new Face(worldBounds, p1, p2, p3, textureName);
            top->setTexture(texture);
            addFace(top);
        }

        Brush::~Brush() {
            setEntity(NULL);
            delete m_geometry;
            m_geometry = NULL;
            while (!m_faces.empty()) delete m_faces.back(), m_faces.pop_back();
            m_editState = EditState::Default;
        }

        void Brush::restore(const Brush& brushTemplate, bool checkId) {
            if (checkId)
                assert(uniqueId() == brushTemplate.uniqueId());
            
            while(!m_faces.empty()) delete m_faces.back(), m_faces.pop_back();
            if (m_geometry != NULL)
                delete m_geometry;
            m_geometry = new BrushGeometry(m_worldBounds);
            
            const FaceList templateFaces = brushTemplate.faces();
            for (unsigned int i = 0; i < templateFaces.size(); i++) {
                Face* face = new Face(m_worldBounds, *templateFaces[i]);
                addFace(face);
            }
            
            if (m_entity != NULL)
                m_entity->invalidateGeometry();
        }

        void Brush::setEntity(Entity* entity) {
            if (entity == m_entity)
                return;
            
            if (m_entity != NULL && m_editState == EditState::Selected)
                m_entity->decSelectedBrushCount();
            
            m_entity = entity;
            if (m_entity != NULL && m_editState == EditState::Selected)
                m_entity->incSelectedBrushCount();
        }

        bool Brush::addFace(Face* face) {
            try {
                FaceList droppedFaces;
                CutResult result = m_geometry->addFace(*face, droppedFaces);
                
                if (result == CutResult::Redundant) {
                    delete face;
                    return true;
                }
                
                if (result == CutResult::Null) {
                    delete face;
                    return false;
                }
                
                for (unsigned int i = 0; i < droppedFaces.size(); i++) {
                    Face* droppedFace = droppedFaces[i];
                    FaceList::iterator it = find(m_faces.begin(), m_faces.end(), droppedFace);
                    delete *it;
                    m_faces.erase(it);
                }
                
                face->setBrush(this);
                m_faces.push_back(face);
                m_entity->invalidateGeometry();
                
                return true;
            } catch (GeometryException e) {
                delete face;
                return false;
            }
        }
        
        void Brush::pick(const Ray& ray, PickResult& pickResults, Filter& filter) const {
        }
    }
}