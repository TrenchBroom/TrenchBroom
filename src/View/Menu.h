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

#ifndef __TrenchBroom__Menu__
#define __TrenchBroom__Menu__

#include "SharedPointer.h"

#include <vector>

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcut;
        class MenuItemParent;
        
        class MenuItem {
        public:
            typedef enum {
                MITSeparator,
                MITAction,
                MITCheck,
                MITMenu,
                MITMultiMenu
            } MenuItemType;
            
            typedef std::tr1::shared_ptr<MenuItem> Ptr;
            typedef std::vector<Ptr> List;
        private:
            MenuItemType m_type;
            MenuItemParent* m_parent;
        public:
            MenuItem(MenuItemType type, MenuItemParent* parent) :
            m_type(type),
            m_parent(parent) {}
            
            virtual ~MenuItem() {}
            
            inline MenuItemType type() const {
                return m_type;
            }
            
            inline const MenuItemParent* parent() const {
                return m_parent;
            }
            
            virtual const KeyboardShortcut* shortcutByKeys(const int key, const int modifierKey1, const int modifierKey2, const int modifierKey3) const {
                return NULL;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__Menu__) */
