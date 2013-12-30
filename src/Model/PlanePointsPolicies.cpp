/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#include "PlanePointsPolicies.h"

namespace TrenchBroom {
    namespace Model {
        void FloatPlanePointsPolicy::computePoints(const Plane3& plane, BrushFace::Points& points) {
            for (size_t i = 0; i < 3; ++i)
                points[i].correct();
        }

        void RoundDownIntegerPlanePointsPolicy::computePoints(const Plane3& plane, BrushFace::Points& points) {
            for (size_t i = 0; i < 3; ++i)
                for (size_t j = 0; j < 3; ++j)
                    points[i][j] = static_cast<long>(points[i][j]);
        }

        
        class GridSearchCursor {
        private:
            static const size_t Center = 4;
            
            static const Vec2 MoveOffsets[];
            
            const Plane3& m_plane;
            const FloatType m_frequency;
            
            Vec2 m_position;
            FloatType m_errors[9];
        public:
            GridSearchCursor(const Plane3& plane, const FloatType frequency) :
            m_plane(plane),
            m_frequency(frequency) {
                for (size_t i = 0; i < 9; ++i)
                    m_errors[i] = 0.0;
            }
            
            Vec3 findMinimum(const Vec3& initialPosition) {
                for (size_t i = 0; i < 2; ++i)
                    m_position[i] = Math::round(initialPosition[i]);
                
                findLocalMinimum();
                const Vec2 localMinPos = m_position;
                const FloatType localMinErr = m_errors[Center];
                
                Vec2 globalMinPos = localMinPos;
                FloatType globalMinErr = localMinErr;
                
                if (globalMinErr > 0.0) {
                    // To escape local minima, let's search some adjacent quadrants
                    // The number of extra quadrants should depend on the frequency: The higher the frequency, the
                    // more quadrants should be searched.
                    const size_t numQuadrants = static_cast<size_t>(std::ceil(m_frequency * m_frequency * 3.0));
                    for (size_t i = 0; i < numQuadrants && globalMinErr > 0.0; ++i) {
                        if (i != Center) {
                            m_position = localMinPos + i * 3.0 * MoveOffsets[i];
                            findLocalMinimum();
                            const FloatType newError = m_errors[Center];
                            if (newError < globalMinErr) {
                                globalMinPos = m_position;
                                globalMinErr = newError;
                            }
                        }
                    }
                }
                
                return Vec3(globalMinPos.x(),
                            globalMinPos.y(),
                            Math::round(m_plane.zAt(globalMinPos)));
            }
        private:
            void findLocalMinimum() {
                updateErrors();
                
                size_t smallestError = findSmallestError();
                while (smallestError != Center)
                    smallestError = moveCursor(smallestError);
            }
            
            size_t moveCursor(const size_t direction) {
                m_position += MoveOffsets[direction];
                updateErrors();
                return findSmallestError();
            }
            
            void updateErrors() {
                for (size_t i = 0; i < 9; ++i)
                    m_errors[i] = computeError(i);
            }
            
            FloatType computeError(const size_t location) const {
                const FloatType z = m_plane.zAt(m_position + MoveOffsets[location]);
                return std::abs(z - Math::round(z));
            }
            
            size_t findSmallestError() {
                size_t smallest = Center;
                for (size_t i = 0; i < 9; ++i) {
                    if (m_errors[i] < m_errors[smallest])
                        smallest = i;
                }
                return smallest;
            }
        };

        const Vec2 GridSearchCursor::MoveOffsets[] = {
            Vec2(-1.0,  1.0), Vec2( 0.0,  1.0), Vec2( 1.0,  1.0),
            Vec2(-1.0,  0.0), Vec2( 0.0,  0.0), Vec2( 1.0,  0.0),
            Vec2(-1.0, -1.0), Vec2( 0.0, -1.0), Vec2( 1.0, -1.0)
        };
    }
}
