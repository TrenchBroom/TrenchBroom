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
            m_name = name;
            layerDidChangeNotifier(this);
        }
        
        bool Layer::visible() const {
            return m_visible;
        }
        
        void Layer::setVisible(const bool visible) {
            if (visible == m_visible)
                return;
            m_visible = visible;
            layerDidChangeNotifier(this);
        }
        
        bool Layer::locked() const {
            return m_locked;
        }
        
        void Layer::setLocked(const bool locked) {
            if (locked == m_locked)
                return;
            m_locked = locked;
            layerDidChangeNotifier(this);
        }
        
        const Model::ObjectList& Layer::objects() const {
            return m_objects;
        }

        void Layer::addObject(Object* object) {
            assert(object->layer() == this);
            assert(!VectorUtils::contains(m_objects, object));
            
            layerWillChangeNotifier(this);
            m_objects.push_back(object);
            layerDidChangeNotifier(this);
        }
        
        void Layer::removeObject(Object* object) {
            assert(object->layer() == this);
            assert(VectorUtils::contains(m_objects, object));

            layerWillChangeNotifier(this);
            VectorUtils::erase(m_objects, object);
            layerDidChangeNotifier(this);
        }
    }
}
