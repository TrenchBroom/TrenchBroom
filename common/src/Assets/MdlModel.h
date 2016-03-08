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

#ifndef TrenchBroom_MdlModel
#define TrenchBroom_MdlModel

#include "VecMath.h"
#include "StringUtils.h"
#include "Assets/AssetTypes.h"
#include "Assets/EntityModel.h"
#include "Assets/TextureCollection.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/Vertex.h"
#include "Renderer/IndexRangeMap.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        typedef std::vector<float> MdlTimeList;
        
        class MdlSkin {
        private:
            TextureCollection m_textures;
            MdlTimeList m_times;
        public:
            MdlSkin(Texture* texture);
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
            BBox3f m_bounds;
        public:
            MdlFrame(const String& name, const VertexList& triangles, const BBox3f& bounds);
            const MdlFrame* firstFrame() const;
            const VertexList& triangles() const;
            BBox3f bounds() const;
            BBox3f transformedBounds(const Mat4x4f& transformation) const;
        };
        
        class MdlFrameGroup : public MdlBaseFrame {
        private:
            typedef std::vector<MdlFrame*> SingleFrameList;
            
            MdlTimeList m_times;
            SingleFrameList m_frames;
        public:
            ~MdlFrameGroup();
            const MdlFrame* firstFrame() const;
            void addFrame(MdlFrame* frame, const float time);
        };
        
        class MdlModel : public EntityModel {
        private:
            typedef std::vector<MdlSkin*> MdlSkinList;
            typedef std::vector<MdlBaseFrame*> MdlFrameList;
            
            String m_name;
            MdlSkinList m_skins;
            MdlFrameList m_frames;
        public:
            MdlModel(const String& name);
            ~MdlModel();
            
            void addSkin(MdlSkin* skin);
            void addFrame(MdlBaseFrame* frame);
        private:
            Renderer::TexturedIndexRangeRenderer* doBuildRenderer(const size_t skinIndex, const size_t frameIndex) const;
            BBox3f doGetBounds(const size_t skinIndex, const size_t frameIndex) const;
            BBox3f doGetTransformedBounds(const size_t skinIndex, const size_t frameIndex, const Mat4x4f& transformation) const;
            void doPrepare(int minFilter, int magFilter);
            void doSetTextureMode(int minFilter, int magFilter);
        };
    }
}

#endif /* defined(TrenchBroom_MdlModel) */
