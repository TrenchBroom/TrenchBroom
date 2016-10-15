/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#ifndef AttributableNodeVariableStore_h
#define AttributableNodeVariableStore_h

#include "Macros.h"
#include "EL.h"

namespace TrenchBroom {
    namespace Model {
        class AttributableNode;
        
        class AttributableNodeVariableStore : public EL::VariableStore {
        private:
            AttributableNode* m_node;
        public:
            AttributableNodeVariableStore(AttributableNode* node);
        private:
            VariableStore* doClone() const;
            EL::Value doGetValue(const String& name) const;
            StringSet doGetNames() const;
            void doDeclare(const String& name, const EL::Value& value);
            void doAssign(const String& name, const EL::Value& value);

            deleteCopyAndAssignment(AttributableNodeVariableStore)
        };
    }
}

#endif /* AttributableNodeVariableStore_h */
