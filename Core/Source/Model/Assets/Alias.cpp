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
#include "AliasNormals.h"

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
            
            AliasSkin::AliasSkin(const unsigned char* picture, int width, int height) : width(width), height(height) {
                count = 1;
                pictures.push_back(picture);
            }
            
            AliasSkin::AliasSkin(vector<const unsigned char*>& pictures, vector<float>& times, int count, int width, int height) : pictures(pictures), times(times), count(count), width(width), height(height) {
                assert(pictures.size() == times.size());
            }
            
            AliasSkin::~AliasSkin() {
                while(!pictures.empty()) delete pictures.back(), pictures.pop_back();
            }
            
            AliasSingleFrame::AliasSingleFrame(string& name, vector<AliasFrameTriangle*>& triangles, Vec3f center, BBox bounds, BBox maxBounds) : name(name), triangles(triangles), center(center), bounds(bounds), maxBounds(maxBounds) {}
            
            AliasSingleFrame* AliasSingleFrame::firstFrame() {
                return this;
            }
            
            AliasFrameGroup::AliasFrameGroup(vector<float>& times, vector<AliasSingleFrame*>& frames)
            : times(times), frames(frames) {
                assert(times.size() == frames.size());
                
                if (frames.size() > 0) {
                    bounds = frames[0]->bounds;
                    for (int i = 1; i < frames.size(); i++)
                        bounds += frames[0]->bounds;
                } else {
                    bounds.min = Null3f;
                    bounds.max = Null3f;
                }
            }
            
            AliasSingleFrame* AliasFrameGroup::firstFrame() {
                return frames[0];
            }
            
            Vec3f Alias::unpackFrameVertex(AliasPackedFrameVertex& packedVertex, Vec3f origin, Vec3f size) {
                Vec3f vertex;
                vertex.x = size.x * packedVertex.x + origin.x;
                vertex.y = size.y * packedVertex.y + origin.y;
                vertex.z = size.z * packedVertex.z + origin.z;
                return vertex;
            }
            
            AliasSingleFrame* Alias::readFrame(IO::PakStream& stream, Vec3f origin, Vec3f scale, int skinWidth, int skinHeight, vector<AliasSkinVertex>& vertices, vector<AliasSkinTriangle>& triangles) {
                char name[MDL_SIMPLE_FRAME_NAME_SIZE];
                stream->seekg(MDL_SIMPLE_FRAME_NAME, ios::cur);
                stream->read(name, MDL_SIMPLE_FRAME_NAME_SIZE);
                
                vector<AliasPackedFrameVertex> packedFrameVertices(vertices.size());
                for (int i = 0; i < vertices.size(); i++) {
                    AliasPackedFrameVertex packedVertex;
                    stream->read((char *)&packedVertex, 4 * sizeof(unsigned char));
                    packedFrameVertices[i] = packedVertex;
                }
                
                vector<Vec3f> frameVertices(vertices.size());
                Vec3f center;
                BBox bounds;
                
                frameVertices[0] = unpackFrameVertex(packedFrameVertices[0], origin, scale);
                center = frameVertices[0];
                bounds.min = frameVertices[0];
                bounds.max = frameVertices[0];
                
                for (int i = 1; i < vertices.size(); i++) {
                    frameVertices[i] = unpackFrameVertex(packedFrameVertices[i], origin, scale);
                    center += frameVertices[i];
                    bounds += frameVertices[i];
                }
                
                center /= static_cast<float>(vertices.size());
                
                Vec3f diff = frameVertices[0] - center;
				float distSquared = diff.lengthSquared();
                for (int i = 1; i < vertices.size(); i++) {
                    diff = frameVertices[i] - center;
                    distSquared = Math::fmax(distSquared, diff.lengthSquared());
                }
                
                float dist = sqrt(distSquared);
                
                BBox maxBounds;
                maxBounds.min = center;
                maxBounds.min.x -= dist;
                maxBounds.min.y -= dist;
                maxBounds.min.z -= dist;
                maxBounds.max = center;
                maxBounds.max.x += dist;
                maxBounds.max.y += dist;
                maxBounds.max.z += dist;
                
                vector<AliasFrameTriangle*> frameTriangles(triangles.size());
                for (int i = 0; i < triangles.size(); i++) {
                    AliasFrameTriangle* frameTriangle = new AliasFrameTriangle();
                    for (int j = 0; j < 3; j++) {
                        int index = triangles[i].vertices[j];
                        frameTriangle->vertices[j].position = frameVertices[index];
                        frameTriangle->vertices[j].normal = AliasNormals[packedFrameVertices[index].i];
                        frameTriangle->vertices[j].texCoords.x = (float)vertices[index].s / skinWidth;
                        frameTriangle->vertices[j].texCoords.y = (float)vertices[index].t / skinHeight;
                        if (vertices[index].onseam && !triangles[i].front)
                            frameTriangle->vertices[j].texCoords.x += 0.5f;
                    }
                    frameTriangles[i] = frameTriangle;
                }
                
                string cppName(name);
                return new AliasSingleFrame(cppName, frameTriangles, center, bounds, maxBounds);
            }
            
            AliasSingleFrame::~AliasSingleFrame() {
                while(!triangles.empty()) delete triangles.back(), triangles.pop_back();
            }
            
            Alias::Alias(string& name, IO::PakStream stream) : name(name) {
                Vec3f scale, origin;
                int32_t skinCount, skinWidth, skinHeight, skinSize;
                int32_t vertexCount, triangleCount, frameCount;
                
                stream->seekg(MDL_HEADER_SCALE, ios::beg);
                stream->read((char *)&scale, sizeof(Vec3f));
                stream->read((char *)&origin, sizeof(Vec3f));
                
                stream->seekg(MDL_HEADER_NUMSKINS, ios::beg);
                stream->read((char *)&skinCount, sizeof(int32_t));
                stream->read((char *)&skinWidth, sizeof(int32_t));
                stream->read((char *)&skinHeight, sizeof(int32_t));
                skinSize = skinWidth * skinHeight;
                
                stream->read((char *)&vertexCount, sizeof(int32_t));
                stream->read((char *)&triangleCount, sizeof(int32_t));
                stream->read((char *)&frameCount, sizeof(int32_t));
                
                stream->seekg(MDL_SKINS, ios::beg);
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
                        vector<float> times(numPics);
                        vector<const unsigned char *> skinPictures(numPics);
                        
                        streampos base = stream->tellg();
                        for (int j = 0; j < numPics; j++) {
                            stream->seekg(j * sizeof(float) + base, ios::beg);
                            stream->read((char *)&times[i], sizeof(float));
                            
                            unsigned char* skinPicture = new unsigned char[skinSize];
                            stream->seekg(numPics * sizeof(float) + j * skinSize + base, ios::beg);
                            stream->read((char *)&skinPicture, skinSize);
                            
                            skinPictures[i] = skinPicture;
                        }
                        
                        AliasSkin* skin = new AliasSkin(skinPictures, times, numPics, skinWidth, skinHeight);
                        skins.push_back(skin);
                    }
                }
                
                // now stream is at the first skin vertex
                vector<AliasSkinVertex> vertices(vertexCount);
                for (int i = 0; i < vertexCount; i++) {
                    stream->read((char *)&vertices[i].onseam, sizeof(int32_t));
                    stream->read((char *)&vertices[i].s, sizeof(int32_t));
                    stream->read((char *)&vertices[i].t, sizeof(int32_t));
                }
                
                // now stream is at the first skin triangle
                vector<AliasSkinTriangle> triangles(triangleCount);
                for (int i = 0; i < triangleCount; i++) {
                    stream->read((char *)&triangles[i].front, sizeof(int32_t));
                    for (int j = 0; j < 3; j++)
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
                        streampos base = stream->tellg();
                        stream->read((char *)&groupFrameCount, sizeof(int32_t));
                        
                        streampos timePos = MDL_MULTI_FRAME_TIMES + base;
                        streampos framePos = MDL_MULTI_FRAME_TIMES + groupFrameCount * sizeof(float) + base;
                        
                        vector<float> groupFrameTimes(groupFrameCount);
                        vector<AliasSingleFrame*> groupFrames(groupFrameCount);
                        for (int j = 0; j < groupFrameCount; j++) {
                            stream->seekg(timePos, ios::beg);
                            stream->read((char *)&groupFrameTimes[i], sizeof(float));
                            timePos = j * sizeof(float) + timePos;
                            
                            stream->seekg(framePos, ios::beg);
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
            
            Alias* AliasManager::aliasForName(string& name, vector<string>& paths) {
                string pathList = accumulate(paths.begin(), paths.end(), string(";"));
                string key = pathList + ":" + name;
                
                map<string, Alias*>::iterator it = aliases.find(key);
                if (it != aliases.end())
                    return it->second;
                
                fprintf(stdout, "Loading alias model '%s', search paths: %s\n", name.c_str(), pathList.c_str());
                
                IO::PakManager& pakManager = *IO::PakManager::sharedManager;
                IO::PakStream stream = pakManager.streamForEntry(name, paths);
                if (stream.get() != NULL) {
                    Alias* alias = new Alias(name, stream);
                    aliases[key] = alias;
                    return alias;
                }
                
                fprintf(stdout, "Warning: Unable to find alias model '%s'\n", name.c_str());
                return NULL;
            }
            
            AliasManager::AliasManager() {}
            
            AliasManager::~AliasManager() {
                for (map<string, Alias*>::iterator it = aliases.begin(); it != aliases.end(); ++it)
                    delete it->second;
            }
        }
    }
}