/*
 Copyright (C) 2021 Kristian Duske

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

#pragma once

#include "FloatType.h"
#include "Assets/AssetReference.h"

#include <vecmath/forward.h>
#include <vecmath/bbox.h>
#include <vecmath/vec.h>

#include <iosfwd>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace Model {
        class BezierPatch {
        public:
            using Point = vm::vec<FloatType, 5>;
        private:
            size_t m_pointRowCount;
            size_t m_pointColumnCount;
            std::vector<Point> m_controlPoints;
            vm::bbox3 m_bounds;

            std::string m_textureName;
            Assets::AssetReference<Assets::Texture> m_textureReference;
        public:
            BezierPatch(size_t pointRowCount, size_t pointColumnCount, std::vector<Point> controlPoints, std::string textureName);
            ~BezierPatch();

            BezierPatch(const BezierPatch& other);
            BezierPatch(BezierPatch&& other) noexcept;

            BezierPatch& operator=(const BezierPatch& other);
            BezierPatch& operator=(BezierPatch&& other) noexcept;

            size_t pointRowCount() const;
            size_t pointColumnCount() const;

            size_t quadRowCount() const;
            size_t quadColumnCount() const;

            size_t surfaceRowCount() const;
            size_t surfaceColumnCount() const;

            const std::vector<Point>& controlPoints() const;
            const Point& controlPoint(size_t row, size_t col) const;
            void setControlPoint(size_t row, size_t col, Point controlPoint);

            const vm::bbox3& bounds() const;

            const std::string& textureName() const;
            void setTextureName(std::string textureName);

            const Assets::Texture* texture() const;
            bool setTexture(Assets::Texture* texture);

            void transform(const vm::mat4x4& transformation);

            std::vector<Point> evaluate(size_t subdivisionsPerSurface) const;
        };

        std::ostream& operator<<(std::ostream& str, const BezierPatch& patch);

        bool operator==(const BezierPatch& lhs, const BezierPatch& rhs);
        bool operator!=(const BezierPatch& lhs, const BezierPatch& rhs);
    }
}
