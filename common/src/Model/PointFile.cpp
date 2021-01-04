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

#include "IO/IOUtils.h"
#include "IO/Path.h"

#include <vecmath/vec.h>
#include <vecmath/vec_io.h>

#include <cassert>
#include <fstream>
#include <string>

namespace TrenchBroom {
    namespace Model {
        PointFile::PointFile() :
        m_current(0) {}

        PointFile::PointFile(const IO::Path& path) :
        m_current(0) {
            load(path);
        }

        bool PointFile::canLoad(const IO::Path& path) {
            std::ifstream stream = openPathAsInputStream(path);
            return stream.is_open() && stream.good();
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

        const std::vector<vm::vec3f>& PointFile::points() const {
            return m_points;
        }

        const vm::vec3f& PointFile::currentPoint() const {
            return m_points[m_current];
        }

        const vm::vec3f PointFile::currentDirection() const {
            if (m_points.size() <= 1) {
                return vm::vec3f::pos_x();
            } else if (m_current >= m_points.size() - 1) {
                return normalize(m_points[m_points.size() - 1] - m_points[m_points.size() - 2]);
            } else {
                return normalize(m_points[m_current + 1] - m_points[m_current]);
            }
        }

        void PointFile::advance() {
            assert(hasNextPoint());
            ++m_current;
        }

        void PointFile::retreat() {
            assert(hasPreviousPoint());
            --m_current;
        }

        void PointFile::load(const IO::Path& path) {
            static const float Threshold = vm::to_radians(15.0f);

            std::ifstream stream = openPathAsInputStream(path);
            assert(stream.is_open());

            std::vector<vm::vec3f> points;

            if (!stream.eof()) {
                std::string line;
                std::getline(stream, line);
                points.push_back(vm::parse<float, 3>(line).value_or(vm::vec3f::zero()));
                vm::vec3f lastPoint = points.back();

                if (!stream.eof()) {
                    std::getline(stream, line);
                    vm::vec3f curPoint = vm::parse<float, 3>(line).value_or(vm::vec3f::zero());
                    vm::vec3f refDir = normalize(curPoint - lastPoint);

                    while (!stream.eof()) {
                        lastPoint = curPoint;
                        std::getline(stream, line);
                        curPoint = vm::parse<float, 3>(line).value_or(vm::vec3f::zero());

                        const vm::vec3f dir = normalize(curPoint - lastPoint);
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
                    const vm::vec3f& curPoint = points[i];
                    const vm::vec3f& nextPoint = points[i + 1];
                    const vm::vec3f dir = normalize(nextPoint - curPoint);

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
