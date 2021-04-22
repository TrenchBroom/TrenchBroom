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

#include <vecmath/forward.h>
#include <vecmath/util.h>

#include <utility>
#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace Renderer {
        vm::vec3f gridColorForTexture(const Assets::Texture* texture);

        void glSetEdgeOffset(double f);
        void glResetEdgeOffset();

        void coordinateSystemVerticesX(const vm::bbox3f& bounds, vm::vec3f& start, vm::vec3f& end);
        void coordinateSystemVerticesY(const vm::bbox3f& bounds, vm::vec3f& start, vm::vec3f& end);
        void coordinateSystemVerticesZ(const vm::bbox3f& bounds, vm::vec3f& start, vm::vec3f& end);

        class TextureRenderFunc {
        public:
            virtual ~TextureRenderFunc();
            virtual void before(const Assets::Texture* texture);
            virtual void after(const Assets::Texture* texture);
        };

        class DefaultTextureRenderFunc : public TextureRenderFunc {
        public:
            void before(const Assets::Texture* texture) override;
            void after(const Assets::Texture* texture) override;
        };

        std::vector<vm::vec2f> circle2D(float radius, size_t segments);
        std::vector<vm::vec2f> circle2D(float radius, float startAngle, float angleLength, size_t segments);
        std::vector<vm::vec3f> circle2D(float radius, vm::axis::type axis, float startAngle, float angleLength, size_t segments);
        std::pair<float, float> startAngleAndLength(vm::axis::type axis, const vm::vec3f& startAxis, const vm::vec3f& endAxis);

        size_t roundedRect2DVertexCount(size_t cornerSegments);
        std::vector<vm::vec2f> roundedRect2D(const vm::vec2f& size, float cornerRadius, size_t cornerSegments);
        std::vector<vm::vec2f> roundedRect2D(float width, float height, float cornerRadius, size_t cornerSegments);

        struct VertsAndNormals {
            std::vector<vm::vec3f> vertices;
            std::vector<vm::vec3f> normals;

            explicit VertsAndNormals(size_t vertexCount);
        };

        std::vector<vm::vec3f> sphere3D(float radius, size_t iterations);
        VertsAndNormals circle3D(float radius, size_t segments);
        VertsAndNormals cylinder3D(float radius, float length, size_t segments);
        VertsAndNormals cone3D(float radius, float length, size_t segments);
    }
}
