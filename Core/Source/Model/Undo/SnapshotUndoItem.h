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


#ifndef TrenchBroom_SnapshotUndoItem_h
#define TrenchBroom_SnapshotUndoItem_h

#include "Model/Undo/UndoItem.h"
#include <vector>
#include <map>
#include <string>

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            class Texture;
        }
        
        class Map;
        class Entity;
        class Brush;
        class Face;
        
        class EntitySnapshot {
        private:
            int m_uniqueId;
            std::map<std::string, std::string> m_properties;
        public:
            EntitySnapshot(const Entity& entity);
            int uniqueId();
            void restore(Entity& entity);
        };
        
        class BrushSnapshot {
        private:
            int m_uniqueId;
            std::vector<Face*> m_faces;
        public:
            BrushSnapshot(const Brush& brush);
            ~BrushSnapshot();
            int uniqueId();
            void restore(Brush& brush);
        };
        
        class FaceSnapshot {
        private:
            int m_faceId;
            float m_xOffset;
            float m_yOffset;
            float m_xScale;
            float m_yScale;
            float m_rotation;
            Assets::Texture* m_texture;
        public:
            FaceSnapshot(const Face& face);
            int faceId();
            void restore(Face& face);
        };
        
        class SnapshotUndoItem : public UndoItem {
        protected:
            std::vector<EntitySnapshot*> m_entities;
            std::vector<BrushSnapshot*> m_brushes;
            std::vector<FaceSnapshot*> m_faces;
        public:
            SnapshotUndoItem(Map& map);
            virtual ~SnapshotUndoItem();
            virtual void performUndo();
        };
    }
}

#endif
