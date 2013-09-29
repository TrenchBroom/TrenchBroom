/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "Quake2MapWriter.h"

#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace IO {
        Quake2MapWriter::Quake2MapWriter() {
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
            "%s %.6g %.6g %.6g %.6g %.6g %d %d %.6g\n";
            
            FaceFormat = str.str();
        }
        
        size_t Quake2MapWriter::writeFace(Model::BrushFace& face, const size_t lineNumber, FILE* stream) {
            const String& textureName = face.textureName().empty() ? Model::BrushFace::NoTextureName : face.textureName();
            const Model::BrushFace::Points& points = face.points();
            
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
                         face.xOffset(),
                         face.yOffset(),
                         face.rotation(),
                         face.xScale(),
                         face.yScale(),
                         face.surfaceContents(),
                         face.surfaceFlags(),
                         face.surfaceValue());
            face.setFilePosition(lineNumber, 1);
            return 1;
        }
        
        void Quake2MapWriter::writeFace(const Model::BrushFace& face, std::ostream& stream) {
            const String& textureName = face.textureName().empty() ? Model::BrushFace::NoTextureName : face.textureName();
            const Model::BrushFace::Points& points = face.points();
            
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
            face.xOffset()          << " " <<
            face.yOffset()          << " " <<
            face.rotation()         << " " <<
            face.xScale()           << " " <<
            face.yScale()           << " " <<
            face.surfaceContents()  << " " <<
            face.surfaceFlags()     << " " <<
            face.surfaceValue()     << "\n";
        }
    }
}
