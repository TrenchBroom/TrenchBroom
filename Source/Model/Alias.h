/*
 Copyright (C) 2010-2012 Kristian Duske
 
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

#ifndef TrenchBroom_Alias_h
#define TrenchBroom_Alias_h

#include "IO/Pak.h"
#include "Utility/Console.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <istream>
#include <map>
#include <vector>

#ifdef _MSC_VER
#include <cstdint>
#endif

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        namespace AliasLayout {
            static const unsigned int HeaderScale       = 0x8;
            static const unsigned int HeaderNumSkins    = 0x30;
            static const unsigned int Skins             = 0x54;
            static const unsigned int SimpleFrameName   = 0x8;
            static const unsigned int SimpleFrameLength = 0x10;
            static const unsigned int MultiFrameTimes   = 0xC;
            static const unsigned int FrameVertexSize   = 0x4;
        }
        
        class AliasSkinVertex {
        public:
            bool onseam;
            int s, t;
        };
        
        typedef std::vector<AliasSkinVertex> AliasSkinVertexList;
        
        class AliasSkinTriangle {
        public:
            bool front;
            unsigned int vertices[3];
        };
        
        typedef std::vector<AliasSkinTriangle> AliasSkinTriangleList;
        
        class AliasPackedFrameVertex {
        public:
            unsigned char x, y, z, i;
        };
        
        // publicly visible classes below
        
        class AliasFrameVertex {
        private:
            Vec3f m_position;
            Vec3f m_normal;
            Vec2f m_texCoords;
        public:
            inline const Vec3f& position() const {
                return m_position;
            }
            
            inline void setPosition(const Vec3f& position) {
                m_position = position;
            }
            
            inline const Vec3f& normal() const {
                return m_normal;
            }
            
            inline void setNormal(const Vec3f& normal) {
                m_normal = normal;
            }
            
            inline const Vec2f& texCoords() const {
                return m_texCoords;
            }
            
            inline void setTexCoords(const Vec2f& texCoords) {
                m_texCoords = texCoords;
            }
        };
        
        class AliasFrameTriangle {
        private:
            AliasFrameVertex m_vertices[3];
        public:
            inline AliasFrameVertex& operator[] (const size_t index) {
                assert(index >= 0 && index < 3);
                return m_vertices[index];
            }
            
            inline const AliasFrameVertex& operator[] (const size_t index) const {
                assert(index >= 0 && index < 3);
                return m_vertices[index];
            }
        };
        
        typedef std::vector<AliasFrameTriangle*> AliasFrameTriangleList;
        typedef std::vector<float> AliasTimeList;
        typedef std::vector<const unsigned char*> AliasPictureList;
        
        class AliasSkin {
        public:
        private:
            AliasPictureList m_pictures;
            AliasTimeList m_times;
            unsigned int m_count;
            unsigned int m_width;
            unsigned int m_height;
        public:
            AliasSkin(const unsigned char* picture, unsigned int width, unsigned int height);
            AliasSkin(const AliasPictureList& pictures, const AliasTimeList& times, unsigned int count, unsigned int width, unsigned int height);
            ~AliasSkin();
            
            inline unsigned int width() const {
                return m_width;
            }
        
            inline unsigned int height() const {
                return m_height;
            }
        
            inline const AliasPictureList& pictures() const {
                return m_pictures;
            }
        };
        
        class AliasSingleFrame;
        typedef std::vector<AliasSingleFrame*> AliasSingleFrameList;
        typedef std::vector<AliasSkin*> AliasSkinList;
        
        class AliasFrame {
        public:
            virtual ~AliasFrame() {};
            virtual AliasSingleFrame* firstFrame() = 0;
        };
        
        typedef std::vector<AliasFrame*> AliasFrameList;
        
        class AliasSingleFrame : public AliasFrame {
        private:
            String m_name;
            AliasFrameTriangleList m_triangles;
            Vec3f m_center;
            BBox m_bounds;
        public:
            AliasSingleFrame(const String& name, const AliasFrameTriangleList& triangles, const Vec3f& center, const BBox& bounds);
            ~AliasSingleFrame();
            
            inline const String& name() const {
                return m_name;
            }
            
            inline const AliasFrameTriangleList& triangles() const {
                return m_triangles;
            }
            
            inline const Vec3f& center() const {
                return m_center;
            }
            
            inline const BBox& bounds() const {
                return m_bounds;
            }
            
            AliasSingleFrame* firstFrame();
        };
        
        class AliasFrameGroup : public AliasFrame {
        private:
            AliasTimeList m_times;
            AliasSingleFrameList m_frames;
            BBox m_bounds;
        public:
            AliasFrameGroup(const AliasTimeList& times, const AliasSingleFrameList& frames);
            ~AliasFrameGroup();
            AliasSingleFrame* firstFrame();
        };
        
        class Alias {
        private:
            String m_name;
            AliasFrameList m_frames;
            AliasSkinList m_skins;
            
            Vec3f unpackFrameVertex(const AliasPackedFrameVertex& packedVertex, const Vec3f& origin, const Vec3f& size);
            AliasSingleFrame* readFrame(char*& cursor, const Vec3f& origin, const Vec3f& scale, unsigned int skinWidth, unsigned int skinHeight, const AliasSkinVertexList& vertices, const AliasSkinTriangleList& triangles);
        public:
            Alias(const String& name, char* begin, char* end);
            ~Alias();
            
            inline const String& name() const {
                return m_name;
            }
            
            inline const AliasFrameList& frames() const {
                return m_frames;
            }
            
            inline AliasSingleFrame& frame(size_t index) const {
                assert(index < m_frames.size());
                return *m_frames[index]->firstFrame();
            }
            
            inline AliasSingleFrame& firstFrame() const {
                return *m_frames[0]->firstFrame();
            }
            
            inline const AliasSkinList& skins() const {
                return m_skins;
            }
        };
        
        class AliasManager {
        private:
            typedef std::map<String, Alias*> AliasMap;
            
            AliasMap m_aliases;
        public:
            static AliasManager* sharedManager;
            AliasManager();
            ~AliasManager();
            Alias const * const alias(const String& name, const StringList& paths, Utility::Console& console);
        };
    }
}
#endif
