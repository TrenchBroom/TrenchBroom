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

#include "CharArrayReader.h"

#include <cassert>
#include <cstring>
#include <functional>

namespace TrenchBroom {
    namespace IO {
        CharArrayReader::CharArrayReader(const char* begin, const char* end) :
        m_begin(begin),
        m_end(end),
        m_current(m_begin) {
            if (m_begin > m_end) {
                throw CharArrayReaderException("begin must be <= end");
            }
        }

        size_t CharArrayReader::size() const {
            return static_cast<size_t>(m_end - m_begin);
        }

        size_t CharArrayReader::currentOffset() const {
            return static_cast<size_t>(m_current - m_begin);
        }

        void CharArrayReader::seekFromBegin(const size_t offset) {
            if (offset > size()) {
                throw CharArrayReaderException("can't seek to offset " + std::to_string(offset) + " in buffer of size " + std::to_string(size()));
            }
            m_current = m_begin + offset;
        }

        void CharArrayReader::seekFromEnd(const size_t offset) {
            if (offset > size()) {
                throw CharArrayReaderException("can't seek to offset " + std::to_string(offset) + " before end in buffer of size " + std::to_string(size()));
            }
            m_current = m_end - offset;
        }

        void CharArrayReader::seekForward(const size_t offset) {
            if (m_current + offset > m_end) {
                throw CharArrayReaderException("can't seek " + std::to_string(offset) + " bytes from current offset " + std::to_string(currentOffset()) + " in buffer of size " + std::to_string(size()));
            }
            m_current += offset;
        }

        CharArrayReader CharArrayReader::subReaderFromBegin(const size_t offset, const size_t length) const {
            if (offset + length > size()) {
                throw CharArrayReaderException("sub reader " + std::to_string(offset) + "," + std::to_string(length) + " out of bounds");
            }
            return CharArrayReader(m_begin + offset, m_begin + offset + length);
        }

        CharArrayReader CharArrayReader::subReaderFromBegin(const size_t offset) const {
            return subReaderFromBegin(offset, size() - offset);
        }

        void CharArrayReader::read(char* val, const size_t readSize) {
            ensureCanRead(readSize);
            std::memcpy(val, m_current, readSize);
            m_current += readSize;
        }

        void CharArrayReader::read(unsigned char* val, const size_t size) {
            read(reinterpret_cast<char*>(val), size);
        }

        bool CharArrayReader::canRead(const size_t size) const {
            return static_cast<size_t>(m_end - m_current) >= size;
        }

        void CharArrayReader::ensureCanRead(const size_t readSize) const {
            if (!canRead(readSize)) {
                throw CharArrayReaderException("can't read " + std::to_string(readSize) + " bytes from current offset " + std::to_string(currentOffset()) + " in buffer of size " + std::to_string(size()));
            }
        }

        bool CharArrayReader::eof() const {
            return m_current == m_end;
        }

        String CharArrayReader::readString(const size_t size) {
            std::vector<char> buffer;
            buffer.resize(size + 1);
            buffer[size] = 0;
            read(buffer.data(), size);
            return String(buffer.data());
        }
    }
}
