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

#include "MapFileSerializer.h"
#include "Exceptions.h"
#include "IO/DiskFileSystem.h"
#include "IO/Path.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace IO {
        class StandardFileSerializer : public MapFileSerializer {
        public:
            enum class Format {
                Quake,
                Quake2,
                Daikatana
            };
        private:
            Format m_format;
            String FaceFormat;
            String DkExtraFormat;
        public:
            StandardFileSerializer(FILE* stream, const Format format) :
            MapFileSerializer(stream),
            m_format(format) {
                StringStream str;
                str <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "%s %.6g %.6g %.6g %.6g %.6g";
                if (m_format != Format::Quake) {
                    str << " %d %d %.6g";
                }
                FaceFormat = str.str();
                DkExtraFormat = " %d %d %d";
            }
        private:
            size_t doWriteBrushFace(FILE* stream, Model::BrushFace* face) override {
                const String& textureName = face->textureName().empty() ? Model::BrushFace::NoTextureName : face->textureName();
                const Model::BrushFace::Points& points = face->points();

                switch (m_format) {
                    case Format::Quake:
                        std::fprintf(stream, FaceFormat.c_str(),
                                     points[0].x(),
                                     points[0].y(),
                                     points[0].z(),
                                     points[1].x(),
                                     points[1].y(),
                                     points[1].z(),
                                     points[2].x(),
                                     points[2].y(),
                                     points[2].z(),
                                     textureName.c_str(),
                                     face->xOffset(),
                                     face->yOffset(),
                                     face->rotation(),
                                     face->xScale(),
                                     face->yScale());
                        break;
                    case Format::Quake2:
                        std::fprintf(stream, FaceFormat.c_str(),
                                     points[0].x(),
                                     points[0].y(),
                                     points[0].z(),
                                     points[1].x(),
                                     points[1].y(),
                                     points[1].z(),
                                     points[2].x(),
                                     points[2].y(),
                                     points[2].z(),
                                     textureName.c_str(),
                                     face->xOffset(),
                                     face->yOffset(),
                                     face->rotation(),
                                     face->xScale(),
                                     face->yScale(),
                                     face->surfaceContents(),
                                     face->surfaceFlags(),
                                     face->surfaceValue());
                        break;
                    case Format::Daikatana:
                        std::fprintf(stream, FaceFormat.c_str(),
                                     points[0].x(),
                                     points[0].y(),
                                     points[0].z(),
                                     points[1].x(),
                                     points[1].y(),
                                     points[1].z(),
                                     points[2].x(),
                                     points[2].y(),
                                     points[2].z(),
                                     textureName.c_str(),
                                     face->xOffset(),
                                     face->yOffset(),
                                     face->rotation(),
                                     face->xScale(),
                                     face->yScale(),
                                     face->surfaceContents(),
                                     face->surfaceFlags(),
                                     face->surfaceValue());
                        if (face->hasColor()) {
                            std::fprintf(stream, DkExtraFormat.c_str(),
                                         face->color().r() * 255.0f,
                                         face->color().g() * 255.0f,
                                         face->color().b() * 255.0f);
                        }
                        break;
                }

                std::fprintf(stream, "\n");
                return 1;
            }
        };
        
        class Hexen2FileSerializer : public MapFileSerializer {
        private:
            String FaceFormat;
        public:
            Hexen2FileSerializer(FILE* stream) :
            MapFileSerializer(stream) {
                StringStream str;
                str <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "%s %.6g %.6g %.6g %.6g %.6g 0\n"; // the extra value is written here
                
                FaceFormat = str.str();
            }
        private:
            size_t doWriteBrushFace(FILE* stream, Model::BrushFace* face) override {
                const String& textureName = face->textureName().empty() ? Model::BrushFace::NoTextureName : face->textureName();
                const Model::BrushFace::Points& points = face->points();
                
                std::fprintf(stream, FaceFormat.c_str(),
                             points[0].x(),
                             points[0].y(),
                             points[0].z(),
                             points[1].x(),
                             points[1].y(),
                             points[1].z(),
                             points[2].x(),
                             points[2].y(),
                             points[2].z(),
                             textureName.c_str(),
                             face->xOffset(),
                             face->yOffset(),
                             face->rotation(),
                             face->xScale(),
                             face->yScale());
                return 1;
            }
        };
        
        class ValveFileSerializer : public MapFileSerializer {
        private:
            String FaceFormat;
        public:
            ValveFileSerializer(FILE* stream) :
            MapFileSerializer(stream) {
                StringStream str;
                str <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "( %." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g " <<
                "%." << FloatPrecision << "g ) " <<
                "%s " <<
                "[ %.6g %.6g %.6g %.6g ] " <<
                "[ %.6g %.6g %.6g %.6g ] " <<
                "%.6g %.6g %.6g\n";
                
                FaceFormat = str.str();
            }
        private:
            size_t doWriteBrushFace(FILE* stream, Model::BrushFace* face) override {
                const String& textureName = face->textureName().empty() ? Model::BrushFace::NoTextureName : face->textureName();
                const Vec3 xAxis = face->textureXAxis();
                const Vec3 yAxis = face->textureYAxis();
                const Model::BrushFace::Points& points = face->points();
                
                std::fprintf(stream, FaceFormat.c_str(),
                             points[0].x(),
                             points[0].y(),
                             points[0].z(),
                             points[1].x(),
                             points[1].y(),
                             points[1].z(),
                             points[2].x(),
                             points[2].y(),
                             points[2].z(),
                             
                             textureName.c_str(),
                             
                             xAxis.x(),
                             xAxis.y(),
                             xAxis.z(),
                             face->xOffset(),
                             
                             yAxis.x(),
                             yAxis.y(),
                             yAxis.z(),
                             face->yOffset(),
                             
                             face->rotation(),
                             face->xScale(),
                             face->yScale());
                return 1;
            }
        };

        NodeSerializer::Ptr MapFileSerializer::create(const Model::MapFormat::Type format, FILE* stream) {
            switch (format) {
                case Model::MapFormat::Standard:
                    return NodeSerializer::Ptr(new StandardFileSerializer(stream, StandardFileSerializer::Format::Quake));
                case Model::MapFormat::Quake2:
                    return NodeSerializer::Ptr(new StandardFileSerializer(stream, StandardFileSerializer::Format::Quake2));
                case Model::MapFormat::Daikatana:
                    return NodeSerializer::Ptr(new StandardFileSerializer(stream, StandardFileSerializer::Format::Daikatana));
                case Model::MapFormat::Valve:
                    return NodeSerializer::Ptr(new ValveFileSerializer(stream));
                case Model::MapFormat::Hexen2:
                    return NodeSerializer::Ptr(new Hexen2FileSerializer(stream));
                case Model::MapFormat::Unknown:
                default:
                    throw FileFormatException("Unknown map file format");
            }
        }
        
        MapFileSerializer::MapFileSerializer(FILE* stream) :
        m_line(1),
        m_stream(stream) {
            ensure(m_stream != nullptr, "stream is null");
        }
        
        void MapFileSerializer::doBeginFile() {}
        void MapFileSerializer::doEndFile() {}

        void MapFileSerializer::doBeginEntity(const Model::Node* node) {
            std::fprintf(m_stream, "// entity %u\n", entityNo());
            ++m_line;
            m_startLineStack.push_back(m_line);
            std::fprintf(m_stream, "{\n");
            ++m_line;
        }
        
        void MapFileSerializer::doEndEntity(Model::Node* node) {
            std::fprintf(m_stream, "}\n");
            ++m_line;
            setFilePosition(node);
        }
        
        void MapFileSerializer::doEntityAttribute(const Model::EntityAttribute& attribute) { 
            std::fprintf(m_stream, "\"%s\" \"%s\"\n",
                         escapeEntityAttribute( attribute.name()).c_str(),
                         escapeEntityAttribute(attribute.value()).c_str());
            ++m_line;
        }
        
        void MapFileSerializer::doBeginBrush(const Model::Brush* brush) {
            std::fprintf(m_stream, "// brush %u\n", brushNo());
            ++m_line;
            m_startLineStack.push_back(m_line);
            std::fprintf(m_stream, "{\n");
            ++m_line;
        }
        
        void MapFileSerializer::doEndBrush(Model::Brush* brush) {
            std::fprintf(m_stream, "}\n");
            ++m_line;
            setFilePosition(brush);
        }
        
        void MapFileSerializer::doBrushFace(Model::BrushFace* face) {
            const size_t lines = doWriteBrushFace(m_stream, face);
            face->setFilePosition(m_line, lines);
            m_line += lines;
        }
        
        void MapFileSerializer::setFilePosition(Model::Node* node) {
            const size_t start = startLine();
            node->setFilePosition(start, m_line - start);
        }

        size_t MapFileSerializer::startLine() {
            assert(!m_startLineStack.empty());
            const size_t result = m_startLineStack.back();
            m_startLineStack.pop_back();
            return result;
        }
    }
}
