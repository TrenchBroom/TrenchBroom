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

#ifndef TrenchBroom_DkmParser
#define TrenchBroom_DkmParser

#include "StringUtils.h"
#include "VecMath.h"
#include "Assets/AssetTypes.h"
#include "Assets/Md2Model.h"
#include "IO/EntityModelParser.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class EntityModel;
        class Palette;
    }
    
    namespace IO {
        class FileSystem;
        class Path;
        
        namespace DkmLayout {
            static const int Ident = (('2'<<24) + ('P'<<16) + ('D'<<8) + 'I');
            static const int Version1 = 8;
            static const int Version2 = 8;
            static const size_t SkinNameLength = 64;
            static const size_t FrameNameLength = 16;
        }

        // see http://tfc.duke.free.fr/coding/md2-specs-en.html
        class DkmParser : public EntityModelParser {
        private:
            static const Vec3f Normals[162];

            struct DkmSkin {
                char name[DkmLayout::SkinNameLength];
            };
            typedef std::vector<DkmSkin> DkmSkinList;
            
            struct DkmVertex {
                unsigned char x, y, z;
                unsigned char normalIndex;
            };
            typedef std::vector<DkmVertex> DkmVertexList;
            
            struct DkmFrame {
                Vec3f scale;
                Vec3f offset;
                char name[DkmLayout::FrameNameLength];
                DkmVertexList vertices;
                
                DkmFrame(size_t vertexCount);
                Vec3f vertex(size_t index) const;
                const Vec3f& normal(size_t index) const;
            };
            typedef std::vector<DkmFrame> Md2FrameList;

            struct DkmMeshVertex {
                Vec2f texCoords;
                size_t vertexIndex;
            };
            typedef std::vector<DkmMeshVertex> DkmMeshVertexList;
            
            struct DkmMesh {
                enum Type {
                    Fan,
                    Strip
                };
                
                Type type;
                size_t vertexCount;
                DkmMeshVertexList vertices;
                
                DkmMesh(int i_vertexCount);
            };
            typedef std::vector<DkmMesh> DkmMeshList;
            
            
            String m_name;
            const char* m_begin;
            /* const char* m_end; */
            const Assets::Palette& m_palette;
            const FileSystem& m_fs;
        public:
            DkmParser(const String& name, const char* begin, const char* end, const Assets::Palette& palette, const FileSystem& fs);
        private:
            Assets::EntityModel* doParseModel() override;
            DkmSkinList parseSkins(const char* begin, size_t skinCount);
            Md2FrameList parseFrames(const char* begin, size_t frameCount, size_t frameVertexCount, int version);
            DkmMeshList parseMeshes(const char* begin, size_t commandCount);
            Assets::EntityModel* buildModel(const DkmSkinList& skins, const Md2FrameList& frames, const DkmMeshList& meshes);
            Assets::TextureList loadTextures(const DkmSkinList& skins);
            Assets::Texture* readTexture(const DkmSkin& skin);
            Assets::Md2Model::FrameList buildFrames(const Md2FrameList& frames, const DkmMeshList& meshes);
            Assets::Md2Model::Frame* buildFrame(const DkmFrame& frame, const DkmMeshList& meshes);
            Assets::Md2Model::VertexList getVertices(const DkmFrame& frame, const DkmMeshVertexList& meshVertices) const;
        };
    }
}

#endif /* defined(TrenchBroom_DkmParser) */
