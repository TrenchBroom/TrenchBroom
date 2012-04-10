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

#ifndef TrenchBroom_QuakeMap_h
#define TrenchBroom_QuakeMap_h

#include <vector>
#include "VecMath.h"
#include "Event.h"

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
        class MoveResult;
        class Entity;
        class Brush;
        class Face;
        
        class Map {
        private:
            Octree* m_octree;
            Picker* m_picker;
            Selection* m_selection;
            EntityDefinitionManager* m_entityDefinitionManager;
            GroupManager* m_groupManager;
            
            vector<Entity*> m_entities;
            Entity* m_worldspawn;
            BBox m_worldBounds;
            
            vector<Vec3f> m_leakPoints;
            
            bool m_postNotifications;
        public:
# pragma mark Events
            typedef Event<const vector<Entity*>&> EntityEvent;
            typedef Event<const vector<Brush*>&> BrushEvent;
            typedef Event<const vector<Face*>&> FaceEvent;
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
            
            Map(const BBox& worldBounds, const string& entityDefinitionFilePath);
            ~Map();
            
# pragma mark Saving and Clearing
            void save(const string& path);
            void clear();
            
# pragma mark Point File Support
            void loadPointFile(const string& path);
            void unloadPointFile();
            const vector<Vec3f>& leakPoints() const;
            
# pragma mark Entity related functions
            vector<Entity*>& entities();
            Entity* worldspawn(bool create);
            void addEntity(Entity* entity);
            Entity* createEntity(const string& classname);
            Entity* createEntity(const map<string, string> properties);
            void setEntityDefinition(Entity* entity);
            void setEntityProperty(const string& key, const string* value);
            
# pragma mark Brush related functions
            void addBrushesToEntity(Entity& entity);
            void moveBrushesToEntity(Entity& entity);
            Brush* createBrush(Entity& entity, const Brush& brushTemplate);
            Brush* createBrush(Entity& entity, BBox bounds, Assets::Texture& texture);
            void snapBrushes();
            bool resizeBrushes(vector<Face*>& faces, float delta, bool lockTextures);
            
# pragma mark Common functions
            void duplicateObjects(vector<Entity*>& newEntities, vector<Brush*>& newBrushes);
            void translateObjects(Vec3f delta, bool lockTextures);
            void rotateObjects90CW(EAxis axis, Vec3f center, bool lockTextures);
            void rotateObjects90CCW(EAxis axis, Vec3f center, bool lockTextures);
            void rotateObjects(Quat rotation, Vec3f center, bool lockTextures);
            void flipObjects(EAxis axis, Vec3f center, bool lockTextures);
            void deleteObjects();
            
# pragma mark Face related functoins
            void setXOffset(int xOffset);
            void setYOffset(int yOffset);
            void translateFaces(float delta, Vec3f dir);
            void setRotation(float rotation);
            void rotateFaces(float angle);
            void setXScale(float xScale);
            void setYScale(float yScale);
            bool deleteFaces();
            
# pragma mark Vertex related functions
            MoveResult moveVertex(Brush& brush, int vertexIndex, Vec3f delta);
            MoveResult moveEdge(Brush& brush, int edgeIndex, Vec3f delta);
            MoveResult moveFace(Brush& brush, int faceIndex, Vec3f delta);
            
# pragma mark Getters
            BBox worldBounds();
            Octree& octree();
            Picker& picker();
            Selection& selection();
            EntityDefinitionManager& entityDefinitionManager();
            GroupManager& groupManager();
        };
    }
}

#endif
