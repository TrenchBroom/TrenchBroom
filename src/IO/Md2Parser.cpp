/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "Md2Parser.h"

#include "Exceptions.h"
#include "Assets/AutoTexture.h"
#include "Assets/Md2Model.h"
#include "Assets/Palette.h"
#include "IO/GameFS.h"
#include "IO/ImageLoader.h"
#include "IO/IOUtils.h"
#include "IO/MappedFile.h"
#include "IO/Path.h"
#include "Renderer/Mesh.h"
#include "Renderer/Vertex.h"
#include "Renderer/VertexSpec.h"

namespace TrenchBroom {
    namespace IO {
        const Vec3f Md2Parser::Normals[162] = {
            Vec3f(-0.525731, 0.000000, 0.850651),
            Vec3f(-0.442863, 0.238856, 0.864188),
            Vec3f(-0.295242, 0.000000, 0.955423),
            Vec3f(-0.309017, 0.500000, 0.809017),
            Vec3f(-0.162460, 0.262866, 0.951056),
            Vec3f(0.000000, 0.000000, 1.000000),
            Vec3f(0.000000, 0.850651, 0.525731),
            Vec3f(-0.147621, 0.716567, 0.681718),
            Vec3f(0.147621, 0.716567, 0.681718),
            Vec3f(0.000000, 0.525731, 0.850651),
            Vec3f(0.309017, 0.500000, 0.809017),
            Vec3f(0.525731, 0.000000, 0.850651),
            Vec3f(0.295242, 0.000000, 0.955423),
            Vec3f(0.442863, 0.238856, 0.864188),
            Vec3f(0.162460, 0.262866, 0.951056),
            Vec3f(-0.681718, 0.147621, 0.716567),
            Vec3f(-0.809017, 0.309017, 0.500000),
            Vec3f(-0.587785, 0.425325, 0.688191),
            Vec3f(-0.850651, 0.525731, 0.000000),
            Vec3f(-0.864188, 0.442863, 0.238856),
            Vec3f(-0.716567, 0.681718, 0.147621),
            Vec3f(-0.688191, 0.587785, 0.425325),
            Vec3f(-0.500000, 0.809017, 0.309017),
            Vec3f(-0.238856, 0.864188, 0.442863),
            Vec3f(-0.425325, 0.688191, 0.587785),
            Vec3f(-0.716567, 0.681718, -0.147621),
            Vec3f(-0.500000, 0.809017, -0.309017),
            Vec3f(-0.525731, 0.850651, 0.000000),
            Vec3f(0.000000, 0.850651, -0.525731),
            Vec3f(-0.238856, 0.864188, -0.442863),
            Vec3f(0.000000, 0.955423, -0.295242),
            Vec3f(-0.262866, 0.951056, -0.162460),
            Vec3f(0.000000, 1.000000, 0.000000),
            Vec3f(0.000000, 0.955423, 0.295242),
            Vec3f(-0.262866, 0.951056, 0.162460),
            Vec3f(0.238856, 0.864188, 0.442863),
            Vec3f(0.262866, 0.951056, 0.162460),
            Vec3f(0.500000, 0.809017, 0.309017),
            Vec3f(0.238856, 0.864188, -0.442863),
            Vec3f(0.262866, 0.951056, -0.162460),
            Vec3f(0.500000, 0.809017, -0.309017),
            Vec3f(0.850651, 0.525731, 0.000000),
            Vec3f(0.716567, 0.681718, 0.147621),
            Vec3f(0.716567, 0.681718, -0.147621),
            Vec3f(0.525731, 0.850651, 0.000000),
            Vec3f(0.425325, 0.688191, 0.587785),
            Vec3f(0.864188, 0.442863, 0.238856),
            Vec3f(0.688191, 0.587785, 0.425325),
            Vec3f(0.809017, 0.309017, 0.500000),
            Vec3f(0.681718, 0.147621, 0.716567),
            Vec3f(0.587785, 0.425325, 0.688191),
            Vec3f(0.955423, 0.295242, 0.000000),
            Vec3f(1.000000, 0.000000, 0.000000),
            Vec3f(0.951056, 0.162460, 0.262866),
            Vec3f(0.850651, -0.525731, 0.000000),
            Vec3f(0.955423, -0.295242, 0.000000),
            Vec3f(0.864188, -0.442863, 0.238856),
            Vec3f(0.951056, -0.162460, 0.262866),
            Vec3f(0.809017, -0.309017, 0.500000),
            Vec3f(0.681718, -0.147621, 0.716567),
            Vec3f(0.850651, 0.000000, 0.525731),
            Vec3f(0.864188, 0.442863, -0.238856),
            Vec3f(0.809017, 0.309017, -0.500000),
            Vec3f(0.951056, 0.162460, -0.262866),
            Vec3f(0.525731, 0.000000, -0.850651),
            Vec3f(0.681718, 0.147621, -0.716567),
            Vec3f(0.681718, -0.147621, -0.716567),
            Vec3f(0.850651, 0.000000, -0.525731),
            Vec3f(0.809017, -0.309017, -0.500000),
            Vec3f(0.864188, -0.442863, -0.238856),
            Vec3f(0.951056, -0.162460, -0.262866),
            Vec3f(0.147621, 0.716567, -0.681718),
            Vec3f(0.309017, 0.500000, -0.809017),
            Vec3f(0.425325, 0.688191, -0.587785),
            Vec3f(0.442863, 0.238856, -0.864188),
            Vec3f(0.587785, 0.425325, -0.688191),
            Vec3f(0.688191, 0.587785, -0.425325),
            Vec3f(-0.147621, 0.716567, -0.681718),
            Vec3f(-0.309017, 0.500000, -0.809017),
            Vec3f(0.000000, 0.525731, -0.850651),
            Vec3f(-0.525731, 0.000000, -0.850651),
            Vec3f(-0.442863, 0.238856, -0.864188),
            Vec3f(-0.295242, 0.000000, -0.955423),
            Vec3f(-0.162460, 0.262866, -0.951056),
            Vec3f(0.000000, 0.000000, -1.000000),
            Vec3f(0.295242, 0.000000, -0.955423),
            Vec3f(0.162460, 0.262866, -0.951056),
            Vec3f(-0.442863, -0.238856, -0.864188),
            Vec3f(-0.309017, -0.500000, -0.809017),
            Vec3f(-0.162460, -0.262866, -0.951056),
            Vec3f(0.000000, -0.850651, -0.525731),
            Vec3f(-0.147621, -0.716567, -0.681718),
            Vec3f(0.147621, -0.716567, -0.681718),
            Vec3f(0.000000, -0.525731, -0.850651),
            Vec3f(0.309017, -0.500000, -0.809017),
            Vec3f(0.442863, -0.238856, -0.864188),
            Vec3f(0.162460, -0.262866, -0.951056),
            Vec3f(0.238856, -0.864188, -0.442863),
            Vec3f(0.500000, -0.809017, -0.309017),
            Vec3f(0.425325, -0.688191, -0.587785),
            Vec3f(0.716567, -0.681718, -0.147621),
            Vec3f(0.688191, -0.587785, -0.425325),
            Vec3f(0.587785, -0.425325, -0.688191),
            Vec3f(0.000000, -0.955423, -0.295242),
            Vec3f(0.000000, -1.000000, 0.000000),
            Vec3f(0.262866, -0.951056, -0.162460),
            Vec3f(0.000000, -0.850651, 0.525731),
            Vec3f(0.000000, -0.955423, 0.295242),
            Vec3f(0.238856, -0.864188, 0.442863),
            Vec3f(0.262866, -0.951056, 0.162460),
            Vec3f(0.500000, -0.809017, 0.309017),
            Vec3f(0.716567, -0.681718, 0.147621),
            Vec3f(0.525731, -0.850651, 0.000000),
            Vec3f(-0.238856, -0.864188, -0.442863),
            Vec3f(-0.500000, -0.809017, -0.309017),
            Vec3f(-0.262866, -0.951056, -0.162460),
            Vec3f(-0.850651, -0.525731, 0.000000),
            Vec3f(-0.716567, -0.681718, -0.147621),
            Vec3f(-0.716567, -0.681718, 0.147621),
            Vec3f(-0.525731, -0.850651, 0.000000),
            Vec3f(-0.500000, -0.809017, 0.309017),
            Vec3f(-0.238856, -0.864188, 0.442863),
            Vec3f(-0.262866, -0.951056, 0.162460),
            Vec3f(-0.864188, -0.442863, 0.238856),
            Vec3f(-0.809017, -0.309017, 0.500000),
            Vec3f(-0.688191, -0.587785, 0.425325), 
            Vec3f(-0.681718, -0.147621, 0.716567), 
            Vec3f(-0.442863, -0.238856, 0.864188), 
            Vec3f(-0.587785, -0.425325, 0.688191), 
            Vec3f(-0.309017, -0.500000, 0.809017), 
            Vec3f(-0.147621, -0.716567, 0.681718), 
            Vec3f(-0.425325, -0.688191, 0.587785), 
            Vec3f(-0.162460, -0.262866, 0.951056), 
            Vec3f(0.442863, -0.238856, 0.864188), 
            Vec3f(0.162460, -0.262866, 0.951056), 
            Vec3f(0.309017, -0.500000, 0.809017), 
            Vec3f(0.147621, -0.716567, 0.681718), 
            Vec3f(0.000000, -0.525731, 0.850651), 
            Vec3f(0.425325, -0.688191, 0.587785), 
            Vec3f(0.587785, -0.425325, 0.688191), 
            Vec3f(0.688191, -0.587785, 0.425325), 
            Vec3f(-0.955423, 0.295242, 0.000000), 
            Vec3f(-0.951056, 0.162460, 0.262866), 
            Vec3f(-1.000000, 0.000000, 0.000000), 
            Vec3f(-0.850651, 0.000000, 0.525731), 
            Vec3f(-0.955423, -0.295242, 0.000000), 
            Vec3f(-0.951056, -0.162460, 0.262866), 
            Vec3f(-0.864188, 0.442863, -0.238856), 
            Vec3f(-0.951056, 0.162460, -0.262866), 
            Vec3f(-0.809017, 0.309017, -0.500000), 
            Vec3f(-0.864188, -0.442863, -0.238856), 
            Vec3f(-0.951056, -0.162460, -0.262866), 
            Vec3f(-0.809017, -0.309017, -0.500000), 
            Vec3f(-0.681718, 0.147621, -0.716567), 
            Vec3f(-0.681718, -0.147621, -0.716567), 
            Vec3f(-0.850651, 0.000000, -0.525731), 
            Vec3f(-0.688191, 0.587785, -0.425325), 
            Vec3f(-0.587785, 0.425325, -0.688191), 
            Vec3f(-0.425325, 0.688191, -0.587785), 
            Vec3f(-0.425325, -0.688191, -0.587785), 
            Vec3f(-0.587785, -0.425325, -0.688191), 
            Vec3f(-0.688191, -0.587785, -0.425325)
        };
        
        Md2Parser::Md2Frame::Md2Frame(const size_t vertexCount) :
        vertices(vertexCount) {}

        Vec3f Md2Parser::Md2Frame::vertex(const size_t index) const {
            const Md2Vertex& vertex = vertices[index];
            return Vec3f(static_cast<float>(vertex.position[0]) * scale[0] + offset[0],
                         static_cast<float>(vertex.position[1]) * scale[1] + offset[1],
                         static_cast<float>(vertex.position[2]) * scale[2] + offset[2]);
        }

        const Vec3f& Md2Parser::Md2Frame::normal(const size_t index) const {
            const Md2Vertex& vertex = vertices[index];
            return Normals[vertex.normalIndex];
        }

        Md2Parser::Md2Mesh::Md2Mesh(const int i_vertexCount) :
        type(i_vertexCount < 0 ? Fan : Strip),
        vertexCount(static_cast<size_t>(i_vertexCount < 0 ? -i_vertexCount : i_vertexCount)),
        vertices(vertexCount) {}

        Md2Parser::Md2Parser(const String& name, const char* begin, const char* end, const Assets::Palette& palette, const GameFS& fs, const Path& textureBasePath) :
        m_name(name),
        m_begin(begin),
        m_end(end),
        m_palette(palette),
        m_fs(fs),
        m_textureBasePath(textureBasePath) {}
        
        Assets::EntityModel* Md2Parser::doParseModel() {
            const char* cursor = m_begin;
            const int ident = readInt<int32_t>(cursor);
            const int version = readInt<int32_t>(cursor);
            
            if (ident != Md2Layout::Ident)
                throw AssetException() << "Unknown MD2 model ident: " << ident;
            if (version != Md2Layout::Version)
                throw AssetException() << "Unknown MD2 model version: " << version;
            
            /*const size_t skinWidth =*/ readSize<int32_t>(cursor);
            /*const size_t skinHeight =*/ readSize<int32_t>(cursor);
            /*const size_t frameSize =*/ readSize<int32_t>(cursor);
            
            const size_t skinCount = readSize<int32_t>(cursor);
            const size_t frameVertexCount = readSize<int32_t>(cursor);
            /* const size_t texCoordCount =*/ readSize<int32_t>(cursor);
            /* const size_t triangleCount =*/ readSize<int32_t>(cursor);
            const size_t commandCount = readSize<int32_t>(cursor);
            const size_t frameCount = readSize<int32_t>(cursor);
            
            const size_t skinOffset = readSize<int32_t>(cursor);
            /* const size_t texCoordOffset =*/ readSize<int32_t>(cursor);
            /* const size_t triangleOffset =*/ readSize<int32_t>(cursor);
            const size_t frameOffset = readSize<int32_t>(cursor);
            const size_t commandOffset = readSize<int32_t>(cursor);

            const Md2SkinList skins = parseSkins(m_begin + skinOffset, skinCount);
            const Md2FrameList frames = parseFrames(m_begin + frameOffset, frameCount, frameVertexCount);
            const Md2MeshList meshes = parseMeshes(m_begin + commandOffset, commandCount);
            
            return buildModel(skins, frames, meshes);
        }

        Md2Parser::Md2SkinList Md2Parser::parseSkins(const char* begin, const size_t skinCount) {
            Md2SkinList skins(skinCount);
            readVector(begin, skins);
            return skins;
        }

        Md2Parser::Md2FrameList Md2Parser::parseFrames(const char* begin, const size_t frameCount, const size_t frameVertexCount) {
            Md2FrameList frames(frameCount, Md2Frame(frameVertexCount));

            const char* cursor = begin;
            for (size_t i = 0; i < frameCount; ++i) {
                frames[i].scale = readVec3f(cursor);
                frames[i].offset = readVec3f(cursor);
                readBytes(cursor, frames[i].name, Md2Layout::FrameNameLength);
                readVector(cursor, frames[i].vertices);
            }
            
            return frames;
        }

        Md2Parser::Md2MeshList Md2Parser::parseMeshes(const char* begin, const size_t commandCount) {
            Md2MeshList meshes;
            
            const char* cursor = begin;
            while (cursor < begin + commandCount) {
                Md2Mesh mesh(readInt<int32_t>(cursor));
                for (size_t i = 0; i < mesh.vertexCount; ++i) {
                    mesh.vertices[i].texCoords[0] = readFloat<float>(cursor);
                    mesh.vertices[i].texCoords[1] = readFloat<float>(cursor);
                    mesh.vertices[i].vertexIndex = readSize<int32_t>(cursor);
                }
                meshes.push_back(mesh);
            }
            
            return meshes;
        }

        Assets::EntityModel* Md2Parser::buildModel(const Md2SkinList& skins, const Md2FrameList& frames, const Md2MeshList& meshes) {
            Assets::Md2Model* model = new Assets::Md2Model(m_name);
            
            Md2SkinList::const_iterator skinIt, skinEnd;
            for (skinIt = skins.begin(), skinEnd = skins.end(); skinIt != skinEnd; ++skinIt)
                addSkinToModel(*model, *skinIt);
            
            Md2FrameList::const_iterator frameIt, frameEnd;
            for (frameIt = frames.begin(), frameEnd = frames.end(); frameIt != frameEnd; ++frameIt)
                addFrameToModel(*model, *frameIt, meshes);
            
            return model;
        }

        void Md2Parser::addSkinToModel(Assets::Md2Model& model, const Md2Skin& skin) {
            const Path skinPath(String(skin.name));
            MappedFile::Ptr file = m_fs.findFile(skinPath);
            if (file != NULL) {
                Color avgColor;
                const ImageLoader image(ImageLoader::PCX, file->begin(), file->end());
                
                const Buffer<unsigned char>& indices = image.indices();
                Buffer<unsigned char> rgbImage(indices.size() * 3);
                m_palette.indexedToRgb(indices, indices.size(), rgbImage, avgColor);
                
                Assets::AutoTexture* texture = new Assets::AutoTexture(image.width(), image.height(), rgbImage);
                model.addSkin(texture);
            }
        }
        
        void Md2Parser::addFrameToModel(Assets::Md2Model& model, const Md2Frame& frame, const Md2MeshList& meshes) {
            typedef Assets::Md2Model::Vertex Vertex;
            typedef Assets::Md2Model::Mesh Mesh;
            
            Mesh::TriangleSeries triangleFans;
            Mesh::TriangleSeries triangleStrips;
            
            Md2MeshList::const_iterator mIt, mEnd;
            for (mIt = meshes.begin(), mEnd = meshes.end(); mIt != mEnd; ++mIt) {
                const Md2Mesh& md2Mesh = *mIt;
                Vertex::List triangles;
                
                Md2MeshVertexList::const_iterator vIt, vEnd;
                for (vIt = md2Mesh.vertices.begin(), vEnd = md2Mesh.vertices.end(); vIt != vEnd; ++vIt) {
                    const Md2MeshVertex& md2MeshVertex = *vIt;
                    
                    const Vec3f position = frame.vertex(md2MeshVertex.vertexIndex);
                    const Vec3f& normal = frame.normal(md2MeshVertex.vertexIndex);
                    const Vec2f& texCoords = md2MeshVertex.texCoords;
                    
                    triangles.push_back(Vertex(position, normal, texCoords));
                }
                
                if (md2Mesh.type == Md2Mesh::Strip)
                    triangleStrips.push_back(triangles);
                else
                    triangleFans.push_back(triangles);
            }
            
            model.addFrame(triangleFans, triangleStrips);
        }
    }
}
