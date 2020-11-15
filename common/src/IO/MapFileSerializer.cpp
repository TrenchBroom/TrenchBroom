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

#include "Ensure.h"
#include "Exceptions.h"
#include "Macros.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/EntityAttributes.h"

#include <fmt/format.h>
#include <fmt/compile.h>

#include <memory>
#include <sstream>

namespace TrenchBroom {
    namespace IO {
        class QuakeFileSerializer : public MapFileSerializer {
        public:
            explicit QuakeFileSerializer(std::ostream& stream) :
            MapFileSerializer(stream) {}
        private:
            size_t doWriteBrushFace(std::vector<char>& out, const Model::BrushFace& face) override {
                writeFacePoints(out, face);
                writeTextureInfo(out, face);
                fmt::format_to(std::back_inserter(out), "\n");
                return 1;
            }
        protected:
            void writeFacePoints(std::vector<char>& out, const Model::BrushFace& face) {
                const Model::BrushFace::Points& points = face.points();

                fmt::format_to(std::back_inserter(out), FMT_COMPILE("( {} {} {} ) ( {} {} {} ) ( {} {} {} )"),
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

            void writeTextureInfo(std::vector<char>& out, const Model::BrushFace& face) {
                const std::string& textureName = face.attributes().textureName().empty() ? Model::BrushFaceAttributes::NoTextureName : face.attributes().textureName();

                fmt::format_to(std::back_inserter(out), FMT_COMPILE(" {} {} {} {} {} {}"),
                             textureName.c_str(),
                             static_cast<double>(face.attributes().xOffset()),
                             static_cast<double>(face.attributes().yOffset()),
                             static_cast<double>(face.attributes().rotation()),
                             static_cast<double>(face.attributes().xScale()),
                             static_cast<double>(face.attributes().yScale()));
            }

            void writeValveTextureInfo(std::vector<char>& out, const Model::BrushFace& face) {
                const std::string& textureName = face.attributes().textureName().empty() ? Model::BrushFaceAttributes::NoTextureName : face.attributes().textureName();
                const vm::vec3 xAxis = face.textureXAxis();
                const vm::vec3 yAxis = face.textureYAxis();

                fmt::format_to(std::back_inserter(out), FMT_COMPILE(" {} [ {} {} {} {} ] [ {} {} {} {} ] {} {} {}"),
                             textureName.c_str(),

                             xAxis.x(),
                             xAxis.y(),
                             xAxis.z(),
                             static_cast<double>(face.attributes().xOffset()),

                             yAxis.x(),
                             yAxis.y(),
                             yAxis.z(),
                             static_cast<double>(face.attributes().yOffset()),

                             static_cast<double>(face.attributes().rotation()),
                             static_cast<double>(face.attributes().xScale()),
                             static_cast<double>(face.attributes().yScale()));
            }
        };

        class Quake2FileSerializer : public QuakeFileSerializer {
        public:
            explicit Quake2FileSerializer(std::ostream& stream) :
            QuakeFileSerializer(stream) {}
        private:
            size_t doWriteBrushFace(std::vector<char>& out, const Model::BrushFace& face) override {
                writeFacePoints(out, face);
                writeTextureInfo(out, face);

                // Neverball's "mapc" doesn't like it if surface attributes aren't present.
                // This suggests the Radiants always output these, so it's probably a compatibility danger.
                writeSurfaceAttributes(out, face);

                fmt::format_to(std::back_inserter(out), "\n");
                return 1;
            }
        protected:
            void writeSurfaceAttributes(std::vector<char>& out, const Model::BrushFace& face) {
                fmt::format_to(std::back_inserter(out), " {} {} {}",
                             face.attributes().surfaceContents(),
                             face.attributes().surfaceFlags(),
                             static_cast<double>(face.attributes().surfaceValue()));
            }
        };

        class Quake2ValveFileSerializer : public Quake2FileSerializer {
        public:
            explicit Quake2ValveFileSerializer(std::ostream& stream) :
            Quake2FileSerializer(stream) {}
        private:
            size_t doWriteBrushFace(std::vector<char>& out, const Model::BrushFace& face) override {
                writeFacePoints(out, face);
                writeValveTextureInfo(out, face);
                writeSurfaceAttributes(out, face);

                fmt::format_to(std::back_inserter(out), "\n");
                return 1;
            }
        };

        class DaikatanaFileSerializer : public Quake2FileSerializer {
        private:
            std::string SurfaceColorFormat;
        public:
            explicit DaikatanaFileSerializer(std::ostream& stream) :
            Quake2FileSerializer(stream),
            SurfaceColorFormat(" %d %d %d") {}
        private:
            size_t doWriteBrushFace(std::vector<char>& out, const Model::BrushFace& face) override {
                writeFacePoints(out, face);
                writeTextureInfo(out, face);

                if (face.attributes().hasSurfaceAttributes() || face.attributes().hasColor()) {
                    writeSurfaceAttributes(out, face);
                }
                if (face.attributes().hasColor()) {
                    writeSurfaceColor(out, face);
                }

                fmt::format_to(std::back_inserter(out), "\n");
                return 1;
            }
        protected:
            void writeSurfaceColor(std::vector<char>& out, const Model::BrushFace& face) {
                fmt::format_to(std::back_inserter(out), " {} {} {}",
                             static_cast<int>(face.attributes().color().r()),
                             static_cast<int>(face.attributes().color().g()),
                             static_cast<int>(face.attributes().color().b()));
            }
        };

        class Hexen2FileSerializer : public QuakeFileSerializer {
        public:
            explicit Hexen2FileSerializer(std::ostream& stream):
            QuakeFileSerializer(stream) {}
        private:
            size_t doWriteBrushFace(std::vector<char>& out, const Model::BrushFace& face) override {
                writeFacePoints(out, face);
                writeTextureInfo(out, face);
                fmt::format_to(std::back_inserter(out), " 0\n"); // extra value written here
                return 1;
            }
        };

        class ValveFileSerializer : public QuakeFileSerializer {
        public:
            explicit ValveFileSerializer(std::ostream& stream) :
            QuakeFileSerializer(stream) {}
        private:
            size_t doWriteBrushFace(std::vector<char>& out, const Model::BrushFace& face) override {
                writeFacePoints(out, face);
                writeValveTextureInfo(out, face);
                fmt::format_to(std::back_inserter(out), "\n");
                return 1;
            }
        };

        std::unique_ptr<NodeSerializer> MapFileSerializer::create(const Model::MapFormat format, std::ostream& stream) {
            switch (format) {
                case Model::MapFormat::Standard:
                    return std::make_unique<QuakeFileSerializer>(stream);
                case Model::MapFormat::Quake2:
                    // TODO 2427: Implement Quake3 serializers and use them
                case Model::MapFormat::Quake3:
                case Model::MapFormat::Quake3_Legacy:
                    return std::make_unique<Quake2FileSerializer>(stream);
                case Model::MapFormat::Quake2_Valve:
                case Model::MapFormat::Quake3_Valve:
                    return std::make_unique<Quake2ValveFileSerializer>(stream);
                case Model::MapFormat::Daikatana:
                    return std::make_unique<DaikatanaFileSerializer>(stream);
                case Model::MapFormat::Valve:
                    return std::make_unique<ValveFileSerializer>(stream);
                case Model::MapFormat::Hexen2:
                    return std::make_unique<Hexen2FileSerializer>(stream);
                case Model::MapFormat::Unknown:
                    throw FileFormatException("Unknown map file format");
                switchDefault()
            }
        }

        MapFileSerializer::MapFileSerializer(std::ostream& stream) :
        m_line(1),
        m_stream(stream) {}

        MapFileSerializer::~MapFileSerializer() {
            m_stream.write(m_buf.data(), m_buf.size());
        }

        void MapFileSerializer::doBeginFile() {}
        void MapFileSerializer::doEndFile() {}

        void MapFileSerializer::doBeginEntity(const Model::Node* /* node */) {
            fmt::format_to(std::back_inserter(m_buf), "// entity {}\n", entityNo());
            ++m_line;
            m_startLineStack.push_back(m_line);
            fmt::format_to(std::back_inserter(m_buf), "{{\n");
            ++m_line;
        }

        void MapFileSerializer::doEndEntity(const Model::Node* node) {
            fmt::format_to(std::back_inserter(m_buf), "}}\n");
            ++m_line;
            setFilePosition(node);
        }

        void MapFileSerializer::doEntityAttribute(const Model::EntityAttribute& attribute) {
            fmt::format_to(std::back_inserter(m_buf), "\"{}\" \"{}\"\n",
                         escapeEntityAttribute( attribute.name()).c_str(),
                         escapeEntityAttribute(attribute.value()).c_str());
            ++m_line;
        }

        void MapFileSerializer::doBeginBrush(const Model::BrushNode* /* brush */) {
            fmt::format_to(std::back_inserter(m_buf), "// brush {}\n", brushNo());
            ++m_line;
            m_startLineStack.push_back(m_line);
            fmt::format_to(std::back_inserter(m_buf), "{{\n");
            ++m_line;
        }

        void MapFileSerializer::doEndBrush(const Model::BrushNode* brush) {
            fmt::format_to(std::back_inserter(m_buf), "}}\n");
            ++m_line;
            setFilePosition(brush);
        }

        void MapFileSerializer::doBrushFace(const Model::BrushFace& face) {
            const size_t lines = doWriteBrushFace(m_buf, face);
            face.setFilePosition(m_line, lines);
            m_line += lines;
        }

        void MapFileSerializer::setFilePosition(const Model::Node* node) {
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
