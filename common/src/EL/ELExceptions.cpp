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
        Exception::Exception() throw() {}
        Exception::Exception(const String& str) throw() : ExceptionStream(str) {}
        Exception::~Exception() throw() {}
        
        ConversionError::ConversionError(const String& value, const ValueType from, const ValueType to) throw() :
        Exception("Cannot convert value '" + value + "' of type '" + typeName(from) + "' to type '" + typeName(to) + "'") {}
        
        DereferenceError::DereferenceError(const String& value, const ValueType from, const ValueType to) throw() :
        Exception("Cannot dereference value '" + value + "' of type '" + typeName(from) + "' as type '" + typeName(to) + "'") {}
        
        EvaluationError::EvaluationError(const String& msg) throw() :
        Exception(msg) {}
        
        IndexError::IndexError(const Value& indexableValue, const Value& indexValue) throw() :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + " with '" + indexValue.describe() + "' of type '" + typeName(indexValue.type()) + "'") {}
        
        IndexError::IndexError(const Value& indexableValue, const size_t index) throw() :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + " with integral index") {}
        
        IndexError::IndexError(const Value& indexableValue, const String& key) throw() :
        EvaluationError("Cannot index value '" + indexableValue.describe() + "' of type '" + indexableValue.typeName() + " with string index") {}
        
        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, const size_t outOfBoundsIndex) throw() :
        IndexError(indexableValue, indexValue) {
            *this << ": Index value " << outOfBoundsIndex << " is out of bounds";
        }
        
        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, const String& outOfBoundsIndex) throw() :
        IndexError(indexableValue, indexValue) {
            *this << ": Key '" << outOfBoundsIndex << "' not found";
        }
        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const size_t index) throw() :
        IndexError(indexableValue, index) {
            *this << ": Index value " << index << " is out of bounds";
        }
        IndexOutOfBoundsError::IndexOutOfBoundsError(const Value& indexableValue, const String& key) throw() :
        IndexError(indexableValue, key) {
            *this << ": Key '" << key << "' not found";
        }
    }
}
