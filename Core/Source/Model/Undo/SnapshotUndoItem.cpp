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


#include "SnapshotUndoItem.h"
#include "Model/Undo/UndoManager.h"
#include "Model/Map/Map.h"
#include "Model/Map/Entity.h"
#include "Model/Map/Brush.h"
#include "Model/Map/Face.h"
#include "Model/Selection.h"
#include <cassert>

namespace TrenchBroom {
    namespace Model {
        EntitySnapshot::EntitySnapshot(const Entity& entity) {
            m_uniqueId = entity.uniqueId();
            m_properties = entity.properties();
        }
        
        int EntitySnapshot::uniqueId() {
            return m_uniqueId;
        }
        
        void EntitySnapshot::restore(Entity& entity) {
            entity.setProperties(m_properties, true);
        }

        BrushSnapshot::BrushSnapshot(const Brush& brush) {
            m_uniqueId = brush.uniqueId();
            const FaceList& brushFaces = brush.faces;
            for (unsigned int i = 0; i < brushFaces.size(); i++) {
                Face* snapshot = new Face(*brushFaces[i]);
                m_faces.push_back(snapshot);
            }
        }
        
        BrushSnapshot::~BrushSnapshot() {
            // must not delete the face snapshots because they are now in use by the original brush!
        }
        
        int BrushSnapshot::uniqueId() {
            return m_uniqueId;
        }
        
        void BrushSnapshot::restore(Brush& brush) {
            brush.replaceFaces(m_faces);
        }

        FaceSnapshot::FaceSnapshot(const Face& face) {
            m_faceId = face.faceId;
            m_xOffset = face.xOffset;
            m_yOffset = face.yOffset;
            m_xScale = face.xScale;
            m_yScale = face.yScale;
            m_rotation = face.rotation;
            m_texture = face.texture;
            m_textureName = face.textureName;
        }
        
        int FaceSnapshot::faceId() {
            return m_faceId;
        }
        
        void FaceSnapshot::restore(Face& face) {
            face.setXOffset(m_xOffset);
            face.setYOffset(m_yOffset);
            face.setRotation(m_rotation);
            face.setXScale(m_xScale);
            face.setYScale(m_yScale);
            face.setTexture(m_texture);
            if (m_texture == NULL)
                face.textureName = m_textureName;
        }

        SnapshotUndoItem::SnapshotUndoItem(Map& map) : SelectionUndoItem(map) {
            Selection& selection = m_map.selection();
            const std::vector<Entity*>& entities = selection.entities();
            const BrushList& brushes = selection.brushes();
            const FaceList& faces = selection.faces();
            
            for (unsigned int i = 0; i < entities.size(); i++) {
                EntitySnapshot* snapshot = new EntitySnapshot(*entities[i]);
                m_entities.push_back(snapshot);
            }
            
            for (unsigned int i = 0; i < brushes.size(); i++) {
                BrushSnapshot* snapshot = new BrushSnapshot(*brushes[i]);
                m_brushes.push_back(snapshot);
            }
            
            for (unsigned int i = 0; i < faces.size(); i++) {
                FaceSnapshot* snapshot = new FaceSnapshot(*faces[i]);
                m_faces.push_back(snapshot);
            }
        }
        
        SnapshotUndoItem::~SnapshotUndoItem() {
            while (!m_faces.empty()) delete m_faces.back(), m_faces.pop_back();
            while (!m_brushes.empty()) delete m_brushes.back(), m_brushes.pop_back();
            while (!m_entities.empty()) delete m_entities.back(), m_entities.pop_back();
        }
        
        void SnapshotUndoItem::performUndo() {
            Selection& selection = m_map.selection();
            const std::vector<Entity*>& selectedEntities = selection.entities();
            const BrushList& selectedBrushes = selection.brushes();
            const FaceList& selectedFaces = selection.faces();

            assert(m_entities.size() == selectedEntities.size());
            assert(m_brushes.size() == selectedBrushes.size());
            assert(m_faces.size() == selectedFaces.size());
            
            UndoManager& undoManager = m_map.undoManager();
            SnapshotUndoItem* redoItem = new SnapshotUndoItem(m_map);
            undoManager.addItem(redoItem);
            
            if (!m_faces.empty()) {
                m_map.facesWillChange(selectedFaces);

                for (unsigned int i = 0; i < m_faces.size(); i++) {
                    FaceSnapshot* snapshot = m_faces[i];
                    Face* original = selectedFaces[i];
                    assert(snapshot->faceId() == original->faceId);
                    snapshot->restore(*original);
                }
                
                m_map.facesDidChange(selectedFaces);
            }
            
            if (!m_brushes.empty()) {
                m_map.brushesWillChange(selectedBrushes);
                
                for (unsigned int i = 0; i < m_brushes.size(); i++) {
                    BrushSnapshot* snapshot = m_brushes[i];
                    Brush* original = selectedBrushes[i];
                    assert(snapshot->uniqueId() == original->uniqueId());
                    snapshot->restore(*original);
                }
                
                m_map.brushesDidChange(selectedBrushes);
            }
            
            if (!m_entities.empty()) {
                m_map.propertiesWillChange(selectedEntities);
                
                for (unsigned int i = 0; i < m_entities.size(); i++) {
                    EntitySnapshot* snapshot = m_entities[i];
                    Entity* original = selectedEntities[i];
                    assert(snapshot->uniqueId() == original->uniqueId());
                    snapshot->restore(*original);
                }
                
                m_map.propertiesDidChange(selectedEntities);
            }
        }
    }
}