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

#ifndef TrenchBroom_MdlModel
#define TrenchBroom_MdlModel

#include "StringUtils.h"
#include "Assets/AssetTypes.h"
#include "Assets/EntityModel.h"
#include "Assets/TextureCollection.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/Vertex.h"
#include "Renderer/IndexRangeMap.h"

#include <vecmath/forward.h>
#include <vecmath/bbox.h>

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        typedef std::vector<float> MdlTimeList;
        
        class MdlSkin {
        private:
            TextureCollection m_textures;
            MdlTimeList m_times;
        public:
            explicit MdlSkin(Texture* texture);
            MdlSkin(const TextureList& textures, const MdlTimeList& times);
            
            void prepare(int minFilter, int magFilter);
            void setTextureMode(int minFilter, int magFilter);
            const Texture* firstPicture() const;
        };

        class MdlFrame;
        
        class MdlBaseFrame {
        public:
            virtual ~MdlBaseFrame();
            virtual const MdlFrame* firstFrame() const = 0;
        };
        
        class MdlFrame : public MdlBaseFrame {
        public:
            typedef Renderer::VertexSpecs::P3T2::Vertex Vertex;
            typedef Vertex::List VertexList;
        private:
            String m_name;
            VertexList m_triangles;
            vm::bbox3f m_bounds;
        public:
            MdlFrame(const String& name, const VertexList& triangles, const vm::bbox3f& bounds);
            const MdlFrame* firstFrame() const override;
            const VertexList& triangles() const;
            vm::bbox3f bounds() const;
            vm::bbox3f transformedBounds(const vm::mat4x4f& transformation) const;
        };
        
        class MdlFrameGroup : public MdlBaseFrame {
        private:
            typedef std::vector<MdlFrame*> SingleFrameList;
            
            MdlTimeList m_times;
            SingleFrameList m_frames;
        public:
            ~MdlFrameGroup() override;
            const MdlFrame* firstFrame() const override;
            void addFrame(MdlFrame* frame, float time);
        };
        
        class MdlModel : public EntityModel {
        private:
            typedef std::vector<MdlSkin*> MdlSkinList;
            typedef std::vector<MdlBaseFrame*> MdlFrameList;
            
            String m_name;
            MdlSkinList m_skins;
            MdlFrameList m_frames;
        public:
            explicit MdlModel(const String& name);
            ~MdlModel() override;
            
            void addSkin(MdlSkin* skin);
            void addFrame(MdlBaseFrame* frame);
        private:
            Renderer::TexturedIndexRangeRenderer* doBuildRenderer(size_t skinIndex, size_t frameIndex) const override;
            vm::bbox3f doGetBounds(size_t skinIndex, size_t frameIndex) const override;
            vm::bbox3f doGetTransformedBounds(size_t skinIndex, size_t frameIndex, const vm::mat4x4f& transformation) const override;
            void doPrepare(int minFilter, int magFilter) override;
            void doSetTextureMode(int minFilter, int magFilter) override;
        };
    }
}

#endif /* defined(TrenchBroom_MdlModel) */
