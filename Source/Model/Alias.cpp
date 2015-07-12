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

#include "Alias.h"

#include "Model/AliasNormals.h"
#include "IO/IOUtils.h"
#include "Utility/List.h"

#include <cassert>
#include <cmath>
#include <numeric>

namespace TrenchBroom {
    namespace Model {
        AliasSkin::AliasSkin(const unsigned char* picture, unsigned int width, unsigned int height) :
        m_width(width),
        m_height(height) {
            m_count = 1;
            m_pictures.push_back(picture);
        }

        AliasSkin::AliasSkin(const AliasPictureList& pictures, const AliasTimeList& times, unsigned int count, unsigned  int width, unsigned int height) :
        m_pictures(pictures),
        m_times(times),
        m_count(count),
        m_width(width),
        m_height(height) {
            assert(m_pictures.size() == m_times.size());
        }

        AliasSkin::~AliasSkin() {
            Utility::deleteAll(m_pictures);
        }

        AliasSingleFrame::AliasSingleFrame(const String& name, const AliasFrameTriangleList& triangles, const Vec3f& center, const BBoxf& bounds) :
        m_name(name),
        m_triangles(triangles),
        m_center(center),
        m_bounds(bounds) {}

        AliasSingleFrame* AliasSingleFrame::firstFrame() {
            return this;
        }

        AliasFrameGroup::AliasFrameGroup(const AliasTimeList& times, const AliasSingleFrameList& frames) :
        m_times(times),
        m_frames(frames) {
            assert(m_times.size() == m_frames.size());

            if (!m_frames.empty()) {
                m_bounds = m_frames[0]->bounds();
                for (size_t i = 1; i < m_frames.size(); i++)
                    m_bounds.mergeWith(m_frames[i]->bounds());
            } else {
                m_bounds.min = m_bounds.max = Vec3f::Null;
            }
        }

        AliasFrameGroup::~AliasFrameGroup() {
            Utility::deleteAll(m_frames);
        }

        AliasSingleFrame* AliasFrameGroup::firstFrame() {
            return m_frames[0];
        }

        Vec3f Alias::unpackFrameVertex(const AliasPackedFrameVertex& packedVertex, const Vec3f& origin, const Vec3f& size) {
            Vec3f vertex;
            for (size_t i = 0; i < 3; i++)
                vertex[i] = size[i] * packedVertex[i] + origin[i];
            return vertex;
        }

        AliasSingleFrame* Alias::readFrame(char*& cursor, const Vec3f& origin, const Vec3f& scale, unsigned int skinWidth, unsigned int skinHeight, const AliasSkinVertexList& vertices, const AliasSkinTriangleList& triangles) {
            using namespace IO;
            
            char name[AliasLayout::SimpleFrameLength];
            cursor += AliasLayout::SimpleFrameName;
            readBytes(cursor, name, AliasLayout::SimpleFrameLength);

            std::vector<AliasPackedFrameVertex> packedFrameVertices(vertices.size());
            readBytes(cursor, reinterpret_cast<char*>(&packedFrameVertices[0]), vertices.size() * 4);

            Vec3f::List frameVertices(vertices.size());
            Vec3f center;
            BBoxf bounds;

            frameVertices[0] = unpackFrameVertex(packedFrameVertices[0], origin, scale);
            center = frameVertices[0];
            bounds.min = frameVertices[0];
            bounds.max = frameVertices[0];

            for (unsigned int i = 1; i < vertices.size(); i++) {
                frameVertices[i] = unpackFrameVertex(packedFrameVertices[i], origin, scale);
                center += frameVertices[i];
                bounds.mergeWith(frameVertices[i]);
            }

            center /= static_cast<float>(vertices.size());

            AliasFrameTriangleList frameTriangles;
            frameTriangles.reserve(triangles.size());
            for (unsigned int i = 0; i < triangles.size(); i++) {
                AliasFrameTriangle* frameTriangle = new AliasFrameTriangle();
                for (unsigned int j = 0; j < 3; j++) {
                    size_t index = triangles[i].vertices[j];

                    Vec2f texCoords;
                    texCoords[0] = static_cast<float>(vertices[index].s) / static_cast<float>(skinWidth);
                    texCoords[1] = static_cast<float>(vertices[index].t) / static_cast<float>(skinHeight);

                    if (vertices[index].onseam && !triangles[i].front)
                        texCoords[0] += 0.5f;

                    (*frameTriangle)[j].setPosition(frameVertices[index]);
                    (*frameTriangle)[j].setNormal(AliasNormals[packedFrameVertices[index][3]]);
                    (*frameTriangle)[j].setTexCoords(texCoords);
                }

                frameTriangles.push_back(frameTriangle);
            }

            return new AliasSingleFrame(name, frameTriangles, center, bounds);
        }

        AliasSingleFrame::~AliasSingleFrame() {
            Utility::deleteAll(m_triangles);
        }

        Alias::Alias(const String& name, char* begin, char* end) :
        m_name(name) {
            using namespace IO;
            
            char* cursor = begin + AliasLayout::HeaderScale;
            Vec3f scale = readVec3f(cursor);
            Vec3f origin = readVec3f(cursor);

            cursor = begin + AliasLayout::HeaderNumSkins;
            unsigned int skinCount = readUnsignedInt<int32_t>(cursor);
            unsigned int skinWidth = readUnsignedInt<int32_t>(cursor);
            unsigned int skinHeight = readUnsignedInt<int32_t>(cursor);
            unsigned int skinSize = skinWidth * skinHeight;

            unsigned int vertexCount = readUnsignedInt<int32_t>(cursor);
            unsigned int triangleCount = readUnsignedInt<int32_t>(cursor);
            unsigned int frameCount = readUnsignedInt<int32_t>(cursor);
            frameCount = frameCount; // dummy to suppress "unused" warning
            
            cursor = begin + AliasLayout::Skins;
            for (unsigned int i = 0; i < skinCount; i++) {
                unsigned int skinGroup = readUnsignedInt<int32_t>(cursor);
                if (skinGroup == 0) {
                    unsigned char* skinPicture = new unsigned char[skinSize];
                    readBytes(cursor, skinPicture, skinSize);

                    AliasSkin* skin = new AliasSkin(skinPicture, skinWidth, skinHeight);
                    m_skins.push_back(skin);
                } else {
                    unsigned int numPics = readUnsignedInt<int32_t>(cursor);
                    AliasTimeList times(numPics);
                    AliasPictureList skinPictures(numPics);

                    char* base = cursor;
                    for (size_t j = 0; j < static_cast<size_t>(numPics); j++) {
                        cursor = base + j * sizeof(float);
                        times[j] = readFloat<float>(cursor);

                        unsigned char* skinPicture = new unsigned char[skinSize];
                        cursor = base + numPics * 4 + j * skinSize;
                        readBytes(cursor, skinPicture, skinSize);

                        skinPictures[j] = skinPicture;
                    }

                    AliasSkin* skin = new AliasSkin(skinPictures, times, numPics, skinWidth, skinHeight);
                    m_skins.push_back(skin);
                }
            }

            // now cursor is at the first skin vertex
            std::vector<AliasSkinVertex> vertices(vertexCount);
            for (unsigned int i = 0; i < vertexCount; i++) {
                vertices[i].onseam = readBool<int32_t>(cursor);
                vertices[i].s = readInt<int32_t>(cursor);
                vertices[i].t = readInt<int32_t>(cursor);
            }

            // now cursor is at the first skin triangle
            std::vector<AliasSkinTriangle> triangles(triangleCount);
            for (unsigned int i = 0; i < triangleCount; i++) {
                triangles[i].front = readBool<int32_t>(cursor);
                for (unsigned int j = 0; j < 3; j++)
                    triangles[i].vertices[j] = readUnsignedInt<int32_t>(cursor);
            }

            // now cursor is at the first frame
            for (unsigned int i = 0; i < frameCount; i++) {
                int type = readInt<int32_t>(cursor);
                if (type == 0) { // single frame
                    m_frames.push_back(readFrame(cursor, origin, scale, skinWidth, skinHeight, vertices, triangles));
                } else { // frame group
                    char* base = cursor;
                    unsigned int groupFrameCount = readUnsignedInt<int32_t>(cursor);

                    char* timeCursor = base + AliasLayout::MultiFrameTimes;
                    char* frameCursor = base + AliasLayout::MultiFrameTimes + groupFrameCount * sizeof(float);

                    AliasTimeList groupFrameTimes(groupFrameCount);
                    AliasSingleFrameList groupFrames(groupFrameCount);
                    for (unsigned int j = 0; j < groupFrameCount; j++) {
                        groupFrameTimes[j] = readFloat<float>(timeCursor);
                        groupFrames[j] = readFrame(frameCursor, origin, scale, skinWidth, skinHeight, vertices, triangles);
                    }

                    m_frames.push_back(new AliasFrameGroup(groupFrameTimes, groupFrames));
                    cursor = frameCursor;
                }
            }
        }

        Alias::~Alias() {
            Utility::deleteAll(m_frames);
            Utility::deleteAll(m_skins);
        }

        AliasManager* AliasManager::sharedManager = NULL;

        Alias const * const AliasManager::alias(const String& name, const StringList& paths, Utility::Console& console) {
            String pathList = Utility::join(paths, ",");
            String key = pathList + ":" + name;

            AliasMap::iterator it = m_aliases.find(key);
            if (it != m_aliases.end())
                return it->second;

            console.info("Loading '%s' (searching %s)", name.c_str(), pathList.c_str());

            IO::MappedFile::Ptr file = IO::findGameFile(name, paths);
            if (file.get() != NULL) {
                Alias* alias = new Alias(name, file->begin(), file->end());
                m_aliases[key] = alias;
                return alias;
            }

            console.warn("Unable to find MDL '%s'", name.c_str());
            return NULL;
        }

        AliasManager::AliasManager() {}

        AliasManager::~AliasManager() {
            AliasMap::iterator it, end;
            for (it = m_aliases.begin(), end = m_aliases.end(); it != end; ++it)
                delete it->second;
            m_aliases.clear();
        }
    }
}
