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

#include "MapStreamSerializer.h"
#include "StringUtils.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace IO {
        class QuakeStreamSerializer : public MapStreamSerializer {
        public:
            QuakeStreamSerializer(std::ostream& stream) :
            MapStreamSerializer(stream) {}
        private:
            virtual void doWriteBrushFace(std::ostream& stream, Model::BrushFace* face) override {
                writeFacePoints(stream, face);
                stream << " ";
                writeTextureInfo(stream, face);
                stream << "\n";
            }
        protected:
            void writeFacePoints(std::ostream& stream, Model::BrushFace* face) {
                const Model::BrushFace::Points& points = face->points();

                stream.precision(FloatPrecision);
                stream << "( " <<
                StringUtils::ftos(points[0].x(), FloatPrecision) << " " <<
                StringUtils::ftos(points[0].y(), FloatPrecision) << " " <<
                StringUtils::ftos(points[0].z(), FloatPrecision) <<" ) ( " <<
                StringUtils::ftos(points[1].x(), FloatPrecision) << " " <<
                StringUtils::ftos(points[1].y(), FloatPrecision) << " " <<
                StringUtils::ftos(points[1].z(), FloatPrecision) << " ) ( " <<
                StringUtils::ftos(points[2].x(), FloatPrecision) << " " <<
                StringUtils::ftos(points[2].y(), FloatPrecision) << " " <<
                StringUtils::ftos(points[2].z(), FloatPrecision) << " )";
            }

            void writeTextureInfo(std::ostream& stream, Model::BrushFace* face) {
                const String& textureName = face->textureName().empty() ? Model::BrushFace::NoTextureName : face->textureName();
                stream << textureName << " " <<
                StringUtils::ftos(face->xOffset(), FloatPrecision)  << " " <<
                StringUtils::ftos(face->yOffset(), FloatPrecision)  << " " <<
                StringUtils::ftos(face->rotation(), FloatPrecision) << " " <<
                StringUtils::ftos(face->xScale(), FloatPrecision)   << " " <<
                StringUtils::ftos(face->yScale(), FloatPrecision);
            }
        };

        class Quake2StreamSerializer : public QuakeStreamSerializer {
        public:
            Quake2StreamSerializer(std::ostream& stream) :
            QuakeStreamSerializer(stream) {}
        private:
            virtual void doWriteBrushFace(std::ostream& stream, Model::BrushFace* face) override {
                writeFacePoints(stream, face);
                stream << " ";
                writeTextureInfo(stream, face);
                if (face->hasSurfaceAttributes()) {
                    stream << " ";
                    writeSurfaceAttributes(stream, face);

                }
                stream << "\n";
            }
        protected:
            void writeSurfaceAttributes(std::ostream& stream, Model::BrushFace* face) {
                stream <<
                face->surfaceContents()  << " " <<
                face->surfaceFlags()     << " " <<
                StringUtils::ftos(face->surfaceValue(), FloatPrecision);
            }
        };

        class DaikatanaStreamSerializer : public Quake2StreamSerializer {
        public:
            DaikatanaStreamSerializer(std::ostream& stream) :
            Quake2StreamSerializer(stream) {}
        private:
            virtual void doWriteBrushFace(std::ostream& stream, Model::BrushFace* face) override {
                writeFacePoints(stream, face);
                stream << " ";
                writeTextureInfo(stream, face);
                if (face->hasSurfaceAttributes() || face->hasColor()) {
                    stream << " ";
                    writeSurfaceAttributes(stream, face);

                }
                if (face->hasColor()) {
                    stream << " ";
                    writeSurfaceColor(stream, face);
                }
                stream << "\n";
            }
        protected:
            void writeSurfaceColor(std::ostream& stream, Model::BrushFace* face) {
                stream <<
                static_cast<int>(face->color().r() * 255.0f) << " " <<
                static_cast<int>(face->color().g() * 255.0f) << " " <<
                static_cast<int>(face->color().b() * 255.0f);
            }
        };

        class ValveStreamSerializer : public QuakeStreamSerializer {
        public:
            ValveStreamSerializer(std::ostream& stream) :
            QuakeStreamSerializer(stream) {}
        private:
            void doWriteBrushFace(std::ostream& stream, Model::BrushFace* face) override {

                writeFacePoints(stream, face);
                stream << " ";
                writeValveTextureInfo(stream, face);
                stream << "\n";
            }
        private:
            void writeValveTextureInfo(std::ostream& stream, Model::BrushFace* face) {
                const String& textureName = face->textureName().empty() ? Model::BrushFace::NoTextureName : face->textureName();
                const Vec3& xAxis = face->textureXAxis();
                const Vec3& yAxis = face->textureYAxis();

                stream.precision(6);
                stream <<
                textureName     << " " <<
                "[ " <<
                xAxis.x() << " " <<
                xAxis.y() << " " <<
                xAxis.z() << " " <<
                face->xOffset()   <<
                " ] [ " <<
                yAxis.x() << " " <<
                yAxis.y() << " " <<
                yAxis.z() << " " <<
                face->yOffset()   <<
                " ] " <<
                face->rotation() << " " <<
                face->xScale()   << " " <<
                face->yScale();
            }
        };
        
        class Hexen2StreamSerializer : public QuakeStreamSerializer {
        public:
            Hexen2StreamSerializer(std::ostream& stream) :
            QuakeStreamSerializer(stream) {}
        private:
            virtual void doWriteBrushFace(std::ostream& stream, Model::BrushFace* face) override {
                writeFacePoints(stream, face);
                stream << " ";
                writeTextureInfo(stream, face);
                stream << " 0\n"; // extra value written here
            }
        };
        
        NodeSerializer::Ptr MapStreamSerializer::create(const Model::MapFormat::Type format, std::ostream& stream) {
            switch (format) {
                case Model::MapFormat::Standard:
                    return NodeSerializer::Ptr(new QuakeStreamSerializer(stream));
                case Model::MapFormat::Quake2:
                    return NodeSerializer::Ptr(new Quake2StreamSerializer(stream));
                case Model::MapFormat::Daikatana:
                    return NodeSerializer::Ptr(new DaikatanaStreamSerializer(stream));
                case Model::MapFormat::Valve:
                    return NodeSerializer::Ptr(new ValveStreamSerializer(stream));
                case Model::MapFormat::Hexen2:
                    return NodeSerializer::Ptr(new Hexen2StreamSerializer(stream));
                case Model::MapFormat::Unknown:
                default:
                    throw FileFormatException("Unknown map file format");
            }
        }

        MapStreamSerializer::MapStreamSerializer(std::ostream& stream) :
        m_stream(stream) {}

        MapStreamSerializer::~MapStreamSerializer() {}
        
        void MapStreamSerializer::doBeginFile() {}
        void MapStreamSerializer::doEndFile() {}

        void MapStreamSerializer::doBeginEntity(const Model::Node* node) {
            m_stream << "// entity " << entityNo() << "\n";
            m_stream << "{\n";
        }
        
        void MapStreamSerializer::doEndEntity(Model::Node* node) {
            m_stream << "}\n";
        }
        
        void MapStreamSerializer::doEntityAttribute(const Model::EntityAttribute& attribute) {
            m_stream << "\"" << escapeEntityAttribute(attribute.name()) << "\" \"" << escapeEntityAttribute(attribute.value()) << "\"\n";
        }
        
        void MapStreamSerializer::doBeginBrush(const Model::Brush* brush) {
            m_stream << "// brush " << brushNo() << "\n";
            m_stream << "{\n";
        }
        
        void MapStreamSerializer::doEndBrush(Model::Brush* brush) {
            m_stream << "}\n";
        }
        
        void MapStreamSerializer::doBrushFace(Model::BrushFace* face) {
            doWriteBrushFace(m_stream, face);
        }
    }
}
