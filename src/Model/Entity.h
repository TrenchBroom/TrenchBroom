/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__Entity__
#define __TrenchBroom__Entity__

#include "SharedPointer.h"
#include "Model/Brush.h"
#include "Model/EntityProperties.h"
#include "Model/Object.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Entity : public Object {
        public:
            typedef std::tr1::shared_ptr<Entity> Ptr;
            typedef std::vector<Entity::Ptr> List;
            static const List EmptyList;
        private:
            static const String DefaultPropertyValue;
            
            EntityProperties m_properties;
            Brush::List m_brushes;
            
            Entity();
        public:
            static Entity::Ptr newEntity();
            
            const EntityProperty::List& properties() const;
            bool hasProperty(const PropertyKey& key) const;
            const PropertyValue& property(const PropertyKey& key, const PropertyValue& defaultValue = DefaultPropertyValue) const;
            void addOrUpdateProperty(const PropertyKey& key, const PropertyValue& value);
            
            const PropertyValue& classname(const PropertyValue& defaultClassname = PropertyValues::NoClassname) const;
            
            const Brush::List& brushes() const;
            void addBrush(Brush::Ptr brush);
            void removeBrush(Brush::Ptr brush);

            template <class Operator, class Filter>
            inline void eachBrushFace(Operator& op, Filter& filter) {
                Brush::List::const_iterator it, end;
                for (it = m_brushes.begin(), end = m_brushes.end(); it != end; ++it) {
                    Brush::Ptr brush = *it;
                    if (filter(brush))
                        brush->eachBrushFace(op, filter);
                }
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Entity__) */
