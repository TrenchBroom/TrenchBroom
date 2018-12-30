/*
 Copyright (C) 2010-2017 Kristian Duske

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

#ifndef TRENCHBROOM_TYPEDATTRIBUTEMAP_H
#define TRENCHBROOM_TYPEDATTRIBUTEMAP_H

#include "StringUtils.h"

#include <any>
#include <unordered_map>

namespace TrenchBroom  {
    class TypedAttributeMap {
    public:
        template <typename T>
        class Attribute {
        private:
            String m_name;
            T m_defaultValue;
        public:
            Attribute(const String& name, const T& defaultValue) :
            m_name(name),
            m_defaultValue(defaultValue) {}

            const String& name() const {
                return m_name;
            }

            const T& defaultValue() const {
                return m_defaultValue;
            }
        };
    private:
        std::unordered_map<String, std::any> m_attributes;
    public:
        template <typename T>
        bool hasAttribute(const Attribute<T>& attribute) const {
            return m_attributes.find(attribute.name()) != std::end(m_attributes);
        }

        template <typename T>
        T getAttribute(const Attribute<T>& attribute) const {
            const auto it = m_attributes.find(attribute.name());
            if (it == std::end(m_attributes)) {
                return attribute.defaultValue();
            } else {
                return std::any_cast<T>(it->second);
            }
        }

        template <typename T>
        void setAttribute(const Attribute<T>& attribute, const T& value) {
            m_attributes[attribute.name()] = value;
        }

        void setAttributes(const TypedAttributeMap& attributes) {
            for (const auto& pair : attributes.m_attributes) {
                m_attributes[pair.first] = pair.second;
            }
        }
    };
}


#endif //TRENCHBROOM_TYPEDATTRIBUTEMAP_H
