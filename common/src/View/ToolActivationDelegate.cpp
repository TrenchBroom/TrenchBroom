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

#include "ToolActivationDelegate.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        ToolActivationDelegate::ToolActivationDelegate(const bool initiallyActive) :
        m_active(initiallyActive) {}
        
        ToolActivationDelegate::~ToolActivationDelegate() {}
        
        bool ToolActivationDelegate::active() const {
            return m_active;
        }
        
        bool ToolActivationDelegate::activate() {
            assert(!active());
            m_active = doActivate();
            return m_active;
        }
        
        bool ToolActivationDelegate::deactivate() {
            assert(active());
            m_active = !doDeactivate();
            return m_active;
        }
    }
}
