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

#pragma once

#include "FloatType.h"

#include <kdl/vector_set.h>

#include <vecmath/forward.h>
#include <vecmath/mat.h>
#include <vecmath/mat_io.h>
#include <vecmath/vec.h>
#include <vecmath/vec_io.h> // enable Catch2 to print vm::vec on test failures

#include <string>

#include "Catch2.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    bool texCoordsEqual(const vm::vec2f& tc1, const vm::vec2f& tc2);
    bool pointExactlyIntegral(const vm::vec3d &point);
    bool UVListsEqual(const std::vector<vm::vec2f>& uvs,
                      const std::vector<vm::vec2f>& transformedVertUVs);

    namespace Model {
        class Brush;
        class BrushFace;
        class BrushNode;

        BrushFace createParaxial(const vm::vec3& point0, const vm::vec3& point1, const vm::vec3& point2, const std::string& textureName = "");

        std::vector<vm::vec3> asVertexList(const std::vector<vm::segment3>& edges);
        std::vector<vm::vec3> asVertexList(const std::vector<vm::polygon3>& faces);

        void assertTexture(const std::string& expected, const BrushNode* brush, const vm::vec3d& faceNormal);
        void assertTexture(const std::string& expected, const BrushNode* brush, const vm::vec3d& v1, const vm::vec3d& v2, const vm::vec3d& v3);
        void assertTexture(const std::string& expected, const BrushNode* brush, const vm::vec3d& v1, const vm::vec3d& v2, const vm::vec3d& v3, const vm::vec3d& v4);
        void assertTexture(const std::string& expected, const BrushNode* brush, const std::vector<vm::vec3d>& vertices);
        void assertTexture(const std::string& expected, const BrushNode* brush, const vm::polygon3d& vertices);

        void assertTexture(const std::string& expected, const Brush& brush, const vm::vec3d& faceNormal);
        void assertTexture(const std::string& expected, const Brush& brush, const vm::vec3d& v1, const vm::vec3d& v2, const vm::vec3d& v3);
        void assertTexture(const std::string& expected, const Brush& brush, const vm::vec3d& v1, const vm::vec3d& v2, const vm::vec3d& v3, const vm::vec3d& v4);
        void assertTexture(const std::string& expected, const Brush& brush, const std::vector<vm::vec3d>& vertices);
        void assertTexture(const std::string& expected, const Brush& brush, const vm::polygon3d& vertices);
    }

    enum class Component {
        R, G, B, A
    };

    enum class ColorMatch {
        Exact, Approximate
    };

    int getComponentOfPixel(const Assets::Texture& texture, std::size_t x, std::size_t y, Component component);
    void checkColor(const Assets::Texture& texture, std::size_t x, std::size_t y,
                    int r, int g, int b, int a, ColorMatch match = ColorMatch::Exact);

    class GlobMatcher : public Catch::MatcherBase<std::string> {
    private:
        std::string m_glob;
    public:
        explicit GlobMatcher(const std::string& glob);
        bool match(const std::string& value) const override;
        std::string describe() const override;
    };

    GlobMatcher MatchesGlob(const std::string& glob);
}
