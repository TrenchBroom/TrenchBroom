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

#include "Bsp29Model.h"

#include "CollectionUtils.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/TexturedIndexRangeMapBuilder.h"
#include "Renderer/TexturedIndexRangeRenderer.h"

#include <vecmath/forward.h>
#include <vecmath/bbox.h>

#include <cassert>

namespace TrenchBroom {
    namespace Assets {
        Bsp29Model::Face::Face(Assets::Texture* texture, const size_t vertexCount) :
        m_texture(texture),
        m_vertices(0) {
            m_vertices.reserve(vertexCount);
        }
        
        void Bsp29Model::Face::addVertex(const vm::vec3f& vertex, const vm::vec2f& texCoord) {
            m_vertices.push_back(Vertex(vertex, texCoord));
        }

        Assets::Texture* Bsp29Model::Face::texture() const {
            return m_texture;
        }
        
        const Bsp29Model::Face::VertexList& Bsp29Model::Face::vertices() const {
            return m_vertices;
        }

        Bsp29Model::SubModel::SubModel(const FaceList& i_faces, const vm::bbox3f& i_bounds) :
        faces(i_faces),
        bounds(i_bounds) {}

        vm::bbox3f Bsp29Model::SubModel::transformedBounds(const vm::mat4x4f& transformation) const {
            vm::bbox3f result;
            result.min = result.max = faces.front().vertices().front().v1;
            
            for (const Face& face : faces) {
                for (const Face::Vertex& vertex : face.vertices()) {
                    result = merge(result, vertex.v1);
                }
            }
            
            return result;
        }

        Bsp29Model::Bsp29Model(const String& name, TextureCollection* textureCollection) :
        m_name(name),
        m_textureCollection(textureCollection) {
            ensure(textureCollection != nullptr, "textureCollection is null");
        }

        Bsp29Model::~Bsp29Model() {
            delete m_textureCollection;
            m_textureCollection = nullptr;
        }
        
        void Bsp29Model::addModel(const FaceList& faces, const vm::bbox3f& bounds) {
            m_subModels.push_back(SubModel(faces, bounds));
        }

        Renderer::TexturedIndexRangeRenderer* Bsp29Model::doBuildRenderer(const size_t skinIndex, const size_t frameIndex) const {
            const SubModel& model = m_subModels.front();

            size_t vertexCount = 0;
            Renderer::TexturedIndexRangeMap::Size size;
            
            for (const Face& face : model.faces) {
                const size_t faceVertexCount = face.vertices().size();
                size.inc(face.texture(), GL_POLYGON, faceVertexCount);
                vertexCount += faceVertexCount;
            }

            Renderer::TexturedIndexRangeMapBuilder<Face::Vertex::Spec> builder(vertexCount, size);
            for (const Face& face : model.faces)
                builder.addPolygon(face.texture(), face.vertices());

            const Renderer::VertexArray vertexArray = Renderer::VertexArray::swap(builder.vertices());
            const Renderer::TexturedIndexRangeMap& indexArray = builder.indices();
            return new Renderer::TexturedIndexRangeRenderer(vertexArray, indexArray);
        }

        vm::bbox3f Bsp29Model::doGetBounds(const size_t /* skinIndex */, const size_t /* frameIndex */) const {
            const SubModel& model = m_subModels.front();
            return model.bounds;
        }

        vm::bbox3f Bsp29Model::doGetTransformedBounds(const size_t /* skinIndex */, const size_t /* frameIndex */, const vm::mat4x4f& transformation) const {
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
