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

#ifndef TrenchBroom_Bsp29Parser
#define TrenchBroom_Bsp29Parser

#include "StringUtils.h"
#include "SharedPointer.h"
#include "Assets/AssetTypes.h"
#include "IO/EntityModelParser.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class Bsp29Model;
        class EntityModel;
        class Palette;
    }

    namespace IO {
        class Bsp29Parser : public EntityModelParser {
        private:
            struct TextureInfo {
                vm::vec3f sAxis;
                vm::vec3f tAxis;
                float sOffset;
                float tOffset;
                size_t textureIndex;
            };
            using TextureInfoList = std::vector<TextureInfo>;
            
            struct EdgeInfo {
                size_t vertexIndex1, vertexIndex2;
            };
            using EdgeInfoList = std::vector<EdgeInfo>;
            
            struct FaceInfo {
                size_t edgeIndex;
                size_t edgeCount;
                size_t textureInfoIndex;
            };
            using FaceInfoList = std::vector<FaceInfo>;
            
            using FaceEdgeIndexList = std::vector<int>;
            using VertexMarkList = std::vector<bool>;

            String m_name;
            const char* m_begin;
            // const char* m_end;
            const Assets::Palette& m_palette;
        public:
            Bsp29Parser(const String& name, const char* begin, const char* end, const Assets::Palette& palette);
        private:
            Assets::EntityModel* doParseModel() override;
            Assets::TextureCollection* parseTextures();
            TextureInfoList parseTextureInfos();
            std::vector<vm::vec3f> parseVertices();
            EdgeInfoList parseEdgeInfos();
            FaceInfoList parseFaceInfos();
            FaceEdgeIndexList parseFaceEdges();
            Assets::Bsp29Model* parseModels(Assets::TextureCollection* textureCollection, const TextureInfoList& textureInfos, const std::vector<vm::vec3f>& vertices, const EdgeInfoList& edgeInfos, const FaceInfoList& faceInfos, const FaceEdgeIndexList& faceEdges);
            vm::vec2f textureCoords(const vm::vec3f& vertex, const TextureInfo& textureInfo, const Assets::Texture* texture) const;
        };
    }
}

#endif /* defined(TrenchBroom_Bsp29Parser) */
