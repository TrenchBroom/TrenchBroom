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

#ifndef EntityAttributesVariableStore_h
#define EntityAttributesVariableStore_h

#include "Macros.h"
#include "EL/VariableStore.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Entity;

        class EntityAttributesVariableStore : public EL::VariableStore {
        private:
            const Entity& m_entity;
        public:
            explicit EntityAttributesVariableStore(const Entity& entity);
        private:
            VariableStore* doClone() const override;
            size_t doGetSize() const override;
            EL::Value doGetValue(const std::string& name) const override;
            std::vector<std::string> doGetNames() const override;
            void doDeclare(const std::string& name, const EL::Value& value) override;
            void doAssign(const std::string& name, const EL::Value& value) override;

            deleteCopyAndMove(EntityAttributesVariableStore)
        };
    }
}

#endif /* EntityAttributesVariableStore_h */
