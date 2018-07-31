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

#include "PointFile.h"

#include "IO/Path.h"

#include <cassert>
#include <fstream>

namespace TrenchBroom {
    namespace Model {
        PointFile::PointFile() :
        m_current(0) {}

        PointFile::PointFile(const IO::Path& path) :
        m_current(0) {
            load(path);
        }
        
        bool PointFile::empty() const {
            return m_points.empty();
        }

        bool PointFile::hasNextPoint() const {
            return m_current < m_points.size() - 1;
        }
        
        bool PointFile::hasPreviousPoint() const {
            return m_current > 0;
        }
    
        const Vec3f::List& PointFile::points() const {
            return m_points;
        }
        
        const Vec3f& PointFile::currentPoint() const {
            return m_points[m_current];
        }
        
        const Vec3f PointFile::currentDirection() const {
            if (m_points.size() <= 1)
                return Vec3f::PosX;
            if (m_current >= m_points.size() - 1)
                return (m_points[m_points.size() - 1] - m_points[m_points.size() - 2]).normalized();
            return (m_points[m_current + 1] - m_points[m_current]).normalized();
        }
        
        void PointFile::advance() {
            assert(hasNextPoint());
            ++m_current;
        }
        
        void PointFile::retreat() {
            assert(hasPreviousPoint());
            --m_current;
        }
        
        void PointFile::load(const IO::Path& pointFilePath) {
            static const float Threshold = Math::radians(15.0f);
            
            std::fstream stream(pointFilePath.asString().c_str(), std::ios::in);
            assert(stream.is_open());
            
            Vec3f::List points;
            String line;
            
            if (!stream.eof()) {
                std::getline(stream, line);
                points.push_back(Vec3f::parse(line));
                Vec3f lastPoint = points.back();
                
                if (!stream.eof()) {
                    std::getline(stream, line);
                    Vec3f curPoint = Vec3f::parse(line);
                    Vec3f refDir = (curPoint - lastPoint).normalized();
                    
                    while (!stream.eof()) {
                        lastPoint = curPoint;
                        std::getline(stream, line);
                        curPoint = Vec3f::parse(line);
                        
                        const Vec3f dir = (curPoint - lastPoint).normalized();
                        if (std::acos(dot(dir, refDir)) > Threshold) {
                            points.push_back(lastPoint);
                            refDir = dir;
                        }
                    }
                    
                    points.push_back(curPoint);
                }
            }

            if (points.size() > 1) {
                for (size_t i = 0; i < points.size() - 1; ++i) {
                    const Vec3f& curPoint = points[i];
                    const Vec3f& nextPoint = points[i + 1];
                    const Vec3f dir = (nextPoint - curPoint).normalized();
                    
                    m_points.push_back(curPoint);
                    const float dist = length(nextPoint - curPoint);
                    size_t segments = static_cast<size_t>(dist / 64.0f);
                    for (unsigned int j = 1; j < segments; ++j)
                        m_points.push_back(curPoint + dir * static_cast<float>(j) * 64.0f);
                }
                m_points.push_back(points.back());
            }
        }
    }
}
