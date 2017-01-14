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

#include "ELExceptions.h"

#include "EL/Types.h"
#include "EL/Value.h"

namespace TrenchBroom {
    namespace EL {
        Exception::Exception() noexcept {}
        Exception::Exception(const String& str) noexcept : ExceptionStream(str) {}
        
        ConversionError::ConversionError(const String& value, const ValueType from, const ValueType to) noexcept :
        Exception("Cannot convert value '" + value + "' of type '" + typeName(from) + "' to type '" + typeName(to) + "'") {}
        
        DereferenceError::DereferenceError(const String& value, const ValueType from, const ValueType to) noexcept :
        Exception("Cannot dereference value '" + value + "' of type '" + typeName(from) + "' as type '" + typeName(to) + "'") {}
        
        EvaluationError::EvaluationError(const String& msg) noexcept :
        Exception(msg) {}
        
        IndexError::IndexError(const Value& indexableValue, const Value& indexValue) noexcept :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + "' using index '" + indexValue.describe() + "' of type '" + typeName(indexValue.type()) + "'") {}
        
        IndexError::IndexError(const Value& indexableValue, const size_t index) noexcept :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + "' using integral index") {}
        
        IndexError::IndexError(const Value& indexableValue, const String& key) noexcept :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + "' using string index") {}
        
        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, const size_t outOfBoundsIndex) noexcept :
        IndexError(indexableValue, indexValue) {
            *this << ": Index value " << outOfBoundsIndex << " is out of bounds";
        }
        
        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, const String& outOfBoundsIndex) noexcept :
        IndexError(indexableValue, indexValue) {
            *this << ": Key '" << outOfBoundsIndex << "' not found";
        }
        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const size_t index) noexcept :
        IndexError(indexableValue, index) {
            *this << ": Index value " << index << " is out of bounds";
        }
        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const String& key) noexcept :
        IndexError(indexableValue, key) {
            *this << ": Key '" << key << "' not found";
        }
    }
}
