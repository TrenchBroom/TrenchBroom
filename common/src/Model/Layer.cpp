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

#include "Layer.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Object.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Model {
        Layer::Layer(const String& name) :
        m_name(name),
        m_visible(true),
        m_locked(false) {}
        
        const String& Layer::name() const {
            return m_name;
        }
        
        void Layer::setName(const String& name) {
            if (name == m_name)
                return;
            layerWillChangeNotifier(this, Attr_Name);
            m_name = name;
            layerDidChangeNotifier(this, Attr_Name);
        }
        
        bool Layer::visible() const {
            return m_visible;
        }
        
        void Layer::setVisible(const bool visible) {
            if (visible == m_visible)
                return;
            layerWillChangeNotifier(this, Attr_Visible);
            m_visible = visible;
            layerDidChangeNotifier(this, Attr_Visible);
        }
        
        bool Layer::locked() const {
            return m_locked;
        }
        
        void Layer::setLocked(const bool locked) {
            if (locked == m_locked)
                return;
            layerWillChangeNotifier(this, Attr_Locked);
            m_locked = locked;
            layerDidChangeNotifier(this, Attr_Locked);
        }
        
        const ObjectList& Layer::objects() const {
            return m_objects;
        }

        const EntityList& Layer::entities() const {
            return m_entities;
        }
        
        const BrushList& Layer::worldBrushes() const {
            return m_worldBrushes;
        }

        void Layer::addEntity(Entity* entity) {
            assert(entity->layer() == this);
            assert(!VectorUtils::contains(m_objects, entity));
            assert(!VectorUtils::contains(m_entities, entity));
            
            layerWillChangeNotifier(this, Attr_Objects);
            m_objects.push_back(entity);
            if (!entity->worldspawn())
                m_entities.push_back(entity);
            objectWasAddedNotifier(this, entity);
            layerDidChangeNotifier(this, Attr_Objects);
        }
        
        void Layer::addBrush(Brush* brush) {
            assert(brush->layer() == this);
            assert(!VectorUtils::contains(m_objects, brush));
            assert(!VectorUtils::contains(m_worldBrushes, brush));
            
            Model::Entity* entity = brush->parent();
            assert(entity != NULL);

            layerWillChangeNotifier(this, Attr_Objects);
            m_objects.push_back(brush);
            if (entity->worldspawn())
                m_worldBrushes.push_back(brush);
            objectWasAddedNotifier(this, brush);
            layerDidChangeNotifier(this, Attr_Objects);
        }
        
        void Layer::removeEntity(Entity* entity) {
            assert(entity->layer() == this);
            assert(VectorUtils::contains(m_objects, entity));
            
            layerDidChangeNotifier(this, Attr_Objects);
            VectorUtils::erase(m_objects, entity);
            if (!entity->worldspawn()) {
                assert(VectorUtils::contains(m_entities, entity));
                VectorUtils::erase(m_entities, entity);
            }
            objectWasRemovedNotifier(this, entity);
            layerDidChangeNotifier(this, Attr_Objects);
        }
        
        void Layer::removeBrush(Brush* brush) {
            assert(brush->layer() == this);
            assert(VectorUtils::contains(m_objects, brush));
            
            layerDidChangeNotifier(this, Attr_Objects);
            VectorUtils::erase(m_objects, brush);
            Model::Entity* entity = brush->parent();
            assert(entity != NULL);
            if (entity->worldspawn()) {
                assert(VectorUtils::contains(m_worldBrushes, brush));
                VectorUtils::erase(m_worldBrushes, brush);
            }
            objectWasRemovedNotifier(this, brush);
            layerDidChangeNotifier(this, Attr_Objects);
        }
    }
}
