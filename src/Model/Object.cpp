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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Object.h"

namespace TrenchBroom {
    namespace Model {
        Object::~Object() {}

        Object::Type Object::type() const {
            return m_type;
        }

        void Object::setFilePosition(const size_t lineNumber, const size_t lineCount) {
            m_lineNumber = lineNumber;
            m_lineCount = lineCount;
        }

        bool Object::selected() const {
            return m_selected;
        }
        
        bool Object::select() {
            if (m_selected)
                return false;
            m_selected = true;
            return true;
        }
        
        bool Object::deselect() {
            if (!m_selected)
                return false;
            m_selected = false;
            return true;
        }

        bool Object::partiallySelected() const {
            return m_childSelectionCount > 0;
        }
        
        size_t Object::childSelectionCount() const {
            return m_childSelectionCount;
        }

        void Object::incChildSelectionCount() {
            ++m_childSelectionCount;
        }
        
        void Object::decChildSelectionCount() {
            --m_childSelectionCount;
        }

        Object::Object(const Type type) :
        m_type(type),
        m_lineNumber(0),
        m_lineCount(0),
        m_selected(false),
        m_childSelectionCount(0) {}
    }
}
