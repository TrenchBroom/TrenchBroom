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
        class StandardStreamSerializer : public MapStreamSerializer {
        private:
            bool m_longFormat;
        public:
            StandardStreamSerializer(std::ostream& stream, const bool longFormat) :
            MapStreamSerializer(stream),
            m_longFormat(longFormat) {}
        private:
            void doWriteBrushFace(std::ostream& stream, Model::BrushFace* face) override {
                const String& textureName = face->textureName().empty() ? Model::BrushFace::NoTextureName : face->textureName();
                const Model::BrushFace::Points& points = face->points();
                
                stream <<
                "( " <<
                StringUtils::ftos(points[0].x(), FloatPrecision) << " " <<
                StringUtils::ftos(points[0].y(), FloatPrecision) << " " <<
                StringUtils::ftos(points[0].z(), FloatPrecision) <<" ) ( " <<
                StringUtils::ftos(points[1].x(), FloatPrecision) << " " <<
                StringUtils::ftos(points[1].y(), FloatPrecision) << " " <<
                StringUtils::ftos(points[1].z(), FloatPrecision) << " ) ( " <<
                StringUtils::ftos(points[2].x(), FloatPrecision) << " " <<
                StringUtils::ftos(points[2].y(), FloatPrecision) << " " <<
                StringUtils::ftos(points[2].z(), FloatPrecision) << " ) ";
                
                stream <<
                textureName << " " <<
                StringUtils::ftos(face->xOffset(), FloatPrecision)  << " " <<
                StringUtils::ftos(face->yOffset(), FloatPrecision)  << " " <<
                StringUtils::ftos(face->rotation(), FloatPrecision) << " " <<
                StringUtils::ftos(face->xScale(), FloatPrecision)   << " " <<
                StringUtils::ftos(face->yScale(), FloatPrecision);
                
                if (m_longFormat) {
                    stream << " " <<
                    face->surfaceContents()  << " " <<
                    face->surfaceFlags()     << " " <<
                    StringUtils::ftos(face->surfaceValue(), FloatPrecision);
                }
                
                stream << "\n";
            }
        };
        
        class ValveStreamSerializer : public MapStreamSerializer {
        public:
            ValveStreamSerializer(std::ostream& stream) :
            MapStreamSerializer(stream) {}
        private:
            void doWriteBrushFace(std::ostream& stream, Model::BrushFace* face) override {
                const String& textureName = face->textureName().empty() ? Model::BrushFace::NoTextureName : face->textureName();
                const vec3& xAxis = face->textureXAxis();
                const vec3& yAxis = face->textureYAxis();
                const Model::BrushFace::Points& points = face->points();
                
                stream.precision(FloatPrecision);
                stream <<
                "( " <<
                points[0].x() << " " <<
                points[0].y() << " " <<
                points[0].z() <<
                " ) ( "           <<
                points[1].x() << " " <<
                points[1].y() << " " <<
                points[1].z() <<
                " ) ( "           <<
                points[2].x() << " " <<
                points[2].y() << " " <<
                points[2].z() <<
                " ) ";
                
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
                face->yScale()   << "\n";
            }
        };
        
        class Hexen2StreamSerializer : public MapStreamSerializer {
        public:
            Hexen2StreamSerializer(std::ostream& stream) :
            MapStreamSerializer(stream) {}
        private:
            void doWriteBrushFace(std::ostream& stream, Model::BrushFace* face) override {
                const String& textureName = face->textureName().empty() ? Model::BrushFace::NoTextureName : face->textureName();
                const Model::BrushFace::Points& points = face->points();
                
                stream.precision(FloatPrecision);
                stream <<
                "( " <<
                points[0].x() << " " <<
                points[0].y() << " " <<
                points[0].z() <<
                " ) ( "           <<
                points[1].x() << " " <<
                points[1].y() << " " <<
                points[1].z() <<
                " ) ( "           <<
                points[2].x() << " " <<
                points[2].y() << " " <<
                points[2].z() <<
                " ) ";
                
                stream.precision(6);
                stream <<
                textureName             << " " <<
                face->xOffset()          << " " <<
                face->yOffset()          << " " <<
                face->rotation()         << " " <<
                face->xScale()           << " " <<
                face->yScale()           << "\n";
            }
        };
        
        NodeSerializer::Ptr MapStreamSerializer::create(const Model::MapFormat::Type format, std::ostream& stream) {
            switch (format) {
                case Model::MapFormat::Standard:
                    return NodeSerializer::Ptr(new StandardStreamSerializer(stream, false));
                case Model::MapFormat::Quake2:
                    return NodeSerializer::Ptr(new StandardStreamSerializer(stream, true));
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
