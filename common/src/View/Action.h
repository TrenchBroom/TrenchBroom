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

#ifndef TrenchBroom_Action
#define TrenchBroom_Action

#include "Preference.h"
#include "StringUtils.h"

#include <vector>

namespace TrenchBroom {
    namespace View {
        class Action {
        public:
            typedef std::vector<Action> List;
        private:
            int m_id;
            String m_name;
            bool m_modifiable;
        public:
            Action(); // default constructor required to initialize arrays of Action instances
            Action(int id, const String& name, bool modifiable);
            
            int id() const;
            const String& name() const;
            
            bool modifiable() const;
        };
    }
}

#endif /* defined(TrenchBroom_Action) */
