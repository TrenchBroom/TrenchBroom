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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_Bsp_h
#define TrenchBroom_Bsp_h

#include <string>
#include <vector>
#include <map>
#include <istream>
#include "Utilities/VecMath.h"
#include "IO/Pak.h"

#if defined _MSC_VER
#include <cstdint>
#elif defined __GNUC__
#include <stdint.h>
#endif

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            class BspTexture {
            public:
                std::string name;
                const unsigned char* image;
                int width;
                int height;
                BspTexture(const std::string& name, const unsigned char* image, int width, int height);
                ~BspTexture();
            };

            class BspTextureInfo {
            public:
                Vec3f sAxis;
                Vec3f tAxis;
                float sOffset;
                float tOffset;
                BspTexture* texture;
            };

            class BspFace {
            public:
                BBox bounds;
                BspTextureInfo* textureInfo;
                std::vector<Vec3f> vertices;
                BspFace(BspTextureInfo* textureInfo, const std::vector<Vec3f>& vertices);
                Vec2f textureCoordinates(const Vec3f& vertex);
            };

            class BspModel {
            public:
                Vec3f center;
                BBox bounds;
                std::vector<BspFace*> faces;
                unsigned int vertexCount;
                BspModel(const std::vector<BspFace*>& faces, int vertexCount, const Vec3f& center, const BBox& bounds);
                ~BspModel();
            };

            class BspEdgeInfo {
            public:
                uint16_t vertex0;
                uint16_t vertex1;
            };

            class BspFaceInfo {
            public:
                int32_t edgeIndex;
                uint16_t edgeCount;
                uint16_t textureInfoIndex;
            };

            class Bsp {
            private:
                void readTextures(IO::PakStream& stream, unsigned int count);
                void readTextureInfos(IO::PakStream& stream, unsigned int count, std::vector<BspTexture*>& textures);
                void readVertices(IO::PakStream& stream, unsigned int count, std::vector<Vec3f>& vertices);
                void readEdges(IO::PakStream& stream, unsigned int count, std::vector<BspEdgeInfo>& edges);
                void readFaces(IO::PakStream& stream, unsigned int count, std::vector<BspFaceInfo>& faces);
                void readFaceEdges(IO::PakStream& stream, unsigned int count, int32_t* indices);
            public:
                std::string name;
                std::vector<BspModel*> models;
                std::vector<BspTexture*>textures;
                std::vector<BspTextureInfo*>textureInfos;
                Bsp(const std::string& name, IO::PakStream stream);
                ~Bsp();
            };

            class BspManager {
            private:
                std::map<std::string, Bsp*> bsps;
            public:
                static BspManager* sharedManager;
                BspManager();
                ~BspManager();
                Bsp* bspForName(const std::string& name, const std::vector<std::string>& paths);
            };
        }
    }
}
#endif
