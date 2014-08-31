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
        m_name(name) {}
        
        const String& Layer::name() const {
            return m_name;
        }
        
        void Layer::setName(const String& name) {
            m_name = name;
        }
        
        const Model::ObjectList& Layer::objects() const {
            return m_objects;
        }
        
        void Layer::addObjects(const Model::ObjectList& objects) {
            Model::ObjectList::const_iterator it, end;
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Model::Object* object = *it;
                addObject(object);
            }
        }
        
        void Layer::addObject(Model::Object* object) {
            object->setLayer(this);
            m_objects.push_back(object);
        }

        void Layer::removeObjects(const Model::ObjectList& objects) {
            Model::ObjectList::const_iterator it, end;
            Model::ObjectList::iterator er = m_objects.end();
            
            for (it = objects.begin(), end = objects.end(); it != end; ++it) {
                Model::Object* object = *it;
                object->setLayer(NULL);
                er = std::remove(m_objects.begin(), er, object);
            }
            
            m_objects.erase(er, m_objects.end());
        }
    }
}
