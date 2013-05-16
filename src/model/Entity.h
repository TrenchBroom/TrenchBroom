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

#include "Model/BrushTypes.h"
#include "Model/EntityProperties.h"
#include "Model/EntityTypes.h"
#include "Model/Object.h"

namespace TrenchBroom {
    namespace Model {
        class Entity : public Object {
        private:
            EntityProperties m_properties;
            BrushList m_brushes;
            
            Entity();
        public:
            static EntityPtr newEntity();
            
            const EntityPropertyList& properties() const;
            const PropertyValue& property(const PropertyKey& key, const PropertyValue& defaultValue) const;
            void addOrUpdateProperty(const PropertyKey& key, const PropertyValue& value);
            
            const PropertyValue& classname(const PropertyValue& defaultClassname = PropertyValues::NoClassname) const;
            
            inline const BrushList& brushes() const {
                return m_brushes;
            }
            
            void addBrush(BrushPtr brush);
            void removeBrush(BrushPtr brush);
        };
    }
}

#endif /* defined(__TrenchBroom__Entity__) */
