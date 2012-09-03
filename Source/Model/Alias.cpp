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
            while(!m_pictures.empty()) delete m_pictures.back(), m_pictures.pop_back();
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
                    m_bounds += m_frames[0]->bounds();
            } else {
                m_bounds.min = m_bounds.max = Vec3f::Null;
            }
        }
        
        AliasFrameGroup::~AliasFrameGroup() {
            while (!m_frames.empty()) delete m_frames.back(), m_frames.pop_back();
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
            stream->read(reinterpret_cast<char *>(&packedFrameVertices[0]), vertices.size() * 4 * sizeof(unsigned char));
            
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
                bounds += frameVertices[i];
            }
            
            center /= static_cast<float>(vertices.size());
            
            AliasFrameTriangleList frameTriangles;
            for (unsigned int i = 0; i < triangles.size(); i++) {
                AliasFrameTriangle* frameTriangle = new AliasFrameTriangle();
                for (unsigned int j = 0; j < 3; j++) {
                    int index = triangles[i].vertices[j];
                    
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
            while(!m_triangles.empty()) delete m_triangles.back(), m_triangles.pop_back();
        }
        
        Alias::Alias(const String& name, IO::PakStream stream) :
        m_name(name) {
            Vec3f scale, origin;
            int32_t skinCount, skinWidth, skinHeight, skinSize;
            int32_t vertexCount, triangleCount, frameCount;
            
            stream->seekg(AliasLayout::HeaderScale, std::ios::beg);
            stream->read(reinterpret_cast<char *>(&scale), sizeof(Vec3f));
            stream->read(reinterpret_cast<char *>(&origin), sizeof(Vec3f));
            
            stream->seekg(AliasLayout::HeaderNumSkins, std::ios::beg);
            stream->read(reinterpret_cast<char *>(&skinCount), sizeof(int32_t));
            stream->read(reinterpret_cast<char *>(&skinWidth), sizeof(int32_t));
            stream->read(reinterpret_cast<char *>(&skinHeight), sizeof(int32_t));
            skinSize = skinWidth * skinHeight;
            
            stream->read(reinterpret_cast<char *>(&vertexCount), sizeof(int32_t));
            stream->read(reinterpret_cast<char *>(&triangleCount), sizeof(int32_t));
            stream->read(reinterpret_cast<char *>(&frameCount), sizeof(int32_t));
            
            stream->seekg(AliasLayout::Skins, std::ios::beg);
            for (unsigned int i = 0; i < skinCount; i++) {
                int32_t skinGroup;
                
                stream->read(reinterpret_cast<char *>(&skinGroup), sizeof(int32_t));
                if (skinGroup == 0) {
                    unsigned char* skinPicture = new unsigned char[skinSize];
                    stream->read(reinterpret_cast<char *>(skinPicture), skinSize);
                    
                    AliasSkin* skin = new AliasSkin(skinPicture, skinWidth, skinHeight);
                    m_skins.push_back(skin);
                } else {
                    int32_t numPics;
                    stream->read(reinterpret_cast<char *>(&numPics), sizeof(int32_t));
                    
                    AliasTimeList times(numPics);
                    AliasPictureList skinPictures(numPics);
                    
                    std::streampos base = stream->tellg();
                    for (unsigned int j = 0; j < numPics; j++) {
                        stream->seekg(j * sizeof(float) + base, std::ios::beg);
                        stream->read(reinterpret_cast<char *>(&times[i]), sizeof(float));
                        
                        unsigned char* skinPicture = new unsigned char[skinSize];
                        stream->seekg(numPics * sizeof(float) + j * skinSize + base, std::ios::beg);
                        stream->read(reinterpret_cast<char *>(&skinPicture), skinSize);
                        
                        skinPictures[i] = skinPicture;
                    }
                    
                    AliasSkin* skin = new AliasSkin(skinPictures, times, numPics, skinWidth, skinHeight);
                    m_skins.push_back(skin);
                }
            }
            
            // now stream is at the first skin vertex
            std::vector<AliasSkinVertex> vertices(vertexCount);
            for (int i = 0; i < vertexCount; i++) {
                stream->read(reinterpret_cast<char *>(&vertices[i].onseam), sizeof(int32_t));
                stream->read(reinterpret_cast<char *>(&vertices[i].s), sizeof(int32_t));
                stream->read(reinterpret_cast<char *>(&vertices[i].t), sizeof(int32_t));
            }
            
            // now stream is at the first skin triangle
            std::vector<AliasSkinTriangle> triangles(triangleCount);
            for (int i = 0; i < triangleCount; i++) {
                stream->read(reinterpret_cast<char *>(&triangles[i].front), sizeof(int32_t));
                for (unsigned int j = 0; j < 3; j++)
                    stream->read(reinterpret_cast<char *>(&triangles[i].vertices[j]), sizeof(int32_t));
            }
            
            // now stream is at the first frame
            for (int i = 0; i < frameCount; i++) {
                int32_t type;
                stream->read(reinterpret_cast<char *>(&type), sizeof(int32_t));
                if (type == 0) { // single frame
                    m_frames.push_back(readFrame(stream, origin, scale, skinWidth, skinHeight, vertices, triangles));
                } else { // frame group
                    int32_t groupFrameCount;
                    std::streampos base = stream->tellg();
                    stream->read(reinterpret_cast<char *>(&groupFrameCount), sizeof(int32_t));
                    
                    std::streampos timePos = AliasLayout::MultiFrameTimes + base;
                    std::streampos framePos = AliasLayout::MultiFrameTimes + groupFrameCount * sizeof(float) + base;
                    
                    AliasTimeList groupFrameTimes(groupFrameCount);
                    AliasSingleFrameList groupFrames(groupFrameCount);
                    for (int j = 0; j < groupFrameCount; j++) {
                        stream->seekg(timePos, std::ios::beg);
                        stream->read(reinterpret_cast<char *>(&groupFrameTimes[i]), sizeof(float));
                        timePos = j * sizeof(float) + timePos;
                        
                        stream->seekg(framePos, std::ios::beg);
                        groupFrames[j] = readFrame(stream, origin, scale, skinWidth, skinHeight, vertices, triangles);
                        framePos = AliasLayout::SimpleFrameLength + (vertexCount + 2) * AliasLayout::FrameVertexSize + framePos;
                    }
                    
                    m_frames.push_back(new AliasFrameGroup(groupFrameTimes, groupFrames));
                }
            }
        }
        
        Alias::~Alias() {
            while(!m_frames.empty()) delete m_frames.back(), m_frames.pop_back();
            while(!m_skins.empty()) delete m_skins.back(), m_skins.pop_back();
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