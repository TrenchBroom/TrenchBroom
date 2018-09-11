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

#ifndef TrenchBroom_Md2Parser
#define TrenchBroom_Md2Parser

#include "StringUtils.h"
#include "Assets/AssetTypes.h"
#include "Assets/Md2Model.h"
#include "IO/EntityModelParser.h"

#include <vecmath/vec.h>

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class EntityModel;
        class Palette;
    }
    
    namespace IO {
        class FileSystem;
        class Path;
        
        namespace Md2Layout {
            static const int Ident = (('2'<<24) + ('P'<<16) + ('D'<<8) + 'I');
            static const int Version = 8;
            static const size_t SkinNameLength = 64;
            static const size_t FrameNameLength = 16;
        }

        // see http://tfc.duke.free.fr/coding/md2-specs-en.html
        class Md2Parser : public EntityModelParser {
        private:
            static const vm::vec3f Normals[162];

            struct Md2Skin {
                char name[Md2Layout::SkinNameLength];
            };
            using Md2SkinList = std::vector<Md2Skin>;
            
            struct Md2Vertex {
                unsigned char x, y, z;
                unsigned char normalIndex;
            };
            using Md2VertexList = std::vector<Md2Vertex>;
            
            struct Md2Frame {
                vm::vec3f scale;
                vm::vec3f offset;
                char name[Md2Layout::FrameNameLength];
                Md2VertexList vertices;
                
                Md2Frame(size_t vertexCount);
                vm::vec3f vertex(size_t index) const;
                const vm::vec3f& normal(size_t index) const;
            };
            using Md2FrameList = std::vector<Md2Frame>;

            struct Md2MeshVertex {
                vm::vec2f texCoords;
                size_t vertexIndex;
            };
            using Md2MeshVertexList = std::vector<Md2MeshVertex>;
            
            struct Md2Mesh {
                enum Type {
                    Fan,
                    Strip
                };
                
                Type type;
                size_t vertexCount;
                Md2MeshVertexList vertices;
                
                explicit Md2Mesh(int i_vertexCount);
            };
            using Md2MeshList =  std::vector<Md2Mesh>;
            
            
            String m_name;
            const char* m_begin;
            /* const char* m_end; */
            const Assets::Palette& m_palette;
            const FileSystem& m_fs;
        public:
            Md2Parser(const String& name, const char* begin, const char* end, const Assets::Palette& palette, const FileSystem& fs);
        private:
            Assets::EntityModel* doParseModel() override;
            Md2SkinList parseSkins(const char* begin, size_t skinCount);
            Md2FrameList parseFrames(const char* begin, size_t frameCount, size_t frameVertexCount);
            Md2MeshList parseMeshes(const char* begin, size_t commandCount);
            Assets::EntityModel* buildModel(const Md2SkinList& skins, const Md2FrameList& frames, const Md2MeshList& meshes);
            Assets::TextureList loadTextures(const Md2SkinList& skins);
            Assets::Texture* readTexture(const Md2Skin& skin);
            Assets::Md2Model::FrameList buildFrames(const Md2FrameList& frames, const Md2MeshList& meshes);
            Assets::Md2Model::Frame* buildFrame(const Md2Frame& frame, const Md2MeshList& meshes);
            Assets::Md2Model::VertexList getVertices(const Md2Frame& frame, const Md2MeshVertexList& meshVertices) const;
        };
    }
}

#endif /* defined(TrenchBroom_Md2Parser) */
