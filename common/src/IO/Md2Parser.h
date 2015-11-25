/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
        class GameFileSystem;
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
            static const Vec3f Normals[162];

            struct Md2Skin {
                char name[Md2Layout::SkinNameLength];
            };
            typedef std::vector<Md2Skin> Md2SkinList;
            
            struct Md2Vertex {
                unsigned char x, y, z;
                unsigned char normalIndex;
            };
            typedef std::vector<Md2Vertex> Md2VertexList;
            
            struct Md2Frame {
                Vec3f scale;
                Vec3f offset;
                char name[Md2Layout::FrameNameLength];
                Md2VertexList vertices;
                
                Md2Frame(size_t vertexCount);
                Vec3f vertex(size_t index) const;
                const Vec3f& normal(size_t index) const;
            };
            typedef std::vector<Md2Frame> Md2FrameList;

            struct Md2MeshVertex {
                Vec2f texCoords;
                size_t vertexIndex;
            };
            typedef std::vector<Md2MeshVertex> Md2MeshVertexList;
            
            struct Md2Mesh {
                enum Type {
                    Fan,
                    Strip
                };
                
                Type type;
                size_t vertexCount;
                Md2MeshVertexList vertices;
                
                Md2Mesh(int i_vertexCount);
            };
            typedef std::vector<Md2Mesh> Md2MeshList;
            
            
            String m_name;
            const char* m_begin;
            /* const char* m_end; */
            const Assets::Palette& m_palette;
            const GameFileSystem& m_fs;
        public:
            Md2Parser(const String& name, const char* begin, const char* end, const Assets::Palette& palette, const GameFileSystem& fs);
        private:
            Assets::EntityModel* doParseModel();
            Md2SkinList parseSkins(const char* begin, const size_t skinCount);
            Md2FrameList parseFrames(const char* begin, const size_t frameCount, const size_t frameVertexCount);
            Md2MeshList parseMeshes(const char* begin, const size_t commandCount);
            Assets::EntityModel* buildModel(const Md2SkinList& skins, const Md2FrameList& frames, const Md2MeshList& meshes);
            Assets::TextureList loadTextures(const Md2SkinList& skins);
            Assets::Texture* loadTexture(const Md2Skin& skin);
            Assets::Md2Model::FrameList buildFrames(const Md2FrameList& frames, const Md2MeshList& meshes);
            Assets::Md2Model::Frame* buildFrame(const Md2Frame& frame, const Md2MeshList& meshes);
            Assets::Md2Model::VertexList getVertices(const Md2Frame& frame, const Md2MeshVertexList& meshVertices) const;
        };
    }
}

#endif /* defined(TrenchBroom_Md2Parser) */
