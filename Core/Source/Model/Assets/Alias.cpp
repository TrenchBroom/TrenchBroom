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
#include <assert.h>
#include <numeric>
#include <cmath>
#include "Model/Assets/AliasNormals.h"
#include "Utilities/Console.h"

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            static const int MDL_HEADER_SCALE               = 0x8;
            static const int MDL_HEADER_NUMSKINS            = 0x30;
            static const int MDL_SKINS                      = 0x54;
            static const int MDL_SIMPLE_FRAME_NAME          = 0x8;
            static const int MDL_SIMPLE_FRAME_NAME_SIZE     = 0x10;
            static const int MDL_MULTI_FRAME_TIMES          = 0xC;
            static const int MDL_FRAME_VERTEX_SIZE          = 0x4;
            
            AliasSkin::AliasSkin(const unsigned char* picture, unsigned int width, unsigned int height) : width(width), height(height) {
                count = 1;
                pictures.push_back(picture);
            }
            
            AliasSkin::AliasSkin(const std::vector<const unsigned char*>& pictures, const std::vector<float>& times, unsigned int count, unsigned  int width, unsigned int height) : pictures(pictures), times(times), count(count), width(width), height(height) {
                assert(pictures.size() == times.size());
            }
            
            AliasSkin::~AliasSkin() {
                while(!pictures.empty()) delete pictures.back(), pictures.pop_back();
            }
            
            AliasSingleFrame::AliasSingleFrame(const std::string& name, const std::vector<AliasFrameTriangle*>& triangles, const Vec3f& center, const BBox& bounds) : name(name), triangles(triangles), center(center), bounds(bounds) {}
            
            AliasSingleFrame* AliasSingleFrame::firstFrame() {
                return this;
            }
            
            AliasFrameGroup::AliasFrameGroup(const std::vector<float>& times, const std::vector<AliasSingleFrame*>& frames)
            : times(times), frames(frames) {
                assert(times.size() == frames.size());
                
                if (frames.size() > 0) {
                    bounds = frames[0]->bounds;
                    for (unsigned int i = 1; i < frames.size(); i++)
                        bounds += frames[0]->bounds;
                } else {
                    bounds.min = bounds.max = Vec3f::Null;
                }
            }
            
            AliasFrameGroup::~AliasFrameGroup() {
                while (!frames.empty()) delete frames.back(), frames.pop_back();
            }

            AliasSingleFrame* AliasFrameGroup::firstFrame() {
                return frames[0];
            }
            
            Vec3f Alias::unpackFrameVertex(const AliasPackedFrameVertex& packedVertex, const Vec3f& origin, const Vec3f& size) {
                Vec3f vertex;
                vertex.x = size.x * packedVertex.x + origin.x;
                vertex.y = size.y * packedVertex.y + origin.y;
                vertex.z = size.z * packedVertex.z + origin.z;
                return vertex;
            }
            
            AliasSingleFrame* Alias::readFrame(IO::PakStream& stream, const Vec3f& origin, const Vec3f& scale, unsigned int skinWidth, unsigned int skinHeight, const std::vector<AliasSkinVertex>& vertices, const std::vector<AliasSkinTriangle>& triangles) {
                char name[MDL_SIMPLE_FRAME_NAME_SIZE];
                stream->seekg(MDL_SIMPLE_FRAME_NAME, std::ios::cur);
                stream->read(name, MDL_SIMPLE_FRAME_NAME_SIZE);
                
                std::vector<AliasPackedFrameVertex> packedFrameVertices(vertices.size());
                for (unsigned int i = 0; i < vertices.size(); i++) {
                    AliasPackedFrameVertex packedVertex;
                    stream->read((char *)&packedVertex, 4 * sizeof(unsigned char));
                    packedFrameVertices[i] = packedVertex;
                }
                
                std::vector<Vec3f> frameVertices(vertices.size());
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
                
                std::vector<AliasFrameTriangle*> frameTriangles(triangles.size());
                for (unsigned int i = 0; i < triangles.size(); i++) {
                    AliasFrameTriangle* frameTriangle = new AliasFrameTriangle();
                    for (unsigned int j = 0; j < 3; j++) {
                        int index = triangles[i].vertices[j];
                        frameTriangle->vertices[j].position = frameVertices[index];
                        frameTriangle->vertices[j].normal = AliasNormals[packedFrameVertices[index].i];
                        frameTriangle->vertices[j].texCoords.x = static_cast<float>(vertices[index].s) / static_cast<float>(skinWidth);
                        frameTriangle->vertices[j].texCoords.y = static_cast<float>(vertices[index].t) / static_cast<float>(skinHeight);
                        if (vertices[index].onseam && !triangles[i].front)
                            frameTriangle->vertices[j].texCoords.x += 0.5f;
                    }
                    frameTriangles[i] = frameTriangle;
                }
                
                return new AliasSingleFrame(name, frameTriangles, center, bounds);
            }
            
            AliasSingleFrame::~AliasSingleFrame() {
                while(!triangles.empty()) delete triangles.back(), triangles.pop_back();
            }
            
            Alias::Alias(const std::string& name, IO::PakStream stream) : name(name) {
                Vec3f scale, origin;
                int32_t skinCount, skinWidth, skinHeight, skinSize;
                int32_t vertexCount, triangleCount, frameCount;
                
                stream->seekg(MDL_HEADER_SCALE, std::ios::beg);
                stream->read((char *)&scale, sizeof(Vec3f));
                stream->read((char *)&origin, sizeof(Vec3f));
                
                stream->seekg(MDL_HEADER_NUMSKINS, std::ios::beg);
                stream->read((char *)&skinCount, sizeof(int32_t));
                stream->read((char *)&skinWidth, sizeof(int32_t));
                stream->read((char *)&skinHeight, sizeof(int32_t));
                skinSize = skinWidth * skinHeight;
                
                stream->read((char *)&vertexCount, sizeof(int32_t));
                stream->read((char *)&triangleCount, sizeof(int32_t));
                stream->read((char *)&frameCount, sizeof(int32_t));
                
                stream->seekg(MDL_SKINS, std::ios::beg);
                for (int i = 0; i < skinCount; i++) {
                    int32_t skinGroup;
                    
                    stream->read((char *)&skinGroup, sizeof(int32_t));
                    if (skinGroup == 0) {
                        unsigned char* skinPicture = new unsigned char[skinSize];
                        stream->read((char *)skinPicture, skinSize);
                        AliasSkin* skin = new AliasSkin(skinPicture, skinWidth, skinHeight);
                        skins.push_back(skin);
                    } else {
                        int32_t numPics;
                        stream->read((char *)&numPics, sizeof(int32_t));
                        std::vector<float> times(numPics);
                        std::vector<const unsigned char *> skinPictures(numPics);
                        
                        std::streampos base = stream->tellg();
                        for (int j = 0; j < numPics; j++) {
                            stream->seekg(j * sizeof(float) + base, std::ios::beg);
                            stream->read((char *)&times[i], sizeof(float));
                            
                            unsigned char* skinPicture = new unsigned char[skinSize];
                            stream->seekg(numPics * sizeof(float) + j * skinSize + base, std::ios::beg);
                            stream->read((char *)&skinPicture, skinSize);
                            
                            skinPictures[i] = skinPicture;
                        }
                        
                        AliasSkin* skin = new AliasSkin(skinPictures, times, numPics, skinWidth, skinHeight);
                        skins.push_back(skin);
                    }
                }
                
                // now stream is at the first skin vertex
                std::vector<AliasSkinVertex> vertices(vertexCount);
                for (int i = 0; i < vertexCount; i++) {
                    stream->read((char *)&vertices[i].onseam, sizeof(int32_t));
                    stream->read((char *)&vertices[i].s, sizeof(int32_t));
                    stream->read((char *)&vertices[i].t, sizeof(int32_t));
                }
                
                // now stream is at the first skin triangle
                std::vector<AliasSkinTriangle> triangles(triangleCount);
                for (int i = 0; i < triangleCount; i++) {
                    stream->read((char *)&triangles[i].front, sizeof(int32_t));
                    for (unsigned int j = 0; j < 3; j++)
                        stream->read((char *)&triangles[i].vertices[j], sizeof(int32_t));
                }
                
                // now stream is at the first frame
                for (int i = 0; i < frameCount; i++) {
                    int32_t type;
                    stream->read((char *)&type, sizeof(int32_t));
                    if (type == 0) { // single frame
                        frames.push_back(readFrame(stream, origin, scale, skinWidth, skinHeight, vertices, triangles));
                    } else { // frame group
                        int32_t groupFrameCount;
                        std::streampos base = stream->tellg();
                        stream->read((char *)&groupFrameCount, sizeof(int32_t));
                        
                        std::streampos timePos = MDL_MULTI_FRAME_TIMES + base;
                        std::streampos framePos = MDL_MULTI_FRAME_TIMES + groupFrameCount * sizeof(float) + base;
                        
                        std::vector<float> groupFrameTimes(groupFrameCount);
                        std::vector<AliasSingleFrame*> groupFrames(groupFrameCount);
                        for (int j = 0; j < groupFrameCount; j++) {
                            stream->seekg(timePos, std::ios::beg);
                            stream->read((char *)&groupFrameTimes[i], sizeof(float));
                            timePos = j * sizeof(float) + timePos;
                            
                            stream->seekg(framePos, std::ios::beg);
                            groupFrames[j] = readFrame(stream, origin, scale, skinWidth, skinHeight, vertices, triangles);
                            framePos = MDL_SIMPLE_FRAME_NAME_SIZE + (vertexCount + 2) * MDL_FRAME_VERTEX_SIZE + framePos;
                        }
                        
                        AliasFrameGroup* frameGroup = new AliasFrameGroup(groupFrameTimes, groupFrames);
                        frames.push_back(frameGroup);
                    }
                }
            }
            
            Alias::~Alias() {
                while(!frames.empty()) delete frames.back(), frames.pop_back();
                while(!skins.empty()) delete skins.back(), skins.pop_back();
            }
            
            AliasSingleFrame& Alias::firstFrame() {
                return *frames[0]->firstFrame();
            }
            
            AliasManager* AliasManager::sharedManager = NULL;
            
            Alias* AliasManager::aliasForName(const std::string& name, const std::vector<std::string>& paths) {
                std::string pathList = accumulate(paths.begin(), paths.end(), std::string(";"));
                std::string key = pathList + ":" + name;
                
                std::map<std::string, Alias*>::iterator it = aliases.find(key);
                if (it != aliases.end())
                    return it->second;
                
                log(TB_LL_INFO, "Loading alias model '%s', search paths: %s\n", name.c_str(), pathList.c_str());
                
                IO::PakManager& pakManager = *IO::PakManager::sharedManager;
                IO::PakStream stream = pakManager.streamForEntry(name, paths);
                if (stream.get() != NULL) {
                    Alias* alias = new Alias(name, stream);
                    aliases[key] = alias;
                    return alias;
                }
                
                log(TB_LL_WARN, "Unable to find alias model '%s'\n", name.c_str());
                return NULL;
            }
            
            AliasManager::AliasManager() {}
            
            AliasManager::~AliasManager() {
                for (std::map<std::string, Alias*>::iterator it = aliases.begin(); it != aliases.end(); ++it)
                    delete it->second;
            }
        }
    }
}