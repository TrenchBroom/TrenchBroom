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

#include "StringUtils.h"
#include "EL/Value.h"

namespace TrenchBroom {
    namespace EL {
        class VariableStore {
        public:
            VariableStore();
            VariableStore(const VariableStore &other);
            virtual ~VariableStore();
            
            VariableStore* clone() const;
            Value value(const String& name) const;
            const StringSet names() const;
            void declare(const String& name, const Value& value = Value::Undefined);
            void assign(const String& name, const Value& value);
        private:
            virtual VariableStore* doClone() const = 0;
            virtual Value doGetValue(const String& name) const = 0;
            virtual StringSet doGetNames() const = 0;
            virtual void doDeclare(const String& name, const Value& value) = 0;
            virtual void doAssign(const String& name, const Value& value) = 0;
        };
        
        class VariableTable : public VariableStore {
        private:
            using Table = std::map<String, Value>;
            Table m_variables;
        public:
            VariableTable();
            VariableTable(const Table& variables);
        private:
            VariableStore* doClone() const override;
            Value doGetValue(const String& name) const override;
            StringSet doGetNames() const override;
            void doDeclare(const String& name, const Value& value) override;
            void doAssign(const String& name, const Value& value) override;
        };
        
        class NullVariableStore : public VariableStore {
        public:
            NullVariableStore();
        private:
            VariableStore* doClone() const override;
            Value doGetValue(const String& name) const override;
            StringSet doGetNames() const override;
            void doDeclare(const String& name, const Value& value) override;
            void doAssign(const String& name, const Value& value) override;
        };
    }
}

#endif /* VariableStore_h */
