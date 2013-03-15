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

#include "MapWriter.h"

#include "Model/Texture.h"
#include "Model/Brush.h"
#include "Model/BrushGeometry.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/Face.h"
#include "Model/Map.h"
#include "IO/FileManager.h"
#include "Utility/CRC32.h"

#include <cassert>
#include <fstream>
#include <limits>

#if defined _MSC_VER
#include <cstdint>
#elif defined __GNUC__
#include <stdint.h>
#endif

namespace TrenchBroom {
    namespace IO {
        String MapWriter::format(float f) {
            m_floatBuffer.str("");
            m_floatBuffer.width(std::numeric_limits<float>::digits10 + 2);
            m_floatBuffer << std::fixed << f;
            
            String str = m_floatBuffer.str();
            size_t lastZero = str.size();
            for (size_t i = str.size() - 1; i != 0; --i) {
                if (str[i] == '0')
                    lastZero = i;
                else
                    break;
            }
            if (str[--lastZero] == '.')
                --lastZero;
            
            return str.substr(0, lastZero + 1);
        }

        String MapWriter::format(float f, uint32_t& crc) {
            String str = format(f);
            crc = Utility::updateCRC32(str.c_str(), str.length(), crc);
            return str;
        }

        void MapWriter::writeBrushGeometry(const Model::Brush& brush, FILE* stream, uint32_t& crc) {
            typedef std::map<const Model::Vertex*, size_t> VertexIndices;
            typedef std::map<const Model::Edge*, size_t> EdgeIndices;
            typedef std::map<const Model::Face*, size_t> FaceIndices;
            
            const Model::VertexList& vertices = brush.vertices();
            const Model::EdgeList& edges = brush.edges();
            const Model::FaceList& faces = brush.faces();

            VertexIndices vertexIndices;
            EdgeIndices edgeIndices;
            FaceIndices faceIndices;
            
            for (size_t i = 0; i < faces.size(); i++) {
                const Model::Face* face = faces[i];
                faceIndices[face] = i;
            }

            fprintf(stream, "/// VertexData\n");
            fprintf(stream, "/// {");
            for (size_t i = 0; i < vertices.size(); i++) {
                const Model::Vertex* vertex = vertices[i];
                const String x = format(vertex->position.x, crc);
                const String y = format(vertex->position.y, crc);
                const String z = format(vertex->position.z, crc);
                fprintf(stream, " ( %s %s %s )", x.c_str(), y.c_str(), z.c_str());
                vertexIndices[vertex] = i;
            }
            fprintf(stream, " }\n");
            
            fprintf(stream, "/// {");
            for (size_t i = 0; i < edges.size(); i++) {
                const Model::Edge* edge = edges[i];
                fprintf(stream, " ( %lu %lu %lu %lu )",
                        vertexIndices[edge->start],
                        vertexIndices[edge->end],
                        faceIndices[edge->left->face],
                        faceIndices[edge->right->face]);
                edgeIndices[edge] = i;
                
                crc = Utility::updateCRC32(vertexIndices[edge->start], crc);
                crc = Utility::updateCRC32(vertexIndices[edge->end], crc);
                crc = Utility::updateCRC32(faceIndices[edge->left->face], crc);
                crc = Utility::updateCRC32(faceIndices[edge->right->face], crc);
            }
            fprintf(stream, " }\n");
            
            fprintf(stream, "/// {");
            for (size_t i = 0; i < faces.size(); i++) {
                const Model::Face* face = faces[i];
                const Model::EdgeList& faceEdges = face->edges();

                fprintf(stream, " (");
                for (size_t j = 0; j < faceEdges.size(); j++) {
                    const Model::Vertex* vertex = face->vertices()[j];
                    const Model::Edge* edge = face->edges()[j];
                    fprintf(stream, " %lu %lu", vertexIndices[vertex], edgeIndices[edge]);
                    
                    crc = Utility::updateCRC32(vertexIndices[vertex], crc);
                    crc = Utility::updateCRC32(edgeIndices[edge], crc);
                }
                fprintf(stream, " )");
                
            }
            fprintf(stream, " }\n");
            fprintf(stream, "/// CRC %u\n", ~crc);
        }

        void MapWriter::writeFace(const Model::Face& face, FILE* stream, uint32_t& crc) {
            const String textureName = Utility::isBlank(face.textureName()) ? Model::Texture::Empty : face.textureName();

            const String p0x = format(face.point(0).x, crc);
            const String p0y = format(face.point(0).y, crc);
            const String p0z = format(face.point(0).z, crc);
            const String p1x = format(face.point(1).x, crc);
            const String p1y = format(face.point(1).y, crc);
            const String p1z = format(face.point(1).z, crc);
            const String p2x = format(face.point(2).x, crc);
            const String p2y = format(face.point(2).y, crc);
            const String p2z = format(face.point(2).z, crc);
            const String xOffset = format(face.xOffset());
            const String yOffset = format(face.yOffset());
            const String rotation = format(face.rotation());
            const String xScale = format(face.xScale());
            const String yScale = format(face.yScale());
            
            fprintf(stream, "( %s %s %s ) ( %s %s %s ) ( %s %s %s ) %s %s %s %s %s %s\n",
                    p0x.c_str(),
                    p0y.c_str(),
                    p0z.c_str(),
                    p1x.c_str(),
                    p1y.c_str(),
                    p1z.c_str(),
                    p2x.c_str(),
                    p2y.c_str(),
                    p2z.c_str(),
                    textureName.c_str(),
                    xOffset.c_str(),
                    yOffset.c_str(),
                    rotation.c_str(),
                    xScale.c_str(),
                    yScale.c_str());
        }
        
        void MapWriter::writeBrush(const Model::Brush& brush, FILE* stream) {
            uint32_t crc = 0xFFFFFFFF;
            fprintf(stream, "{\n");
            const Model::FaceList& faces = brush.faces();
            Model::FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt)
                writeFace(**faceIt, stream, crc);
            writeBrushGeometry(brush, stream, crc);
            fprintf(stream, "}\n");
        }
        
        void MapWriter::writeEntityHeader(const Model::Entity& entity, FILE* stream) {
            fprintf(stream, "{\n");
            
            const Model::PropertyList& properties = entity.properties();
            Model::PropertyList::const_iterator it, end;
            for (it = properties.begin(), end = properties.end(); it != end; ++it) {
                const Model::Property& property = *it;
                fprintf(stream, "\"%s\" \"%s\"\n", property.key().c_str(), property.value().c_str());
            }
        }
        
        void MapWriter::writeEntityFooter(FILE* stream) {
            fprintf(stream, "}\n");
        }
        
        void MapWriter::writeEntity(const Model::Entity& entity, FILE* stream) {
            writeEntityHeader(entity, stream);
            const Model::BrushList& brushes = entity.brushes();
            for (unsigned int i = 0; i < brushes.size(); i++)
                writeBrush(*brushes[i], stream);
            writeEntityFooter(stream);
        }

        void MapWriter::writeBrushGeometry(const Model::Brush& brush, std::ostream& stream, uint32_t& crc) {
            typedef std::map<const Model::Vertex*, size_t> VertexIndices;
            typedef std::map<const Model::Edge*, size_t> EdgeIndices;
            typedef std::map<const Model::Face*, size_t> FaceIndices;
            
            const Model::VertexList& vertices = brush.vertices();
            const Model::EdgeList& edges = brush.edges();
            const Model::FaceList& faces = brush.faces();
            
            VertexIndices vertexIndices;
            EdgeIndices edgeIndices;
            FaceIndices faceIndices;
            
            for (size_t i = 0; i < faces.size(); i++) {
                const Model::Face* face = faces[i];
                faceIndices[face] = i;
            }

            stream << "/// VertexData\n";
            stream << "/// {";
            for (size_t i = 0; i < vertices.size(); i++) {
                const Model::Vertex* vertex = vertices[i];
                const String x = format(vertex->position.x, crc);
                const String y = format(vertex->position.y, crc);
                const String z = format(vertex->position.z, crc);
                stream << " ( " << x << " " << y << " " << z << " )";
                vertexIndices[vertex] = i;
            }
            stream << " }\n";
            
            stream << "/// {";
            for (size_t i = 0; i < edges.size(); i++) {
                const Model::Edge* edge = edges[i];
                
                stream <<
                " ( " <<
                vertexIndices[edge->start] << " " <<
                vertexIndices[edge->end] << " " <<
                faceIndices[edge->left->face] << " " <<
                faceIndices[edge->right->face] << " )";

                edgeIndices[edge] = i;
                
                crc = Utility::updateCRC32(vertexIndices[edge->start], crc);
                crc = Utility::updateCRC32(vertexIndices[edge->end], crc);
                crc = Utility::updateCRC32(faceIndices[edge->left->face], crc);
                crc = Utility::updateCRC32(faceIndices[edge->right->face], crc);
            }
            stream << " }\n";
            
            stream << "/// {";
            for (size_t i = 0; i < faces.size(); i++) {
                const Model::Face* face = faces[i];
                const Model::EdgeList& faceEdges = face->edges();
                
                stream << " (";
                for (size_t j = 0; j < faceEdges.size(); j++) {
                    const Model::Vertex* vertex = face->vertices()[j];
                    const Model::Edge* edge = face->edges()[j];
                    stream << " " << vertexIndices[vertex] << " " << edgeIndices[edge];

                    crc = Utility::updateCRC32(vertexIndices[vertex], crc);
                    crc = Utility::updateCRC32(edgeIndices[edge], crc);
                }
                stream << " )";
                
            }
            stream << " }\n";
            stream << "/// CRC " << ~crc << "\n";
        }
        
        void MapWriter::writeFace(const Model::Face& face, std::ostream& stream, uint32_t& crc) {
            String textureName = Utility::isBlank(face.textureName()) ? Model::Texture::Empty : face.textureName();
            
            const String p0x = format(face.point(0).x, crc);
            const String p0y = format(face.point(0).y, crc);
            const String p0z = format(face.point(0).z, crc);
            const String p1x = format(face.point(1).x, crc);
            const String p1y = format(face.point(1).y, crc);
            const String p1z = format(face.point(1).z, crc);
            const String p2x = format(face.point(2).x, crc);
            const String p2y = format(face.point(2).y, crc);
            const String p2z = format(face.point(2).z, crc);
            const String xOffset = format(face.xOffset());
            const String yOffset = format(face.yOffset());
            const String rotation = format(face.rotation());
            const String xScale = format(face.xScale());
            const String yScale = format(face.yScale());

            stream <<
            "( " <<
            p0x         << " " <<
            p0y         << " " <<
            p0z         <<
            " ) ( "     <<
            p1x         << " " <<
            p1y         << " " <<
            p1z         <<
            " ) ( "     <<
            p2x         << " " <<
            p2y         << " " <<
            p2z         <<
            " ) "       <<
            textureName << " " <<
            xOffset     << " " <<
            yOffset     << " " <<
            rotation    << " " <<
            xScale      << " " <<
            yScale      << "\n";
        }

        void MapWriter::writeBrush(const Model::Brush& brush, std::ostream& stream) {
            uint32_t crc = 0xFFFFFFFF;
            stream << "{" << "\n";
            const Model::FaceList& faces = brush.faces();
            Model::FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = faces.begin(), faceEnd = faces.end(); faceIt != faceEnd; ++faceIt)
                writeFace(**faceIt, stream, crc);
            writeBrushGeometry(brush, stream, crc);
            stream << "}" << "\n";
        }

        void MapWriter::writeEntityHeader(const Model::Entity& entity, std::ostream& stream) {
            stream << "{" << "\n";
            
            const Model::PropertyList& properties = entity.properties();
            Model::PropertyList::const_iterator it, end;
            for (it = properties.begin(), end = properties.end(); it != end; ++it) {
                const Model::Property& property = *it;
                stream << "\"" << property.key() << "\" \"" << property.value() << "\"" << "\n";
            }
        }
        
        void MapWriter::writeEntityFooter(std::ostream& stream) {
            stream << "}" << "\n";
        }

        void MapWriter::writeEntity(const Model::Entity& entity, std::ostream& stream) {
            writeEntityHeader(entity, stream);
            const Model::BrushList& brushes = entity.brushes();
            for (unsigned int i = 0; i < brushes.size(); i++)
                writeBrush(*brushes[i], stream);
            writeEntityFooter(stream);
        }

        void MapWriter::writeObjectsToStream(const Model::EntityList& pointEntities, const Model::BrushList& brushes, std::ostream& stream) {
            assert(stream.good());

            Model::Entity* worldspawn = NULL;
            
            // group the brushes by their containing entities
            typedef std::map<Model::Entity*, Model::BrushList> EntityBrushMap;
            EntityBrushMap entityToBrushes;
            
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                Model::Brush& brush = **brushIt;
                Model::Entity& entity = *brush.entity();
                entityToBrushes[&entity].push_back(&brush);
                if (entity.worldspawn())
                    worldspawn = &entity;
            }
            
            // write worldspawn first
            if (worldspawn != NULL) {
                Model::BrushList& brushList = entityToBrushes[worldspawn];
                writeEntityHeader(*worldspawn, stream);
                for (brushIt = brushList.begin(), brushEnd = brushList.end(); brushIt != brushEnd; ++brushIt) {
                    writeBrush(**brushIt, stream);
                }
                writeEntityFooter(stream);
            }
            
            // now write the point entities
            Model::EntityList::const_iterator entityIt, entityEnd;
            for (entityIt = pointEntities.begin(), entityEnd = pointEntities.end(); entityIt != entityEnd; ++entityIt) {
                Model::Entity& entity = **entityIt;
                assert(entity.definition()->type() == Model::EntityDefinition::PointEntity);
                writeEntity(entity, stream);
            }

            // finally write the brush entities
            EntityBrushMap::iterator it, end;
            for (it = entityToBrushes.begin(), end = entityToBrushes.end(); it != end; ++it) {
                Model::Entity* entity = it->first;
                if (entity != worldspawn) {
                    Model::BrushList& brushList = it->second;
                    writeEntityHeader(*entity, stream);
                    for (brushIt = brushList.begin(), brushEnd = brushList.end(); brushIt != brushEnd; ++brushIt) {
                        writeBrush(**brushIt, stream);
                    }
                    writeEntityFooter(stream);
                }
            }
        }
        
        void MapWriter::writeFacesToStream(const Model::FaceList& faces, std::ostream& stream) {
            assert(stream.good());
            
            uint32_t crc = 0xFFFFFFFF;
            for (unsigned int i = 0; i < faces.size(); i++)
                writeFace(*faces[i], stream, crc);
        }

        void MapWriter::writeToStream(const Model::Map& map, std::ostream& stream) {
            assert(stream.good());
            
            const Model::EntityList& entities = map.entities();
            for (unsigned int i = 0; i < entities.size(); i++)
                writeEntity(*entities[i], stream);
        }
        
        void MapWriter::writeToFileAtPath(const Model::Map& map, const String& path, bool overwrite) {
            FileManager fileManager;
            if (fileManager.exists(path) && !overwrite)
                return;
            
            const String directoryPath = fileManager.deleteLastPathComponent(path);
            if (!fileManager.exists(directoryPath))
                fileManager.makeDirectory(directoryPath);
            
            FILE* stream = fopen(path.c_str(), "w");
            // std::fstream stream(path.c_str(), std::ios::out | std::ios::trunc);

            const Model::EntityList& entities = map.entities();
            for (unsigned int i = 0; i < entities.size(); i++)
                writeEntity(*entities[i], stream);
            fclose(stream);
        }
    }
}
