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
#include "Macros.h"
#include "IO/DiskFileSystem.h"
#include "IO/Path.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace IO {
        class QuakeFileSerializer : public MapFileSerializer {
        private:
            String FacePointFormat;
            String TextureInfoFormat;
        public:
            QuakeFileSerializer(FILE* stream) :
            MapFileSerializer(stream),
            FacePointFormat(getFacePointFormat()),
            TextureInfoFormat(" %s %.6g %.6g %.6g %.6g %.6g") {}
        private:
            static String getFacePointFormat() {
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
                "%." << FloatPrecision << "g )";
                return str.str();
            }
        private:
            size_t doWriteBrushFace(FILE* stream, Model::BrushFace* face) override {
                writeFacePoints(stream, face);
                writeTextureInfo(stream, face);
                std::fprintf(stream, "\n");
                return 1;
            }
        protected:
            void writeFacePoints(FILE* stream, Model::BrushFace* face) {
                const Model::BrushFace::Points& points = face->points();

                std::fprintf(stream, FacePointFormat.c_str(),
                             points[0].x(),
                             points[0].y(),
                             points[0].z(),
                             points[1].x(),
                             points[1].y(),
                             points[1].z(),
                             points[2].x(),
                             points[2].y(),
                             points[2].z());
            }

            void writeTextureInfo(FILE* stream, Model::BrushFace* face) {
                const String& textureName = face->textureName().empty() ? Model::BrushFace::NoTextureName : face->textureName();
                std::fprintf(stream, TextureInfoFormat.c_str(),
                             textureName.c_str(),
                             face->xOffset(),
                             face->yOffset(),
                             face->rotation(),
                             face->xScale(),
                             face->yScale());
            }
        };

        class Quake2FileSerializer : public QuakeFileSerializer {
        private:
            String SurfaceAttributesFormat;
        public:
            Quake2FileSerializer(FILE* stream) :
            QuakeFileSerializer(stream),
            SurfaceAttributesFormat(" %d %d %.6g") {}
        private:
            size_t doWriteBrushFace(FILE* stream, Model::BrushFace* face) override {
                writeFacePoints(stream, face);
                writeTextureInfo(stream, face);

                if (face->hasSurfaceAttributes()) {
                    writeSurfaceAttributes(stream, face);
                }

                std::fprintf(stream, "\n");
                return 1;
            }
        protected:
            void writeSurfaceAttributes(FILE* stream, Model::BrushFace* face) {
                std::fprintf(stream, SurfaceAttributesFormat.c_str(),
                             face->surfaceContents(),
                             face->surfaceFlags(),
                             face->surfaceValue());
            }
        };


        class DaikatanaFileSerializer : public Quake2FileSerializer {
        private:
            String SurfaceColorFormat;
        public:
            DaikatanaFileSerializer(FILE* stream) :
            Quake2FileSerializer(stream),
            SurfaceColorFormat(" %d %d %d") {}
        private:
            size_t doWriteBrushFace(FILE* stream, Model::BrushFace* face) override {
                writeFacePoints(stream, face);
                writeTextureInfo(stream, face);

                if (face->hasSurfaceAttributes() || face->hasColor()) {
                    writeSurfaceAttributes(stream, face);
                }
                if (face->hasColor()) {
                    writeSurfaceColor(stream, face);
                }

                std::fprintf(stream, "\n");
                return 1;
            }
        protected:
            void writeSurfaceColor(FILE* stream, Model::BrushFace* face) {
                std::fprintf(stream, SurfaceColorFormat.c_str(),
                             static_cast<int>(face->color().r()),
                             static_cast<int>(face->color().g()),
                             static_cast<int>(face->color().b()));
            }
        };

        class Hexen2FileSerializer : public QuakeFileSerializer {
        public:
            Hexen2FileSerializer(FILE* stream):
            QuakeFileSerializer(stream) {}
        private:
            size_t doWriteBrushFace(FILE* stream, Model::BrushFace* face) override {
                writeFacePoints(stream, face);
                writeTextureInfo(stream, face);
                std::fprintf(stream, " 0\n"); // extra value written here
                return 1;
            }
        };

        class ValveFileSerializer : public QuakeFileSerializer {
        private:
            String ValveTextureInfoFormat;
        public:
            ValveFileSerializer(FILE* stream) :
            QuakeFileSerializer(stream),
            ValveTextureInfoFormat(" %s [ %.6g %.6g %.6g %.6g ] [ %.6g %.6g %.6g %.6g ] %.6g %.6g %.6g") {}
        private:
            size_t doWriteBrushFace(FILE* stream, Model::BrushFace* face) override {
                writeFacePoints(stream, face);
                writeValveTextureInfo(stream, face);
                std::fprintf(stream, "\n");
                return 1;
            }
        private:
            void writeValveTextureInfo(FILE* stream, Model::BrushFace* face) {
                const String& textureName = face->textureName().empty() ? Model::BrushFace::NoTextureName : face->textureName();
                const vm::vec3 xAxis = face->textureXAxis();
                const vm::vec3 yAxis = face->textureYAxis();

                std::fprintf(stream, ValveTextureInfoFormat.c_str(),
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
            }
        };

        NodeSerializer::Ptr MapFileSerializer::create(const Model::MapFormat format, FILE* stream) {
            switch (format) {
                case Model::MapFormat::Standard:
                    return NodeSerializer::Ptr(new QuakeFileSerializer(stream));
                case Model::MapFormat::Quake2:
                    // TODO 2427: Implement Quake3 serializers and use them
                case Model::MapFormat::Quake3:
                case Model::MapFormat::Quake3_Legacy:
                    return NodeSerializer::Ptr(new Quake2FileSerializer(stream));
                case Model::MapFormat::Daikatana:
                    return NodeSerializer::Ptr(new DaikatanaFileSerializer(stream));
                case Model::MapFormat::Valve:
                    return NodeSerializer::Ptr(new ValveFileSerializer(stream));
                case Model::MapFormat::Hexen2:
                    return NodeSerializer::Ptr(new Hexen2FileSerializer(stream));
                case Model::MapFormat::Unknown:
                    throw FileFormatException("Unknown map file format");
                switchDefault()
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
