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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__MdlModel__
#define __TrenchBroom__MdlModel__

#include "VecMath.h"
#include "StringUtils.h"
#include "Assets/AssetTypes.h"
#include "Assets/EntityModel.h"
#include "Renderer/VertexSpec.h"
#include "Renderer/Vertex.h"

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class MeshRenderer;
        class Vbo;
    }
    
    namespace Assets {
        typedef std::vector<float> MdlTimeList;
        
        class MdlSkin {
        private:
            TextureList m_textures;
            MdlTimeList m_times;
        public:
            MdlSkin(Texture* texture);
            MdlSkin(const TextureList& textures, const MdlTimeList times);
            ~MdlSkin();
            
            const Texture* firstPicture() const;
        };

        typedef Renderer::VertexSpecs::P3T2::Vertex MdlFrameVertex;
        typedef MdlFrameVertex::List MdlFrameVertexList;
        
        class MdlFrame;
        
        class MdlBaseFrame {
        public:
            virtual ~MdlBaseFrame();
            virtual const MdlFrame* firstFrame() const = 0;
        };
        
        class MdlFrame : public MdlBaseFrame {
        private:
            String m_name;
            MdlFrameVertexList m_triangles;
            BBox3f m_bounds;
        public:
            MdlFrame(const String& name, const MdlFrameVertexList& triangles, const BBox3f& bounds);
            const MdlFrame* firstFrame() const;
            const MdlFrameVertexList& triangles() const;
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
            Renderer::MeshRenderer* doBuildRenderer(Renderer::Vbo& vbo, const size_t skinIndex, const size_t frameIndex) const;
            BBox3f doGetBounds(const size_t skinIndex, const size_t frameIndex) const;
            BBox3f doGetTransformedBounds(const size_t skinIndex, const size_t frameIndex, const Mat4x4f& transformation) const;
        };
    }
}

#endif /* defined(__TrenchBroom__MdlModel__) */
