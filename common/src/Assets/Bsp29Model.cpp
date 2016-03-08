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

#include "Bsp29Model.h"

#include "CollectionUtils.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/TexturedIndexRangeMapBuilder.h"
#include "Renderer/TexturedIndexRangeRenderer.h"

#include <cassert>

namespace TrenchBroom {
    namespace Assets {
        Bsp29Model::Face::Face(Assets::Texture* texture, const size_t vertexCount) :
        m_texture(texture),
        m_vertices(0) {
            m_vertices.reserve(vertexCount);
        }
        
        void Bsp29Model::Face::addVertex(const Vec3f& vertex, const Vec2f& texCoord) {
            m_vertices.push_back(Vertex(vertex, texCoord));
        }

        Assets::Texture* Bsp29Model::Face::texture() const {
            return m_texture;
        }
        
        const Bsp29Model::Face::VertexList& Bsp29Model::Face::vertices() const {
            return m_vertices;
        }

        Bsp29Model::SubModel::SubModel(const FaceList& i_faces, const BBox3f& i_bounds) :
        faces(i_faces),
        bounds(i_bounds) {}

        BBox3f Bsp29Model::SubModel::transformedBounds(const Mat4x4f& transformation) const {
            BBox3f result;
            result.min = result.max = faces.front().vertices().front().v1;
            
            FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt) {
                const Face& face = *faceIt;
                const Face::VertexList& vertices = face.vertices();
                Face::VertexList::const_iterator vIt, vEnd;
                for (vIt = vertices.begin(), vEnd = vertices.end(); vIt != vEnd; ++vIt)
                    result.mergeWith(vIt->v1);
            }
            
            return result;
        }

        Bsp29Model::Bsp29Model(const String& name, TextureCollection* textureCollection) :
        m_name(name),
        m_textureCollection(textureCollection) {
            assert(textureCollection != NULL);
        }

        Bsp29Model::~Bsp29Model() {
            delete m_textureCollection;
            m_textureCollection = NULL;
        }
        
        void Bsp29Model::addModel(const FaceList& faces, const BBox3f& bounds) {
            m_subModels.push_back(SubModel(faces, bounds));
        }

        Renderer::TexturedIndexRangeRenderer* Bsp29Model::doBuildRenderer(const size_t skinIndex, const size_t frameIndex) const {

            FaceList::const_iterator it, end;
            const SubModel& model = m_subModels.front();

            size_t vertexCount = 0;
            Renderer::TexturedIndexRangeMap::Size size;
            
            for (it = model.faces.begin(), end = model.faces.end(); it != end; ++it) {
                const Face& face = *it;
                const size_t faceVertexCount = face.vertices().size();
                size.inc(face.texture(), GL_POLYGON, faceVertexCount);
                vertexCount += faceVertexCount;
            }

            Renderer::TexturedIndexRangeMapBuilder<Face::Vertex::Spec> builder(vertexCount, size);
            for (it = model.faces.begin(), end = model.faces.end(); it != end; ++it) {
                const Face& face = *it;
                builder.addPolygon(face.texture(), face.vertices());
            }

            const Renderer::VertexArray vertexArray = Renderer::VertexArray::swap(builder.vertices());
            const Renderer::TexturedIndexRangeMap& indexArray = builder.indices();
            return new Renderer::TexturedIndexRangeRenderer(vertexArray, indexArray);
        }

        BBox3f Bsp29Model::doGetBounds(const size_t skinIndex, const size_t frameIndex) const {
            const SubModel& model = m_subModels.front();
            return model.bounds;
        }

        BBox3f Bsp29Model::doGetTransformedBounds(const size_t skinIndex, const size_t frameIndex, const Mat4x4f& transformation) const {
            const SubModel& model = m_subModels.front();
            return model.transformedBounds(transformation);
        }

        void Bsp29Model::doPrepare(const int minFilter, const int magFilter) {
            m_textureCollection->prepare(minFilter, magFilter);
        }

        void Bsp29Model::doSetTextureMode(const int minFilter, const int magFilter) {
            m_textureCollection->setTextureMode(minFilter, magFilter);
        }
    }
}
