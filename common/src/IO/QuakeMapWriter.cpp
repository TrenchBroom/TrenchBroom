/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "QuakeMapWriter.h"

#include "StringUtils.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace IO {
        QuakeMapWriter::QuakeMapWriter() {
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
            "%s %.6g %.6g %.6g %.6g %.6g\n";
            
            FaceFormat = str.str();
        }

        size_t QuakeMapWriter::writeFace(Model::BrushFace& face, const size_t lineNumber, FILE* stream) {
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
                         face.yScale());
            face.setFilePosition(lineNumber, 1);
            return 1;
        }
        
        void QuakeMapWriter::writeFace(const Model::BrushFace& face, std::ostream& stream) {
            const String& textureName = face.textureName().empty() ? Model::BrushFace::NoTextureName : face.textureName();
            const Model::BrushFace::Points& points = face.points();
            
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
            StringUtils::ftos(face.xOffset(), FloatPrecision)  << " " <<
            StringUtils::ftos(face.yOffset(), FloatPrecision)  << " " <<
            StringUtils::ftos(face.rotation(), FloatPrecision) << " " <<
            StringUtils::ftos(face.xScale(), FloatPrecision)   << " " <<
            StringUtils::ftos(face.yScale(), FloatPrecision)   << "\n";
        }
    }
}
