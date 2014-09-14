/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ObjectSection.h"

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Object.h"

namespace TrenchBroom {
    namespace Model {
        ObjectSection::~ObjectSection() {}
        
        const ObjectList& ObjectSection::objects() const {
            return m_objects;
        }
        
        const EntityList& ObjectSection::entities() const {
            return m_entities;
        }
        
        const BrushList& ObjectSection::worldBrushes() const {
            return m_worldBrushes;
        }
        
        void ObjectSection::addEntity(Entity* entity) {
            assert(!VectorUtils::contains(m_objects, entity));
            assert(!VectorUtils::contains(m_entities, entity));

            entityWillBeAdded(entity);
            m_objects.push_back(entity);
            if (!entity->worldspawn())
                m_entities.push_back(entity);
            entityWasAdded(entity);
        }
        
        void ObjectSection::addBrush(Brush* brush) {
            assert(!VectorUtils::contains(m_objects, brush));
            assert(!VectorUtils::contains(m_worldBrushes, brush));
            
            Model::Entity* entity = brush->entity();
            assert(entity != NULL);

            brushWillBeAdded(brush);
            m_objects.push_back(brush);
            if (entity->worldspawn())
                m_worldBrushes.push_back(brush);
            brushWasAdded(brush);
        }
        
        void ObjectSection::removeEntity(Entity* entity) {
            assert(VectorUtils::contains(m_objects, entity));

            entityWillBeRemoved(entity);
            VectorUtils::erase(m_objects, entity);
            if (!entity->worldspawn()) {
                assert(VectorUtils::contains(m_entities, entity));
                VectorUtils::erase(m_entities, entity);
            }
            entityWasRemoved(entity);
        }
        
        void ObjectSection::removeBrush(Brush* brush) {
            assert(VectorUtils::contains(m_objects, brush));

            brushWillBeRemoved(brush);
            VectorUtils::erase(m_objects, brush);
            Model::Entity* entity = brush->entity();
            assert(entity != NULL);
            if (entity->worldspawn()) {
                assert(VectorUtils::contains(m_worldBrushes, brush));
                VectorUtils::erase(m_worldBrushes, brush);
            }
            brushWasRemoved(brush);
        }

        void ObjectSection::entityWillBeAdded(Entity* entity) {}
        void ObjectSection::entityWasAdded(Entity* entity) {}
        void ObjectSection::entityWillBeRemoved(Entity* entity) {}
        void ObjectSection::entityWasRemoved(Entity* entity) {}
        void ObjectSection::brushWillBeAdded(Brush* brush) {}
        void ObjectSection::brushWasAdded(Brush* brush) {}
        void ObjectSection::brushWillBeRemoved(Brush* brush) {}
        void ObjectSection::brushWasRemoved(Brush* brush) {}
    }
}
