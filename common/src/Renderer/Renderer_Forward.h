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

#ifndef TRENCHBROOM_RENDERER_FORWARD_H
#define TRENCHBROOM_RENDERER_FORWARD_H

namespace TrenchBroom {
    namespace Renderer {
        class VboManager;
        class Vbo;

        class VertexArray;
        class BrushIndexArray;
        class BrushVertexArray;
        class BrushRendererBrushCache;

        class Camera;

        class RenderBatch;
        class RenderContext;

        template <typename VertexSpec> class IndexRangeMapBuilder;

        class IndexRangeRenderer;
        class TexturedRenderer;
        class TexturedIndexRangeRenderer;

        class ObjectRenderer;
        class EntityLinkRenderer;
        class PointHandleRenderer;

        enum class PrimitiveRendererOcclusionPolicy;
        enum class PrimitiveRendererCullingPolicy;
        class PrimitiveRenderer;

        enum class PrimType;
        class IndexRangeRenderer;

        class ActiveShader;
        class Shader;
        class ShaderConfig;
        class ShaderProgram;
        class ShaderManager;

        class FontGlyph;
        class FontManager;
        class FontTexture;
        class FontDescriptor;
        class TextAnchor;
        class TextRenderer;
    }
}

#endif //TRENCHBROOM_RENDERER_FORWARD_H
