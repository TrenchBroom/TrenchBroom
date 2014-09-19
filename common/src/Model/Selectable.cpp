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

#include "Selectable.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        Selectable::Selectable() :
        m_selected(false) {}
        
        Selectable::~Selectable() {}
        
        bool Selectable::selected() const {
            return m_selected;
        }
        
        void Selectable::select() {
            assert(!m_selected);
            m_selected = true;
            doWasSelected();
        }
        
        void Selectable::deselect() {
            assert(m_selected);
            m_selected = false;
            doWasDeselected();
        }
    }
}
