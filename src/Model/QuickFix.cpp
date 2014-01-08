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

#include "QuickFix.h"

namespace TrenchBroom {
    namespace Model {
        QuickFixType QuickFix::freeType() {
            static QuickFixType type = 0;
            return type++;
        }

        QuickFixType QuickFix::type() const {
            return m_type;
        }

        const String& QuickFix::description() const {
            return m_description;
        }
        
        QuickFix::QuickFix(const QuickFixType type, const String& description) :
        m_type(type),
        m_description(description) {}
    }
}
