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

#ifndef __TrenchBroom__PartiallySelectable__
#define __TrenchBroom__PartiallySelectable__

#include <iostream>

namespace TrenchBroom {
    namespace Model {
        class PartiallySelectable {
        private:
            size_t m_childSelectionCount;
        public:
            PartiallySelectable();
            virtual ~PartiallySelectable();
            bool partiallySelected() const;
            
            void childSelected();
            void childDeselected();
        protected:
            void incChildSelectionCount();
            void decChildSelectionCount();
        };
    }
}

#endif /* defined(__TrenchBroom__PartiallySelectable__) */
