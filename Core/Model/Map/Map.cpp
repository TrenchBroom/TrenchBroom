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

#include "Map.h"
#include <algorithm>
#include <fstream>
#include <cassert>
#include "Utils.h"

namespace TrenchBroom {
    namespace Model {
        Map::Map(const BBox& worldBounds, const string& entityDefinitionFilePath) : Observable(), m_worldBounds(worldBounds) {
            m_octree = new Octree(*this, 256);
            m_selection = new Selection();
            m_entityDefinitionManager = EntityDefinitionManager::sharedManager(entityDefinitionFilePath);
            m_groupManager = new GroupManager(*this);
        }
        
        Map::~Map() {
            setPostNotifications(false);
            clear();
            delete m_octree;
            delete m_selection;
            delete m_groupManager;
        }
        
        void Map::save(const string& path) {
        }

        void Map::clear() {
            m_selection->removeAll();
            unloadPointFile();
            while(!m_entities.empty()) delete m_entities.back(), m_entities.pop_back();
            m_worldspawn = NULL;
            postNotification(MapCleared, NULL);
        }
        
# pragma mark Point File Support
        
        void Map::loadPointFile(const string& path) {
            if (!m_leakPoints.empty()) unloadPointFile();
            
            string line;
            ifstream stream(path.c_str());
            assert(stream.is_open());
            
            while (!stream.eof()) {
                getline(stream, line);
                line = trim(line);
                if (line.length() > 0) {
                    Vec3f point = Vec3f(line);
                    m_leakPoints.push_back(point);
                }
            }
            
            postNotification(PointFileLoaded, NULL);
        }
        
        void Map::unloadPointFile() {
            m_leakPoints.clear();
            postNotification(PointFileUnloaded, NULL);
            
        }
        
        const vector<Vec3f>& Map::leakPoints() const {
            return m_leakPoints;
        }
        
# pragma mark Entity related functions
        vector<Entity*>& Map::entities() {
            return m_entities;
        }
        
        Entity* Map::worldspawn(bool create) {
            if (m_worldspawn != NULL)
                return m_worldspawn;
            for (int i = 0; i < m_entities.size(); i++) {
                Entity* entity = m_entities[i];
                if (entity->worldspawn()) {
                    m_worldspawn = entity;
                    return m_worldspawn;
                }
            }
            
            if (create)
                m_worldspawn = createEntity(WorldspawnClassname);
            return m_worldspawn;
        }
        
        void Map::addEntity(Entity* entity) {
            assert(entity != NULL);
            if (!entity->worldspawn() || worldspawn(false) == NULL) {
                m_entities.push_back(entity);
                entity->setMap(this);
            }
            
            vector <Entity*> entities;
            entities.push_back(entity);
            postNotification(EntitiesAdded, &entities);
        }
        
        Entity* Map::createEntity(const string& classname) {
            EntityDefinition* entityDefinition = m_entityDefinitionManager->definition(classname);
            if (entityDefinition == NULL) {
                fprintf(stdout, "Warning: No entity definition found for class name '%s'", classname.c_str());
                return NULL;
            }
            
            Entity* entity = new Entity();
            entity->setProperty(ClassnameKey, classname);
            entity->setEntityDefinition(entityDefinition);
            addEntity(entity);
            return entity;
        }
        
        Entity* Map::createEntity(const map<string, string> properties) {
            map<string, string>::const_iterator it = properties.find(ClassnameKey);
            assert(it != properties.end());
            
            string classname = it->second;
            EntityDefinition* entityDefinition = m_entityDefinitionManager->definition(classname);
            if (entityDefinition == NULL) {
                fprintf(stdout, "Warning: No entity definition found for class name '%s'", classname.c_str());
                return NULL;
            }
            
            Entity* entity = new Entity(properties);
            entity->setEntityDefinition(entityDefinition);
            addEntity(entity);
            return entity;
        }
        
        void Map::setEntityDefinition(Entity* entity) {
            const string* classname = entity->classname();
            if (classname != NULL) {
                EntityDefinition* entityDefinition = m_entityDefinitionManager->definition(*classname);
                if (entityDefinition != NULL)
                    entity->setEntityDefinition(entityDefinition);
                else
                    fprintf(stdout, "Warning: No entity definition found for class name '%s'", classname->c_str());
            } else {
                fprintf(stdout, "Warning: Entity with id %i is missing classname property (line %i)", entity->uniqueId(), entity->filePosition());
            }
        }
        
        void Map::setEntityProperty(const string& key, const string* value) {
            const vector<Entity*>& entities = m_selection->entities();
            if (entities.empty()) return;
            
            vector<Entity*> changedEntities;
            for (int i = 0; i < entities.size(); i++) {
                Entity* entity = entities[i];
                const string* oldValue = entity->propertyForKey(key);
                if (oldValue != value) changedEntities.push_back(entity);
            }
            
            if (!changedEntities.empty()) {
                postNotification(PropertiesWillChange, &changedEntities);
                for (int i = 0; i < changedEntities.size(); i++) {
                    if (value == NULL) entities[i]->deleteProperty(key);
                    else entities[i]->setProperty(key, value);
                }
                postNotification(PropertiesDidChange, &changedEntities);
            }
        }
        
# pragma mark Brush related functions
        
        void Map::addBrushesToEntity(Entity& entity) {
            const vector<Brush*>& brushes = m_selection->brushes();
            if (brushes.empty()) return;
            
            entity.addBrushes(brushes);
            postNotification(BrushesAdded, &brushes);
        }
        
        void Map::moveBrushesToEntity(Entity& entity) {
            const vector<Brush*> brushes = m_selection->brushes();
            if (brushes.empty()) return;
            
            postNotification(BrushesWillChange, &brushes);
            entity.addBrushes(brushes);
            postNotification(BrushesDidChange, &brushes);
        }
        
        Brush* Map::createBrush(Entity& entity, const Brush& brushTemplate) {
            BBox templateBounds = brushTemplate.bounds();
            if (!m_worldBounds.contains(brushTemplate.bounds())) return NULL;
            
            Brush* brush = new Brush(m_worldBounds, brushTemplate);
            m_selection->removeAll();
            m_selection->addBrush(*brush);
            addBrushesToEntity(entity);
            return brush;
        }
        
        Brush* Map::createBrush(Entity& entity, BBox bounds, Assets::Texture& texture) {
            if (!m_worldBounds.contains(bounds)) return NULL;
            
            Brush* brush = new Brush(m_worldBounds, bounds, texture);
            m_selection->removeAll();
            m_selection->addBrush(*brush);
            addBrushesToEntity(entity);
            return brush;
        }
        
        void Map::snapBrushes() {
            const vector<Brush*>& brushes = m_selection->brushes();
            if (brushes.empty()) return;
            
            postNotification(BrushesWillChange, &brushes);
            for (int i = 0; i < brushes.size(); i++)
                brushes[i]->snap();
            postNotification(BrushesDidChange, &brushes);
        }
        
        bool Map::resizeBrushes(vector<Face*>& faces, float delta, bool lockTextures) {
            if (faces.empty()) return false;
            if (delta == 0) return false;
            
            bool drag = true;
            vector<Brush*> changedBrushes;
            for (int i = 0; i < faces.size() && drag; i++) {
                Face* face = faces[i];
                Brush* brush = face->brush();
                drag &= brush->selected() && brush->canResize(*face, delta);
                changedBrushes.push_back(brush);
            }
            
            if (drag) {
                postNotification(BrushesWillChange, &changedBrushes);
                for (int i = 0; i < faces.size(); i++) {
                    Face* face = faces[i];
                    Brush* brush = face->brush();
                    brush->resize(*face, delta, lockTextures);
                }
                postNotification(BrushesDidChange, &changedBrushes);
            }
            
            return drag;
        }
        
# pragma mark Common functions
        
        void Map::duplicateObjects(vector<Entity*>& newEntities, vector<Brush*>& newBrushes) {
            const vector<Entity*>& entities = m_selection->entities();
            const vector<Brush*>& brushes = m_selection->brushes();
            
            if (!entities.empty()) {
                for (int i = 0; i < entities.size(); i++) {
                    Entity* entity = entities[i];
                    Entity* newEntity = new Entity(entity->properties());
                    
                    EntityDefinition* entityDefinition = m_entityDefinitionManager->definition(*newEntity->classname());
                    assert(entityDefinition != NULL);
                    newEntity->setEntityDefinition(entityDefinition);
                    
                    newEntities.push_back(newEntity);
                    m_entities.push_back(newEntity);
                    
                    for (int i = 0; i < entity->brushes().size(); i++) {
                        Brush* newBrush = new Brush(m_worldBounds, *entity->brushes()[i]);
                        newBrushes.push_back(newBrush);
                        newEntity->addBrush(newBrush);
                    }
                }
            }
            
            if (!brushes.empty()) {
                for (int i = 0; i < brushes.size(); i++) {
                    Brush* newBrush = new Brush(m_worldBounds, *brushes[i]);
                    newBrushes.push_back(newBrush);
                    worldspawn(true)->addBrush(newBrush);
                }
            }
            
            if (!newEntities.empty())
                postNotification(EntitiesAdded, &newEntities);
            if (!newBrushes.empty())
                postNotification(BrushesAdded, &newBrushes);
        }
        
        void Map::translateObjects(Vec3f delta, bool lockTextures) {
            const vector<Entity*>& entities = m_selection->entities();
            const vector<Brush*>& brushes = m_selection->brushes();
            
            if (!entities.empty()) {
                postNotification(PropertiesWillChange, &entities);
                for (int i = 0; i < entities.size(); i++)
                    entities[i]->translate(delta);
                postNotification(PropertiesDidChange, &entities);
            }
            
            if (!brushes.empty()) {
                postNotification(BrushesWillChange, &brushes);
                for (int i = 0; i < brushes.size(); i++)
                    brushes[i]->translate(delta, lockTextures);
                postNotification(BrushesDidChange, &brushes);
            }
        }
        
        void Map::rotateObjects90CW(EAxis axis, Vec3f center, bool lockTextures) {
            const vector<Entity*>& entities = m_selection->entities();
            const vector<Brush*>& brushes = m_selection->brushes();
            
            if (!entities.empty()) {
                postNotification(PropertiesWillChange, &entities);
                for (int i = 0; i < entities.size(); i++)
                    entities[i]->rotate90CW(axis, center);
                postNotification(PropertiesDidChange, &entities);
            }
            
            if (!brushes.empty()) {
                postNotification(BrushesWillChange, &brushes);
                for (int i = 0; i < brushes.size(); i++)
                    brushes[i]->rotate90CW(axis, center, lockTextures);
                postNotification(BrushesDidChange, &brushes);
            }
        }
        
        void Map::rotateObjects90CCW(EAxis axis, Vec3f center, bool lockTextures) {
            const vector<Entity*>& entities = m_selection->entities();
            const vector<Brush*>& brushes = m_selection->brushes();
            
            if (!entities.empty()) {
                postNotification(PropertiesWillChange, &entities);
                for (int i = 0; i < entities.size(); i++)
                    entities[i]->rotate90CCW(axis, center);
                postNotification(PropertiesDidChange, &entities);
            }
            
            if (!brushes.empty()) {
                postNotification(BrushesWillChange, &brushes);
                for (int i = 0; i < brushes.size(); i++)
                    brushes[i]->rotate90CCW(axis, center, lockTextures);
                postNotification(BrushesDidChange, &brushes);
            }
        }
        
        void Map::rotateObjects(Quat rotation, Vec3f center, bool lockTextures) {
            const vector<Entity*>& entities = m_selection->entities();
            const vector<Brush*>& brushes = m_selection->brushes();
            
            if (!entities.empty()) {
                postNotification(PropertiesWillChange, &entities);
                for (int i = 0; i < entities.size(); i++)
                    entities[i]->rotate(rotation, center);
                postNotification(PropertiesDidChange, &entities);
            }
            
            if (!brushes.empty()) {
                postNotification(BrushesWillChange, &brushes);
                for (int i = 0; i < brushes.size(); i++)
                    brushes[i]->rotate(rotation, center, lockTextures);
                postNotification(BrushesDidChange, &brushes);
            }
        }
        
        void Map::flipObjects(EAxis axis, Vec3f center, bool lockTextures) {
            const vector<Entity*>& entities = m_selection->entities();
            const vector<Brush*>& brushes = m_selection->brushes();
            
            if (!entities.empty()) {
                postNotification(PropertiesWillChange, &entities);
                for (int i = 0; i < entities.size(); i++)
                    entities[i]->flip(axis, center);
                postNotification(PropertiesDidChange, &entities);
            }
            
            if (!brushes.empty()) {
                postNotification(BrushesWillChange, &brushes);
                for (int i = 0; i < brushes.size(); i++)
                    brushes[i]->flip(axis, center, lockTextures);
                postNotification(BrushesDidChange, &brushes);
            }
        }
        
        void Map::deleteObjects() {
            const vector<Entity*>& entities = m_selection->entities();
            const vector<Brush*>& brushes = m_selection->brushes();
            
            vector<Entity*> removedEntities;
            if (!brushes.empty()) {
                vector<Brush*> removedBrushes = brushes;
                postNotification(BrushesWillBeRemoved, &removedBrushes);
                m_selection->removeBrushes(removedBrushes);
                for (int i = 0; i < removedBrushes.size(); i++) {
                    Brush* brush = removedBrushes[i];
                    Entity* entity = brush->entity();
                    entity->removeBrush(brush);
                    delete brush;
                    
                    if (entity->brushes().empty() && !entity->worldspawn())
                        removedEntities.push_back(entity);
                }
            }
            
            if (!removedEntities.empty() || !entities.empty()) {
                for (int i = 0; i < entities.size(); i++) {
                    Entity* entity = entities[i];
                    if (!entity->worldspawn()) {
                        worldspawn(true)->addBrushes(entity->brushes());
                        if (find(removedEntities.begin(), removedEntities.end(), entity) == removedEntities.end())
                            removedEntities.push_back(entity);
                    }
                }
                
                postNotification(EntitiesWillBeRemoved, &removedEntities);
                m_selection->removeEntities(removedEntities);
                for (int i = 0; i < removedEntities.size(); i++) {
                    remove(m_entities.begin(), m_entities.end(), removedEntities[i]);
                    delete removedEntities[i];
                }
            }
        }
        
# pragma mark Face related functoins
        void Map::setXOffset(int xOffset) {
            const vector<Face*>& faces = m_selection->faces();
            if (faces.empty()) return;
            
            postNotification(FacesWillChange, &faces);
            for (int i = 0; i < faces.size(); i++)
                faces[i]->setXOffset(xOffset);
            postNotification(FacesDidChange, &faces);
        }
        
        void Map::setYOffset(int yOffset) {
            const vector<Face*>& faces = m_selection->faces();
            if (faces.empty()) return;
            
            postNotification(FacesWillChange, &faces);
            for (int i = 0; i < faces.size(); i++)
                faces[i]->setYOffset(yOffset);
            postNotification(FacesDidChange, &faces);
        }
        
        void Map::translateFaces(float delta, Vec3f dir) {
            const vector<Face*>& faces = m_selection->faces();
            if (faces.empty()) return;
            
            postNotification(FacesWillChange, &faces);
            for (int i = 0; i < faces.size(); i++)
                faces[i]->translateOffsets(delta, dir);
            postNotification(FacesDidChange, &faces);
        }
        
        void Map::setRotation(float rotation) {
            const vector<Face*>& faces = m_selection->faces();
            if (faces.empty()) return;
            
            postNotification(FacesWillChange, &faces);
            for (int i = 0; i < faces.size(); i++)
                faces[i]->setRotation(rotation);
            postNotification(FacesDidChange, &faces);
        }
        
        void Map::rotateFaces(float angle) {
            const vector<Face*>& faces = m_selection->faces();
            if (faces.empty()) return;
            
            postNotification(FacesWillChange, &faces);
            for (int i = 0; i < faces.size(); i++)
                faces[i]->rotateTexture(angle);
            postNotification(FacesDidChange, &faces);
        }
        
        void Map::setXScale(float xScale) {
            const vector<Face*>& faces = m_selection->faces();
            if (faces.empty()) return;
            
            postNotification(FacesWillChange, &faces);
            for (int i = 0; i < faces.size(); i++)
                faces[i]->setXScale(xScale);
            postNotification(FacesDidChange, &faces);
        }
        
        void Map::setYScale(float yScale) {
            const vector<Face*>& faces = m_selection->faces();
            if (faces.empty()) return;
            
            postNotification(FacesWillChange, &faces);
            for (int i = 0; i < faces.size(); i++)
                faces[i]->setYScale(yScale);
            postNotification(FacesDidChange, &faces);
        }
        
        bool Map::deleteFaces() {
            const vector<Face*> faces = m_selection->faces();
            if (faces.empty()) return false;
            
            vector<Brush*> changedBrushes;
            bool del = true;
            for (int i = 0; i < faces.size() && del; i++) {
                Face* face = faces[i];
                Brush* brush = face->brush();
                del &= brush->canDeleteFace(*face);
                changedBrushes.push_back(brush);
            }
            
            if (del) {
                m_selection->removeAll();
                m_selection->addBrushes(changedBrushes);
                postNotification(BrushesWillChange, &changedBrushes);
                for (int i = 0; i < faces.size() && del; i++) {
                    Face* face = faces[i];
                    Brush* brush = face->brush();
                    brush->deleteFace(*face);
                }
                postNotification(BrushesDidChange, &changedBrushes);
            }
            
            return del;
        }
        
# pragma mark Vertex related functions
        MoveResult Map::moveVertex(Brush& brush, int vertexIndex, Vec3f delta) {
            if (find(m_selection->brushes().begin(), m_selection->brushes().end(), &brush) == m_selection->brushes().end())
                m_selection->addBrush(brush);
            vector<Brush*> brushArray;
            brushArray.push_back(&brush);
            postNotification(BrushesWillChange, &brushArray);
            MoveResult result = brush.moveVertex(vertexIndex, delta);
            postNotification(BrushesDidChange, &brushArray);
            return result;
        }
        
        MoveResult Map::moveEdge(Brush& brush, int edgeIndex, Vec3f delta) {
            if (find(m_selection->brushes().begin(), m_selection->brushes().end(), &brush) == m_selection->brushes().end())
                m_selection->addBrush(brush);
            vector<Brush*> brushArray;
            brushArray.push_back(&brush);
            postNotification(BrushesWillChange, &brushArray);
            MoveResult result = brush.moveEdge(edgeIndex, delta);
            postNotification(BrushesDidChange, &brushArray);
            return result;
        }
        
        MoveResult Map::moveFace(Brush& brush, int faceIndex, Vec3f delta) {
            if (find(m_selection->brushes().begin(), m_selection->brushes().end(), &brush) == m_selection->brushes().end())
                m_selection->addBrush(brush);
            vector<Brush*> brushArray;
            brushArray.push_back(&brush);
            postNotification(BrushesWillChange, &brushArray);
            MoveResult result = brush.moveFace(faceIndex, delta);
            postNotification(BrushesDidChange, &brushArray);
            return result;
        }
        
# pragma mark getters
        BBox Map::worldBounds() {
            return m_worldBounds;
        }
        
        Octree& Map::octree() {
            return *m_octree;
        }
        
        Selection& Map::selection() {
            return *m_selection;
        }
        
        EntityDefinitionManager& Map::entityDefinitionManager() {
            return *m_entityDefinitionManager;
        }
        
        GroupManager& Map::groupManager() {
            return *m_groupManager;
        }
    }
}