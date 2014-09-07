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

        void Layer::entityWillBeAdded(Entity* entity) {
            assert(entity->layer() == this);
            layerWillChangeNotifier(this, Attr_Objects);
        }
        
        void Layer::entityWasAdded(Entity* entity) {
            objectWasAddedNotifier(this, entity);
            layerDidChangeNotifier(this, Attr_Objects);
        }
        
        void Layer::entityWillBeRemoved(Entity* entity) {
            assert(entity->layer() == this);
            layerDidChangeNotifier(this, Attr_Objects);
        }
        
        void Layer::entityWasRemoved(Entity* entity) {
            objectWasRemovedNotifier(this, entity);
            layerDidChangeNotifier(this, Attr_Objects);
        }
        
        void Layer::brushWillBeAdded(Brush* brush) {
            assert(brush->layer() == this);
            layerWillChangeNotifier(this, Attr_Objects);
        }
        
        void Layer::brushWasAdded(Brush* brush) {
            objectWasAddedNotifier(this, brush);
            layerDidChangeNotifier(this, Attr_Objects);
        }
        
        void Layer::brushWillBeRemoved(Brush* brush) {
            assert(brush->layer() == this);
            layerDidChangeNotifier(this, Attr_Objects);
        }
        
        void Layer::brushWasRemoved(Brush* brush) {
            objectWasRemovedNotifier(this, brush);
            layerDidChangeNotifier(this, Attr_Objects);
        }
    }
}
