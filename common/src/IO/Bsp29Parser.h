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

#ifndef TrenchBroom_Bsp29Parser
#define TrenchBroom_Bsp29Parser

#include "VecMath.h"
#include "StringUtils.h"
#include "SharedPointer.h"
#include "Assets/AssetTypes.h"
#include "IO/EntityModelParser.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class AutoTexture;
        class Bsp29Model;
        class EntityModel;
        class Palette;
    }

    namespace IO {
        class Bsp29Parser : public EntityModelParser {
        private:
            struct TextureInfo {
                Vec3f sAxis;
                Vec3f tAxis;
                float sOffset;
                float tOffset;
                size_t textureIndex;
            };
            typedef std::vector<TextureInfo> TextureInfoArray;
            
            struct EdgeInfo {
                size_t vertexIndex1, vertexIndex2;
            };
            typedef std::vector<EdgeInfo> EdgeInfoArray;
            
            struct FaceInfo {
                size_t edgeIndex;
                size_t edgeCount;
                size_t textureInfoIndex;
            };
            typedef std::vector<FaceInfo> FaceInfoArray;
            
            typedef std::vector<int> FaceEdgeIndexArray;
            typedef std::vector<bool> VertexMarkArray;
            typedef std::vector<size_t> ModelVertexArray;
            
            String m_name;
            const char* m_begin;
            // const char* m_end;
            const Assets::Palette& m_palette;
        public:
            Bsp29Parser(const String& name, const char* begin, const char* end, const Assets::Palette& palette);
        private:
            Assets::EntityModel* doParseModel();
            Assets::TextureCollection* parseTextures();
            TextureInfoArray parseTextureInfos();
            Vec3f::Array parseVertices();
            EdgeInfoArray parseEdgeInfos();
            FaceInfoArray parseFaceInfos();
            FaceEdgeIndexArray parseFaceEdges();
            Assets::Bsp29Model* parseModels(Assets::TextureCollection* textureCollection, const TextureInfoArray& textureInfos, const Vec3f::Array& vertices, const EdgeInfoArray& edgeInfos, const FaceInfoArray& faceInfos, const FaceEdgeIndexArray& faceEdges);
            Vec2f textureCoords(const Vec3f& vertex, const TextureInfo& textureInfo, const Assets::Texture& texture) const;
        };
    }
}

#endif /* defined(TrenchBroom_Bsp29Parser) */
