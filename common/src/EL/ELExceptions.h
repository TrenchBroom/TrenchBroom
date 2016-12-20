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

#ifndef ELExceptions_h
#define ELExceptions_h

#include "Exceptions.h"
#include "EL/Types.h"

namespace TrenchBroom {
    namespace EL {
        class Exception : public ExceptionStream<Exception> {
        public:
            Exception() noexcept;
            Exception(const String& str) noexcept;
        };
        
        class ConversionError : public Exception {
        public:
            ConversionError(const String& value, const ValueType from, const ValueType to) noexcept;
        };
        
        class DereferenceError : public Exception {
        public:
            DereferenceError(const String& value, const ValueType from, const ValueType to) noexcept;
        };
        
        class EvaluationError : public Exception {
        public:
            EvaluationError(const String& msg) noexcept;
        };
        
        class IndexError : public EvaluationError {
        public:
            IndexError(const Value& indexableValue, const Value& indexValue) noexcept;
            IndexError(const Value& indexableValue, size_t index) noexcept;
            IndexError(const Value& indexableValue, const String& key) noexcept;
        };
        
        class IndexOutOfBoundsError : public IndexError {
        public:
            IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, size_t outOfBoundsIndex) noexcept;
            IndexOutOfBoundsError(const Value& indexableValue, const Value& indexValue, const String& outOfBoundsIndex) noexcept;
            IndexOutOfBoundsError(const Value& indexableValue, size_t index) noexcept;
            IndexOutOfBoundsError(const Value& indexableValue, const String& key) noexcept;
        };
    }
}

#endif /* ELExceptions_h */
