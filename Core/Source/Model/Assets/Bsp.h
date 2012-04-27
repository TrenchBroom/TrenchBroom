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

#ifdef _MSC_VER
#include <cstdint>
#endif

using namespace std;

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            class BspTexture {
            public:
                string name;
                const unsigned char* image;
                int width;
                int height;
                BspTexture(string name, const unsigned char* image, int width, int height);
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
                vector<Vec3f> vertices;
                BspFace(BspTextureInfo* textureInfo, vector<Vec3f>& vertices);
                Vec2f textureCoordinates(const Vec3f& vertex);
            };
            
            class BspModel {
            public:
                Vec3f center;
                BBox bounds;
                BBox maxBounds;
                vector<BspFace*> faces;
                int vertexCount;
                BspModel(vector<BspFace*>& faces, int vertexCount, Vec3f& center, BBox& bounds, BBox& maxBounds);
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
                void readTextures(istream& stream, int counts);
                void readTextureInfos(istream& stream, int count, vector<BspTexture*>& textures);
                void readVertices(istream& stream, int count, vector<Vec3f>& vertices);
                void readEdges(istream& stream, int count, vector<BspEdgeInfo>& edges);
                void readFaces(istream& stream, int count, vector<BspFaceInfo>& faces);
                void readFaceEdges(istream& stream, int count, int32_t* indices);
            public:
                string name;
                vector<BspModel*> models;
                vector<BspTexture*>textures;
                vector<BspTextureInfo*>textureInfos;
                Bsp(string& name, istream& stream);
                ~Bsp();
            };
            
            class BspManager {
            private:
                map<string, Bsp*> bsps;
            public:
                static BspManager* sharedManager;
                BspManager();
                ~BspManager();
                Bsp* bspForName(string& name, vector<string>& paths);
            };
        }
    }
}
#endif
