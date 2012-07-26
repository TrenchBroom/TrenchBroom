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

#ifndef TrenchBroom_Map_h
#define TrenchBroom_Map_h

#include <vector>
#include "Utilities/Event.h"
#include "Utilities/VecMath.h"
#include "Model/Map/BrushTypes.h"
#include "Model/Map/EntityTypes.h"
#include "Model/Map/EntityDefinition.h"
#include "Model/Map/FaceTypes.h"

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            class Texture;
        }
        
        class Octree;
        class Picker;
        class Selection;
        class EntityDefinitionManager;
        class GroupManager;
        class UndoManager;
        class MoveResult;
        class Entity;
        class Brush;
        class Face;
        
        class Map {
        private:
            Octree* m_octree;
            Picker* m_picker;
            Selection* m_selection;
            EntityDefinitionManagerPtr m_entityDefinitionManager;
            GroupManager* m_groupManager;
            UndoManager* m_undoManager;
            
            EntityList m_entities;
            Entity* m_worldspawn;
            BBox m_worldBounds;
            
            std::vector<Vec3f> m_leakPoints;
            std::vector<std::string> m_mods;
            
            bool m_postNotifications;
        public:
            typedef Event<const EntityList&> EntityEvent;
            typedef Event<const BrushList&> BrushEvent;
            typedef Event<const FaceList&> FaceEvent;
            typedef Event<Map&> MapEvent;
            typedef Event<Map&> PointFileEvent;
            EntityEvent entitiesWereAdded;
            EntityEvent entitiesWillBeRemoved;
            EntityEvent propertiesWillChange;
            EntityEvent propertiesDidChange;
            BrushEvent brushesWereAdded;
            BrushEvent brushesWillBeRemoved;
            BrushEvent brushesWillChange;
            BrushEvent brushesDidChange;
            FaceEvent facesWillChange;
            FaceEvent facesDidChange;
            MapEvent mapLoaded;
            MapEvent mapCleared;
            PointFileEvent pointFileLoaded;
            PointFileEvent pointFileUnloaded;
            void setPostNotifications(bool postNotifications);
            
            Map(const BBox& worldBounds, const std::string& entityDefinitionFilePath);
            ~Map();
            
            void clear();
            
            void loadPointFile(const std::string& path);
            void unloadPointFile();
            const std::vector<Vec3f>& leakPoints() const;
            
            const EntityList& entities();
            Entity* worldspawn(bool create);
            void addEntity(Entity* entity);
            void addEntities(const EntityList& entities);
            Entity* createEntity(const std::string& classname);
            Entity* createEntity(const Properties properties);
            void setEntityDefinition(Entity* entity);
            void setEntityProperty(const PropertyKey& key, const PropertyValue* value);
            void setEntityProperty(const PropertyKey& key, const PropertyValue& value);
            void setEntityProperty(const PropertyKey& key, const Vec3f& value, bool round);
            void setEntityProperty(const PropertyKey& key, int value);
            void setEntityProperty(const PropertyKey& key, float value, bool round);
            void renameEntityProperty(const PropertyKey& oldKey, const PropertyKey& newKey);
            void removeEntityProperty(const PropertyKey& key);
            
            void moveBrushesToEntity(Entity& entity);
            Brush* createBrush(Entity& entity, Brush& brushTemplate);
            void snapBrushes();
            bool resizeBrushes(const FaceList& faces, float delta, bool lockTextures);
            
            void duplicateObjects(EntityList& newEntities, BrushList& newBrushes);
            void translateObjects(const Vec3f delta, bool lockTextures);
            void rotateObjects90(EAxis axis, const Vec3f center, bool clockwise, bool lockTextures);
            void rotateObjects(const Quat& rotation, const Vec3f& center, bool lockTextures);
            void flipObjects(EAxis axis, const Vec3f center, bool lockTextures);
            void deleteObjects();
            void restoreObjects(EntityList removedEntities, BrushParentMap removedBrushes, BrushParentMap movedBrushes);
            
            void setTexture(Model::Assets::Texture* texture);
            void setXOffset(int xOffset);
            void setYOffset(int yOffset);
            void translateFaces(float delta, const Vec3f dir);
            void setRotation(float rotation);
            void rotateFaces(float angle);
            void setXScale(float xScale);
            void setYScale(float yScale);
            void resetFaces();
            bool deleteFaces();
            
            MoveResult moveVertex(Brush& brush, size_t vertexIndex, const Vec3f& delta);
            MoveResult moveEdge(Brush& brush, size_t edgeIndex, const Vec3f& delta);
            MoveResult moveFace(Brush& brush, size_t faceIndex, const Vec3f& delta);
            
            const BBox& worldBounds();
            Octree& octree();
            Picker& picker();
            Selection& selection();
            EntityDefinitionManager& entityDefinitionManager();
            GroupManager& groupManager();
            UndoManager& undoManager();
            const std::vector<std::string>& mods();
        };
    }
}

#endif
