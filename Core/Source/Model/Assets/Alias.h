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

#include <istream>
#include <string>
#include <vector>
#include <map>
#include "Utilities/VecMath.h"
#include "IO/Pak.h"


#ifdef _MSC_VER
#include <cstdint>
#endif

using namespace std;

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            
            class AliasSkinVertex {
            public:
                int32_t onseam, s, t;
            };
            
            class AliasSkinTriangle {
            public:
                int32_t front;
                int32_t vertices[3];
            };
            
            class AliasFrameVertex {
            public:
                Vec2f texCoords;
                Vec3f position, normal;
            };
            
            class AliasPackedFrameVertex {
            public:
                unsigned char x, y, z, i;
            };
            
            class AliasFrameTriangle {
            public:
                AliasFrameVertex vertices[3];
            };
            
            class AliasSkin {
            public:
                unsigned int width, height, count;
                vector<float> times;
                vector<const unsigned char*> pictures;
                AliasSkin(const unsigned char* picture, unsigned int width, unsigned int height);
                AliasSkin(const vector<const unsigned char*>& pictures, const vector<float>& times, unsigned int count, unsigned int width, unsigned int height);
                ~AliasSkin();
            };
            
            class AliasSingleFrame;
            
            class AliasFrame {
            public:
                virtual ~AliasFrame() {};
                virtual AliasSingleFrame* firstFrame() = 0;
            };
            
            class AliasSingleFrame : public AliasFrame {
            public:
                string name;
                vector<AliasFrameTriangle*> triangles;
                Vec3f center;
                BBox bounds;
                AliasSingleFrame(const string& name, const vector<AliasFrameTriangle*>& triangles, const Vec3f& center, const BBox& bounds);
                ~AliasSingleFrame();
                AliasSingleFrame* firstFrame();
            };
            
            class AliasFrameGroup : public AliasFrame {
            public:
                vector<float> times;
                vector<AliasSingleFrame*> frames;
                BBox bounds;
                AliasFrameGroup(const vector<float>& times, const vector<AliasSingleFrame*>& frames);
                ~AliasFrameGroup();
                AliasSingleFrame* firstFrame();
            };
            
            class Alias {
            private:
                Vec3f unpackFrameVertex(const AliasPackedFrameVertex& packedVertex, const Vec3f& origin, const Vec3f& size);
                AliasSingleFrame* readFrame(IO::PakStream& stream, const Vec3f& origin, const Vec3f& scale, unsigned int skinWidth, unsigned int skinHeight, const vector<AliasSkinVertex>& vertices, const vector<AliasSkinTriangle>& triangles);
            public:
                string name;
                vector<AliasFrame*> frames;
                vector<AliasSkin*> skins;
                Alias(const string& name, IO::PakStream stream);
                ~Alias();
                AliasSingleFrame& firstFrame();
            };
            
            class AliasManager {
            private:
                map<string, Alias*> aliases;
                AliasManager(const AliasManager&);
            public:
                static AliasManager* sharedManager;
                AliasManager();
                ~AliasManager();
                Alias* aliasForName(const string& name, const vector<string>& paths);
            };
        }
    }
}
#endif
