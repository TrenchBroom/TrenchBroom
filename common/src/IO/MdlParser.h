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

#ifndef TrenchBroom_MdlParser
#define TrenchBroom_MdlParser

#include "StringUtils.h"
#include "ByteBuffer.h"
#include "Assets/AssetTypes.h"
#include "IO/EntityModelParser.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class MdlModel;
        class MdlFrame;
        class Palette;
    }
    
    namespace IO {
        class MdlParser : public EntityModelParser {
        private:
            static const vm::vec3f Normals[162];
            
            struct MdlSkinVertex {
                bool onseam;
                int s;
                int t;
            };
            
            struct MdlSkinTriangle {
                bool front;
                size_t vertices[3];
            };
            
            typedef std::vector<MdlSkinVertex> MdlSkinVertexList;
            typedef std::vector<MdlSkinTriangle> MdlSkinTriangleList;
            typedef vm::vec<unsigned char, 4> PackedFrameVertex;
            typedef std::vector<PackedFrameVertex> PackedFrameVertexList;
            
            String m_name;
            const char* m_begin;
            const char* m_end;
            const Assets::Palette& m_palette;
        public:
            MdlParser(const String& name, const char* begin, const char* end, const Assets::Palette& palette);
        private:
            Assets::EntityModel* doParseModel() override;
            
            void parseSkins(const char*& cursor, Assets::MdlModel& model, size_t count, size_t width, size_t height, int flags);
            MdlSkinVertexList parseSkinVertices(const char*& cursor, size_t count);
            MdlSkinTriangleList parseSkinTriangles(const char*& cursor, size_t count);
            void parseFrames(const char*& cursor, Assets::MdlModel& model, size_t count, const MdlSkinTriangleList& skinTriangles, const MdlSkinVertexList& skinVertices, size_t skinWidth, size_t skinHeight, const vm::vec3f& origin, const vm::vec3f& scale);
            Assets::MdlFrame* parseFrame(const char*& cursor, const MdlSkinTriangleList& skinTriangles, const MdlSkinVertexList& skinVertices, size_t skinWidth, size_t skinHeight, const vm::vec3f& origin, const vm::vec3f& scale);
            vm::vec3f unpackFrameVertex(const PackedFrameVertex& vertex, const vm::vec3f& origin, const vm::vec3f& scale) const;
        };
    }
}

#endif /* defined(TrenchBroom_MdlParser) */
