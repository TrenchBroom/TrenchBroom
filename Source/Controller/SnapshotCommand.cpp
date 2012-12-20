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

#include "SnapshotCommand.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Face.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        
        EntitySnapshot::EntitySnapshot(const Model::Entity& entity) {
            m_uniqueId = entity.uniqueId();
            m_properties = entity.properties();
        }
        
        unsigned int EntitySnapshot::uniqueId() {
            return m_uniqueId;
        }
        
        void EntitySnapshot::restore(Model::Entity& entity) {
            entity.setProperties(m_properties, true);
        }
        
        BrushSnapshot::BrushSnapshot(const Model::Brush& brush) {
            m_uniqueId = brush.uniqueId();
            const Model::FaceList& brushFaces = brush.faces();
            for (unsigned int i = 0; i < brushFaces.size(); i++) {
                Model::Face* snapshot = new Model::Face(*brushFaces[i]);
                m_faces.push_back(snapshot);
            }
        }
        
        BrushSnapshot::~BrushSnapshot() {
            // must not delete the face snapshots because they are now in use by the original brush!
        }
        
        unsigned int BrushSnapshot::uniqueId() {
            return m_uniqueId;
        }
        
        void BrushSnapshot::restore(Model::Brush& brush) {
            brush.replaceFaces(m_faces);
        }
        
        FaceSnapshot::FaceSnapshot(const Model::Face& face) {
            m_faceId = face.faceId();
            m_xOffset = face.xOffset();
            m_yOffset = face.yOffset();
            m_xScale = face.xScale();
            m_yScale = face.yScale();
            m_rotation = face.rotation();
            m_texture = face.texture();
            m_textureName = face.textureName();
        }
        
        unsigned int FaceSnapshot::faceId() {
            return m_faceId;
        }
        
        void FaceSnapshot::restore(Model::Face& face) {
            face.setXOffset(m_xOffset);
            face.setYOffset(m_yOffset);
            face.setRotation(m_rotation);
            face.setXScale(m_xScale);
            face.setYScale(m_yScale);
            face.setTexture(m_texture);
            if (m_texture == NULL)
                face.setTextureName(m_textureName);
        }
        
        void SnapshotCommand::makeSnapshots(const Model::EntityList& entities) {
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity& entity = *entities[i];
                m_entities[entity.uniqueId()] = new EntitySnapshot(entity);
            }
        }
        
        void SnapshotCommand::makeSnapshots(const Model::BrushList& brushes) {
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Model::Brush& brush = *brushes[i];
                m_brushes[brush.uniqueId()] = new BrushSnapshot(brush);
            }
        }
        
        void SnapshotCommand::makeSnapshots(const Model::FaceList& faces) {
            for (unsigned int i = 0; i < faces.size(); i++) {
                Model::Face& face = *faces[i];
                m_faces[face.faceId()] = new FaceSnapshot(face);
            }
        }
        
        void SnapshotCommand::restoreSnapshots(const Model::EntityList& entities) {
            assert(m_entities.size() == entities.size());
            
            if (entities.empty())
                return;
            
            for (unsigned int i = 0; i < entities.size(); i++) {
                Model::Entity& entity = *entities[i];
                EntitySnapshot& snapshot = *m_entities[entity.uniqueId()];
                snapshot.restore(entity);
            }
        }
        
        void SnapshotCommand::restoreSnapshots(const Model::BrushList& brushes) {
            assert(m_brushes.size() == brushes.size());
            
            if (brushes.empty())
                return;
            
            for (unsigned int i = 0; i < brushes.size(); i++) {
                Model::Brush& brush = *brushes[i];
                BrushSnapshot& snapshot = *m_brushes[brush.uniqueId()];
                snapshot.restore(brush);
            }
        }
        
        void SnapshotCommand::restoreSnapshots(const Model::FaceList& faces) {
            assert(m_faces.size() == faces.size());
            
            if (faces.empty())
                return;
            
            for (unsigned int i = 0; i < faces.size(); i++) {
                Model::Face& face = *faces[i];
                FaceSnapshot& snapshot = *m_faces[face.faceId()];
                snapshot.restore(face);
            }
        }

        void SnapshotCommand::clear() {
            {
                EntitySnapshotMap::iterator it, end;
                for (it = m_entities.begin(), end = m_entities.end(); it != end; ++it)
                    delete it->second;
                m_entities.clear();
            }
            {
                BrushSnapshotMap::iterator it, end;
                for (it = m_brushes.begin(), end = m_brushes.end(); it != end; ++it)
                    delete it->second;
                m_brushes.clear();
            }
            {
                FaceSnapshotMap::iterator it, end;
                for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it)
                    delete it->second;
                m_faces.clear();
            }
        }
        
        SnapshotCommand::SnapshotCommand(Command::Type type, Model::MapDocument& document, const wxString& name) :
        DocumentCommand(type, document, true, name, true) {}
        
        SnapshotCommand::~SnapshotCommand() {
            clear();
        }
    }
}
