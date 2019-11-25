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

#ifndef ELExceptions_h
#define ELExceptions_h

#include "Exceptions.h"
#include "StringType.h"

namespace TrenchBroom {
    namespace EL {
        class Value;
        enum class ValueType;

        class Exception : public ::Exception {
        public:
            using ::Exception::Exception;
        };

        class ConversionError : public Exception {
        public:
            ConversionError(const String& value, ValueType from, ValueType to);
        };

        class DereferenceError : public Exception {
        public:
            DereferenceError(const String& value, ValueType from, ValueType to);
        };

        class EvaluationError : public Exception {
        public:
            using Exception::Exception;
        };

        class IndexError : public EvaluationError {
        public:
            IndexError(const Value& indexableValue, const Value& indexValue);
            IndexError(const Value& indexableValue, size_t index);
            IndexError(const Value& indexableValue, const String& key);
        };

        class IndexOutOfBoundsError : public EvaluationError {
        public:
            IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, size_t outOfBoundsIndex);
            IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, const String& outOfBoundsIndex);
            IndexOutOfBoundsError(const Value& indexableValue, size_t index);
            IndexOutOfBoundsError(const Value& indexableValue, const String& key);
        };
    }
}

#endif /* ELExceptions_h */
