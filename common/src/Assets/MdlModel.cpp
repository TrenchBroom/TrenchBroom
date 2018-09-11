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

#include "MdlModel.h"

#include "CollectionUtils.h"
#include "Assets/Texture.h"
#include "Assets/Texture.h"
#include "Renderer/TexturedIndexRangeMap.h"
#include "Renderer/TexturedIndexRangeMapBuilder.h"
#include "Renderer/TexturedIndexRangeRenderer.h"

#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/bbox.h>

#include <cassert>

namespace TrenchBroom {
    namespace Assets {
        MdlSkin::MdlSkin(Texture* texture) :
        m_textures(TextureList(1, texture)) {
            m_times.push_back(0.0f);
        }
        
        MdlSkin::MdlSkin(const TextureList& textures, const MdlTimeList& times) :
        m_textures(textures),
        m_times(times) {
            assert(textures.size() == times.size());
        }
        
        void MdlSkin::prepare(const int minFilter, const int magFilter) {
            m_textures.prepare(minFilter, magFilter);
        }

        void MdlSkin::setTextureMode(const int minFilter, const int magFilter) {
            m_textures.setTextureMode(minFilter, magFilter);
        }

        const Texture* MdlSkin::firstPicture() const {
            return m_textures.textures().front();
        }

        MdlBaseFrame::~MdlBaseFrame() {}

        MdlFrame::MdlFrame(const String& name, const VertexList& triangles, const vm::bbox3f& bounds) :
        m_name(name),
        m_triangles(triangles),
        m_bounds(bounds) {}
        
        const MdlFrame* MdlFrame::firstFrame() const {
            return this;
        }

        const MdlFrame::VertexList& MdlFrame::triangles() const {
            return m_triangles;
        }

        vm::bbox3f MdlFrame::bounds() const {
            return m_bounds;
        }

        vm::bbox3f MdlFrame::transformedBounds(const vm::mat4x4f& transformation) const {
            if (m_triangles.empty()) {
                return vm::bbox3f(-8.0f, 8.0f);
            }

            auto it = std::begin(m_triangles);
            auto end = std::end(m_triangles);
            
            vm::bbox3f bounds;
            bounds.min = bounds.max = transformation * it->v1;
            ++it;
            
            while (it != end) {
                bounds = merge(bounds, transformation * it->v1);
                ++it;
            }
            
            return bounds;
        }

        MdlFrameGroup::~MdlFrameGroup() {
            VectorUtils::clearAndDelete(m_frames);
        }
        
        const MdlFrame* MdlFrameGroup::firstFrame() const {
            if (m_frames.empty()) {
                return nullptr;
            } else {
                return m_frames[0]->firstFrame();
            }
        }
        
        void MdlFrameGroup::addFrame(MdlFrame* frame, const float time) {
            m_frames.push_back(frame);
            m_times.push_back(time);
        }

        MdlModel::MdlModel(const String& name) :
        m_name(name) {}

        MdlModel::~MdlModel() {
            VectorUtils::clearAndDelete(m_skins);
            VectorUtils::clearAndDelete(m_frames);
        }

        void MdlModel::addSkin(MdlSkin* skin) {
            m_skins.push_back(skin);
        }

        void MdlModel::addFrame(MdlBaseFrame* frame) {
            m_frames.push_back(frame);
        }

        Renderer::TexturedIndexRangeRenderer* MdlModel::doBuildRenderer(const size_t skinIndex, const size_t frameIndex) const {
            if (skinIndex >= m_skins.size()) {
                return nullptr;
            }
            if (frameIndex >= m_frames.size()) {
                return nullptr;
            }

            const auto* skin = m_skins[skinIndex];
            const auto* frame = m_frames[frameIndex]->firstFrame();

            const auto* texture = skin->firstPicture();
            const auto& vertices = frame->triangles();
            const auto vertexCount = vertices.size();
            
            const auto vertexArray = Renderer::VertexArray::ref(vertices);
            const Renderer::TexturedIndexRangeMap indexArray(texture, GL_TRIANGLES, 0, vertexCount);
            
            return new Renderer::TexturedIndexRangeRenderer(vertexArray, indexArray);
        }

        vm::bbox3f MdlModel::doGetBounds(const size_t /* skinIndex */, const size_t frameIndex) const {
            if (frameIndex >= m_frames.size()) {
                return vm::bbox3f(-8.0f, 8.0f);
            } else {
                const auto* frame = m_frames[frameIndex]->firstFrame();
                return frame->bounds();
            }
        }

        vm::bbox3f MdlModel::doGetTransformedBounds(const size_t /* skinIndex */, const size_t frameIndex, const vm::mat4x4f& transformation) const {
            if (frameIndex >= m_frames.size()) {
                return vm::bbox3f(-8.0f, 8.0f);
            } else {
                const auto* frame = m_frames[frameIndex]->firstFrame();
                return frame->transformedBounds(transformation);
            };
        }

        
        void MdlModel::doPrepare(const int minFilter, const int magFilter) {
            for (size_t i = 0; i < m_skins.size(); ++i) {
                m_skins[i]->prepare(minFilter, magFilter);
            }
        }

        void MdlModel::doSetTextureMode(const int minFilter, const int magFilter) {
            for (size_t i = 0; i < m_skins.size(); ++i) {
                m_skins[i]->setTextureMode(minFilter, magFilter);
            }
        }
    }
}
