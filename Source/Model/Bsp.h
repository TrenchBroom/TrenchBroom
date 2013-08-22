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

#include "IO/Pak.h"
#include "Utility/Console.h"
#include "Utility/VecMath.h"

#include <istream>
#include <map>
#include <vector>

#if defined _MSC_VER
#include <cstdint>
#elif defined __GNUC__
#include <stdint.h>
#endif

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Model {
        namespace BspLayout {
            static const unsigned int DirTexturesAddress    = 0x14;
            static const unsigned int DirTexturesSize       = 0x18;
            static const unsigned int DirVerticesAddress    = 0x1C;
            static const unsigned int DirVerticesSize       = 0x20;
            static const unsigned int DirTexInfosAddress    = 0x34;
            static const unsigned int DirTexInfoSize        = 0x38;
            static const unsigned int DirFacesAddress       = 0x3C;
            static const unsigned int DirFacesSize          = 0x40;
            static const unsigned int DirEdgesAddress       = 0x64;
            static const unsigned int DirEdgesSize          = 0x68;
            static const unsigned int DirFaceEdgesAddress   = 0x6C;
            static const unsigned int DirFaceEdgesSize      = 0x70;
            static const unsigned int DirModelAddress       = 0x74;
            static const unsigned int DirModelSize          = 0x78;
            
            static const unsigned int TextureNameLength     = 0x10;
            
            static const unsigned int FaceSize              = 0x14;
            static const unsigned int FaceEdgeIndex         = 0x4;
            static const unsigned int FaceRest              = 0x8;

            static const unsigned int TexInfoSize           = 0x28;
            static const unsigned int TexInfoRest           = 0x4;

            static const unsigned int FaceEdgeSize          = 0x4;
            static const unsigned int ModelSize             = 0x40;
            static const unsigned int ModelOrigin           = 0x18;
            static const unsigned int ModelFaceIndex        = 0x38;
            static const unsigned int ModelFaceCount        = 0x3c;
        }
        
        class BspEdgeInfo {
        public:
            unsigned int vertex0;
            unsigned int vertex1;
        };
        
        class BspTexture;
        class BspTextureInfo {
        public:
            Vec3f sAxis;
            Vec3f tAxis;
            float sOffset;
            float tOffset;
            BspTexture* texture;
        };
        
        class BspFaceInfo {
        public:
            unsigned int edgeIndex;
            unsigned int edgeCount;
            unsigned int textureInfoIndex;
        };
        
        class BspTexture {
        private:
            String m_name;
            const unsigned char* m_image;
            unsigned int m_width;
            unsigned int m_height;
        public:
            BspTexture(const String& name, const unsigned char* image, unsigned int width, unsigned int height);
            ~BspTexture();
            
            inline const String& name() const {
                return m_name;
            }
            
            inline const unsigned char* image() const {
                return m_image;
            }
            
            inline unsigned int width() const {
                return m_width;
            }
            
            inline unsigned int height() const {
                return m_height;
            }
        };
        
        class BspFace {
            BBoxf m_bounds;
            BspTextureInfo* m_textureInfo;
            Vec3f::List m_vertices;
        public:
            BspFace(BspTextureInfo* textureInfo, const Vec3f::List& vertices);

            inline void textureCoordinates(const Vec3f& vertex, Vec2f& result) const {
                result[0] = (vertex.dot(m_textureInfo->sAxis) + m_textureInfo->sOffset) / m_textureInfo->texture->width();
                result[1] = (vertex.dot(m_textureInfo->tAxis) + m_textureInfo->tOffset) / m_textureInfo->texture->height();
            }
            
            inline const BspTexture& texture() const {
                return *m_textureInfo->texture;
            }
            
            inline const String& textureName() const {
                return m_textureInfo->texture->name();
            }
            
            inline const Vec3f::List& vertices() const {
                return m_vertices;
            }
        };
        
        typedef std::vector<BspFace*> BspFaceList;

        class BspModel {
        private:
            BspFaceList m_faces;
            unsigned int m_vertexCount;
            Vec3f m_center;
            BBoxf m_bounds;
        public:
            BspModel(const BspFaceList& faces, unsigned int vertexCount, const Vec3f& center, const BBoxf& bounds);
            ~BspModel();
            
            inline unsigned int vertexCount() const {
                return m_vertexCount;
            }
            
            inline const BspFaceList& faces() const {
                return m_faces;
            }
            
            inline const Vec3f& center() const {
                return m_center;
            }
            
            inline const BBoxf& bounds() const {
                return m_bounds;
            }
        };
        

        typedef std::vector<BspModel*> BspModelList;

        class Bsp {
        private:
            typedef std::vector<BspTexture*> BspTextureList;
            typedef std::vector<BspEdgeInfo> BspEdgeInfoList;
            typedef std::vector<BspFaceInfo> BspFaceInfoList;
            typedef std::vector<BspTextureInfo*> BspTextureInfoList;
            typedef std::vector<int> BspFaceEdgeIndexList;

            String m_name;
            BspModelList m_models;
            BspTextureList m_textures;
            BspTextureInfoList m_textureInfos;

            void readTextures(char*& cursor, unsigned int count);
            void readTextureInfos(char*& cursor, unsigned int count, BspTextureList& textures);
            void readVertices(char*& cursor, unsigned int count, Vec3f::List& vertices);
            void readEdges(char*& cursor, unsigned int count, BspEdgeInfoList& edges);
            void readFaces(char*& cursor, unsigned int count, BspFaceInfoList& faces);
            void readFaceEdges(char*& cursor, unsigned int count, BspFaceEdgeIndexList& indices);
        public:
            Bsp(const String& name, char* begin, char* end);
            ~Bsp();
            
            inline const BspModelList& models() const {
                return m_models;
            }
        };
        
        class BspManager {
        private:
            typedef std::map<String, Bsp*> BspMap;
            
            BspMap m_bsps;
        public:
            static BspManager* sharedManager;
            
            BspManager();
            ~BspManager();

            const Bsp* bsp(const String& name, const StringList& paths, Utility::Console& console);
        };
    }
}
#endif
