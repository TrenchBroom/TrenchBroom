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

#include "MdlParser.h"

#include "CollectionUtils.h"
#include "IO/IOUtils.h"

#include <cassert>

namespace TrenchBroom {
    namespace IO {
        namespace MdlLayout {
            static const unsigned int HeaderScale       = 0x8;
            static const unsigned int HeaderNumSkins    = 0x30;
            static const unsigned int Skins             = 0x54;
            static const unsigned int SimpleFrameName   = 0x8;
            static const unsigned int SimpleFrameLength = 0x10;
            static const unsigned int MultiFrameTimes   = 0xC;
            static const unsigned int FrameVertexSize   = 0x4;
        }

        MdlParser::Skin::~Skin() {
            while (!pictures.empty()) delete [] pictures.back(); pictures.pop_back();
        }

        MdlParser::MdlParser(const String& name, const char* begin, const char* end) :
        m_name(name),
        m_begin(begin),
        m_end(end) {
            assert(begin < end);
        }

        Assets::EntityModelCollection* MdlParser::doParseModel() {
            const char* cursor = m_begin + MdlLayout::HeaderScale;
            const Vec3f scale = readVec3f(cursor);
            const Vec3f origin = readVec3f(cursor);
            
            cursor = m_begin + MdlLayout::HeaderNumSkins;
            const size_t skinCount = readSize<int32_t>(cursor);
            const size_t skinWidth = readSize<int32_t>(cursor);
            const size_t skinHeight = readSize<int32_t>(cursor);
            const size_t skinSize = skinWidth * skinHeight;
            SkinList skins;
            skins.resize(skinCount);
            
            const size_t vertexCount = readSize<int32_t>(cursor);
            const size_t triangleCount = readSize<int32_t>(cursor);
            const size_t frameCount = readSize<int32_t>(cursor);
            
            cursor = m_begin + MdlLayout::Skins;
            for (size_t i = 0; i < skinCount; ++i) {
                const size_t skinGroup = readSize<int32_t>(cursor);
                if (skinGroup == 0) {
                    char* picture = new char[skinSize];
                    readBytes(cursor, picture, skinSize);
                    
                    skins[i].pictures.push_back(picture);
                } else {
                    const size_t pictureCount = readSize<int32_t>(cursor);
                    skins[i].pictures.reserve(pictureCount);
                    skins[i].times.reserve(pictureCount);
                    const char* base = cursor;

                    for (size_t j = 0; j < pictureCount; ++j) {
                        cursor = base + j * sizeof(float);
                        skins[i].times[j] = readFloat<float>(cursor);

                        cursor = base + pictureCount * 4 + j * skinSize;
                        char* picture = new char[skinSize];
                        readBytes(cursor, picture, skinSize);
                        skins[i].pictures[j] = picture;
                    }
                }
            }
        }
    }
}
