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

#include "ELExceptions.h"

#include "EL/Types.h"
#include "EL/Value.h"

#include <string>

namespace TrenchBroom {
    namespace EL {
        ConversionError::ConversionError(const std::string& value, const ValueType from, const ValueType to) :
        Exception("Cannot convert value '" + value + "' of type '" + typeName(from) + "' to type '" + typeName(to) + "'") {}

        DereferenceError::DereferenceError(const std::string& value, const ValueType from, const ValueType to) :
        Exception("Cannot dereference value '" + value + "' of type '" + typeName(from) + "' as type '" + typeName(to) + "'") {}

        IndexError::IndexError(const Value& indexableValue, const Value& indexValue) :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + "' using index '" + indexValue.describe() + "' of type '" + typeName(indexValue.type()) + "'") {}

        IndexError::IndexError(const Value& indexableValue, const size_t /* index */) :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + "' using integral index") {}

        IndexError::IndexError(const Value& indexableValue, const std::string& /* key */) :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + "' using string index") {}

        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, const size_t outOfBoundsIndex) :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + "' using index '" + indexValue.describe() + "' of type '" + typeName(indexValue.type()) + "': Index value " + std::to_string(outOfBoundsIndex) + " is out of bounds") {}

        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, const std::string& outOfBoundsIndex) :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + "' using index '" + indexValue.describe() + "' of type '" + typeName(indexValue.type()) + "': Key '" + outOfBoundsIndex + "' not found") {}

        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const size_t index) :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + "' using integral index: Index value " + std::to_string(index) + " is out of bounds") {}

        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const std::string& key) :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + "' using string index: Key '" + key + "' not found") {}
    }
}
