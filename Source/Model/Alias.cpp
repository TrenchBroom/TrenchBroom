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

        AliasSingleFrame::AliasSingleFrame(const String& name, const AliasFrameTriangleList& triangles, const Vec3f& center, const BBox& bounds) :
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

            if (m_frames.size() > 0) {
                m_bounds = m_frames[0]->bounds();
                for (unsigned int i = 1; i < m_frames.size(); i++)
                    m_bounds.mergeWith(m_frames[0]->bounds());
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
            vertex.x = size.x * packedVertex.x + origin.x;
            vertex.y = size.y * packedVertex.y + origin.y;
            vertex.z = size.z * packedVertex.z + origin.z;
            return vertex;
        }

        AliasSingleFrame* Alias::readFrame(IO::PakStream& stream, const Vec3f& origin, const Vec3f& scale, unsigned int skinWidth, unsigned int skinHeight, const AliasSkinVertexList& vertices, const AliasSkinTriangleList& triangles) {
            char name[AliasLayout::SimpleFrameLength];
            stream->seekg(AliasLayout::SimpleFrameName, std::ios::cur);
            stream->read(name, AliasLayout::SimpleFrameLength);

            std::vector<AliasPackedFrameVertex> packedFrameVertices(vertices.size());
            stream->read(reinterpret_cast<char *>(&packedFrameVertices[0]), static_cast<std::streamsize>(vertices.size() * 4 * sizeof(unsigned char)));

            Vec3f::List frameVertices(vertices.size());
            Vec3f center;
            BBox bounds;

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
                    texCoords.x = static_cast<float>(vertices[index].s) / static_cast<float>(skinWidth);
                    texCoords.y = static_cast<float>(vertices[index].t) / static_cast<float>(skinHeight);

                    if (vertices[index].onseam && !triangles[i].front)
                        texCoords.x += 0.5f;

                    (*frameTriangle)[j].setPosition(frameVertices[index]);
                    (*frameTriangle)[j].setNormal(AliasNormals[packedFrameVertices[index].i]);
                    (*frameTriangle)[j].setTexCoords(texCoords);
                }

                frameTriangles.push_back(frameTriangle);
            }

            return new AliasSingleFrame(name, frameTriangles, center, bounds);
        }

        AliasSingleFrame::~AliasSingleFrame() {
            Utility::deleteAll(m_triangles);
        }

        Alias::Alias(const String& name, IO::PakStream stream) :
        m_name(name) {
            stream->seekg(AliasLayout::HeaderScale, std::ios::beg);
            Vec3f scale = IO::readVec3f(stream);
            Vec3f origin = IO::readVec3f(stream);
            
            stream->seekg(AliasLayout::HeaderNumSkins, std::ios::beg);
            unsigned int skinCount = IO::readUnsignedInt<int32_t>(stream);
            unsigned int skinWidth = IO::readUnsignedInt<int32_t>(stream);
            unsigned int skinHeight = IO::readUnsignedInt<int32_t>(stream);
            unsigned int skinSize = skinWidth * skinHeight;

            unsigned int vertexCount = IO::readUnsignedInt<int32_t>(stream);
            unsigned int triangleCount = IO::readUnsignedInt<int32_t>(stream);
            unsigned int frameCount = IO::readUnsignedInt<int32_t>(stream);
            frameCount = frameCount; // dummy to suppress "unused" warning
            
            stream->seekg(AliasLayout::Skins, std::ios::beg);
            for (unsigned int i = 0; i < skinCount; i++) {
                unsigned int skinGroup = IO::readUnsignedInt<int32_t>(stream);
                if (skinGroup == 0) {
                    unsigned char* skinPicture = new unsigned char[skinSize];
                    stream->read(reinterpret_cast<char *>(skinPicture), static_cast<std::streamsize>(skinSize));

                    AliasSkin* skin = new AliasSkin(skinPicture, skinWidth, skinHeight);
                    m_skins.push_back(skin);
                } else {
                    unsigned int numPics = IO::readUnsignedInt<int32_t>(stream);
                    AliasTimeList times(numPics);
                    AliasPictureList skinPictures(numPics);

                    std::streampos base = stream->tellg();
                    for (size_t j = 0; j < static_cast<size_t>(numPics); j++) {
                        std::streamoff timeOffset = static_cast<std::streamoff>(j * 4);
                        timeOffset += base;
                        stream->seekg(timeOffset, std::ios::beg);
                        times[i] = IO::readFloat(stream);

                        unsigned char* skinPicture = new unsigned char[skinSize];
                        std::streamoff picOffset = static_cast<std::streamoff>(numPics * 4 + j * skinSize);
                        picOffset += base;
                        stream->seekg(picOffset, std::ios::beg);
                        stream->read(reinterpret_cast<char *>(skinPicture), static_cast<std::streamsize>(skinSize));

                        skinPictures[i] = skinPicture;
                    }

                    AliasSkin* skin = new AliasSkin(skinPictures, times, numPics, skinWidth, skinHeight);
                    m_skins.push_back(skin);
                }
            }

            // now stream is at the first skin vertex
            std::vector<AliasSkinVertex> vertices(vertexCount);
            for (unsigned int i = 0; i < vertexCount; i++) {
                vertices[i].onseam = IO::readBool<int32_t>(stream);
                vertices[i].s = IO::readInt<int32_t>(stream);
                vertices[i].t = IO::readInt<int32_t>(stream);
            }

            // now stream is at the first skin triangle
            std::vector<AliasSkinTriangle> triangles(triangleCount);
            for (unsigned int i = 0; i < triangleCount; i++) {
                triangles[i].front = IO::readBool<int32_t>(stream);
                for (unsigned int j = 0; j < 3; j++)
                    triangles[i].vertices[j] = IO::readUnsignedInt<int32_t>(stream);
            }

            // now stream is at the first frame
            for (unsigned int i = 0; i < frameCount; i++) {
                int type = IO::readInt<int32_t>(stream);
                if (type == 0) { // single frame
                    m_frames.push_back(readFrame(stream, origin, scale, skinWidth, skinHeight, vertices, triangles));
                } else { // frame group
                    std::streampos base = stream->tellg();
                    unsigned int groupFrameCount = IO::readUnsignedInt<int32_t>(stream);

                    std::streampos timePos = AliasLayout::MultiFrameTimes + base;
                    std::streampos framePos = AliasLayout::MultiFrameTimes + groupFrameCount * sizeof(float);
                    framePos += base;

                    AliasTimeList groupFrameTimes(groupFrameCount);
                    AliasSingleFrameList groupFrames(groupFrameCount);
                    for (unsigned int j = 0; j < groupFrameCount; j++) {
                        stream->seekg(timePos, std::ios::beg);
                        groupFrameTimes[i] = IO::readFloat(stream);
                        timePos += j * sizeof(float);

                        stream->seekg(framePos, std::ios::beg);
                        groupFrames[j] = readFrame(stream, origin, scale, skinWidth, skinHeight, vertices, triangles);
                        framePos += AliasLayout::SimpleFrameLength + (vertexCount + 2) * AliasLayout::FrameVertexSize;
                    }

                    m_frames.push_back(new AliasFrameGroup(groupFrameTimes, groupFrames));
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

            IO::PakManager& pakManager = *IO::PakManager::sharedManager;
            IO::PakStream stream = pakManager.entryStream(name, paths);
            if (stream.get() != NULL) {
                Alias* alias = new Alias(name, stream);
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
