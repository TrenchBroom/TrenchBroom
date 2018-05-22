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

#include "IndexArrayMap.h"

#include "Renderer/IndexArray.h"

#include <stdexcept>
#include <string>

namespace TrenchBroom {
    namespace Renderer {
        IndexArrayRange::IndexArrayRange() :
                offset(0),
                capacity(0),
                count(0) {}

        IndexArrayRange::IndexArrayRange(const size_t i_offset, const size_t i_capacity) :
                offset(i_offset),
                capacity(i_capacity),
                count(0) {}

        size_t IndexArrayRange::add(const size_t i_count) {
            assert(capacity - count >= i_count);
            const size_t result = offset + count;
            count += i_count;
            return result;
        }

        IndexArrayMap::Size::Size() :
                m_points(0),
                m_lines(0),
                m_triangles(0){}
        
        void IndexArrayMap::Size::inc(const PrimType primType, const size_t count) {
            switch (primType) {
                case GL_POINTS:
                    m_points += count;
                    break;
                case GL_LINES:
                    m_lines += count;
                    break;
                case GL_TRIANGLES:
                    m_triangles += count;
                    break;
                default:
                    throw std::invalid_argument("Unsupported primitive type " + std::to_string(primType));
            }
        }

        size_t IndexArrayMap::Size::indexCount() const {
            return m_points + m_lines + m_triangles;
        }

        void IndexArrayMap::initialize(const Size& size, const size_t baseOffset) {
            size_t offset = baseOffset;

            m_pointsRange = IndexArrayRange(offset, size.m_points);
            offset += size.m_points;

            m_linesRange = IndexArrayRange(offset, size.m_lines);
            offset += size.m_lines;

            m_trianglesRange = IndexArrayRange(offset, size.m_triangles);
            offset += size.m_triangles;
        }

        IndexArrayMap::IndexArrayMap() :
                m_pointsRange(0,0),
                m_linesRange(0,0),
                m_trianglesRange(0,0){}

        IndexArrayMap::IndexArrayMap(const Size& size) {
            initialize(size, 0);
        }

        IndexArrayMap::IndexArrayMap(const Size& size, const size_t baseOffset) {
            initialize(size, baseOffset);
        }

        size_t IndexArrayMap::add(const PrimType primType, const size_t count) {
            switch (primType) {
                case GL_POINTS:
                    return m_pointsRange.add(count);
                case GL_LINES:
                    return m_linesRange.add(count);
                case GL_TRIANGLES:
                    return m_trianglesRange.add(count);
                default:
                    throw std::invalid_argument("Unsupported primitive type " + std::to_string(primType));
            }
        }

        void IndexArrayMap::render(std::shared_ptr<IndexHolder> indexArray) const {
            if (m_pointsRange.count > 0) {
                indexArray->render(GL_POINTS, m_pointsRange.offset, m_pointsRange.count);
            }
            if (m_linesRange.count > 0) {
                indexArray->render(GL_LINES, m_linesRange.offset, m_linesRange.count);
            }
            if (m_trianglesRange.count > 0) {
                indexArray->render(GL_TRIANGLES, m_trianglesRange.offset, m_trianglesRange.count);
            }
        }
    }
}
