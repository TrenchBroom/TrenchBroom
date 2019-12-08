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

#ifndef VariableStore_h
#define VariableStore_h

#include "EL/Value.h" // required by VariableTable::Table declaration

#include <map>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace EL {
        class VariableStore {
        public:
            VariableStore() = default;
            VariableStore(const VariableStore& other) = default;
            VariableStore(VariableStore&& other) noexcept = default;
            virtual ~VariableStore() = default;

            VariableStore& operator=(const VariableStore& other) = default;
            VariableStore& operator=(VariableStore&& other) noexcept = default;

            VariableStore* clone() const;
            size_t size() const;
            Value value(const std::string& name) const;
            const std::vector<std::string> names() const;
            void declare(const std::string& name, const Value& value);
            void assign(const std::string& name, const Value& value);
        private:
            virtual VariableStore* doClone() const = 0;
            virtual size_t doGetSize() const = 0;
            virtual Value doGetValue(const std::string& name) const = 0;
            virtual std::vector<std::string> doGetNames() const = 0;
            virtual void doDeclare(const std::string& name, const Value& value) = 0;
            virtual void doAssign(const std::string& name, const Value& value) = 0;
        };

        class VariableTable : public VariableStore {
        private:
            using Table = std::map<std::string, Value>;
            Table m_variables;
        public:
            VariableTable();
            explicit VariableTable(const Table& variables);
        private:
            VariableStore* doClone() const override;
            size_t doGetSize() const override;
            Value doGetValue(const std::string& name) const override;
            std::vector<std::string> doGetNames() const override;
            void doDeclare(const std::string& name, const Value& value) override;
            void doAssign(const std::string& name, const Value& value) override;
        };

        class NullVariableStore : public VariableStore {
        public:
            NullVariableStore();
        private:
            VariableStore* doClone() const override;
            size_t doGetSize() const override;
            Value doGetValue(const std::string& name) const override;
            std::vector<std::string> doGetNames() const override;
            void doDeclare(const std::string& name, const Value& value) override;
            void doAssign(const std::string& name, const Value& value) override;
        };
    }
}

#endif /* VariableStore_h */
