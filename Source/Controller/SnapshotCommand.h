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

#ifndef __TrenchBroom__SnapshotCommand__
#define __TrenchBroom__SnapshotCommand__

#include "Controller/Command.h"

#include "Model/BrushTypes.h"
#include "Model/EntityTypes.h"
#include "Model/FaceTypes.h"
#include "Utility/String.h"


namespace TrenchBroom {
    namespace Model {
        class Brush;
        class Entity;
        class Face;
        class Texture;
    }
    
    namespace Controller {
        class EntitySnapshot {
        private:
            unsigned int m_uniqueId;
            Model::Properties m_properties;
        public:
            EntitySnapshot(const Model::Entity& entity);
            unsigned int uniqueId();
            void restore(Model::Entity& entity);
        };
        
        class BrushSnapshot {
        private:
            unsigned int m_uniqueId;
            Model::FaceList m_faces;
        public:
            BrushSnapshot(const Model::Brush& brush);
            ~BrushSnapshot();
            unsigned int uniqueId();
            void restore(Model::Brush& brush);
        };
        
        class FaceSnapshot {
        private:
            unsigned int m_faceId;
            float m_xOffset;
            float m_yOffset;
            float m_xScale;
            float m_yScale;
            float m_rotation;
            Model::Texture* m_texture;
            String m_textureName;
        public:
            FaceSnapshot(const Model::Face& face);
            unsigned int faceId();
            void restore(Model::Face& face);
        };
        
        class SnapshotCommand : public DocumentCommand {
        private:
            typedef std::map<unsigned int, EntitySnapshot*> EntitySnapshotMap;
            typedef std::map<unsigned int, BrushSnapshot*> BrushSnapshotMap;
            typedef std::map<unsigned int, FaceSnapshot*> FaceSnapshotMap;
            
            EntitySnapshotMap m_entities;
            BrushSnapshotMap m_brushes;
            FaceSnapshotMap m_faces;
        protected:
            void makeSnapshots(const Model::EntityList& entities);
            void makeSnapshots(const Model::BrushList& brushes);
            void makeSnapshots(const Model::FaceList& faces);
            void restoreSnapshots(const Model::EntityList& entities);
            void restoreSnapshots(const Model::BrushList& brushes);
            void restoreSnapshots(const Model::FaceList& faces);
            void clear();
        public:
            SnapshotCommand(Command::Type type, Model::MapDocument& document, const wxString& name);
            virtual ~SnapshotCommand();
        };
    }
}

#endif /* defined(__TrenchBroom__SnapshotCommand__) */
