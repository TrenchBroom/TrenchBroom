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

#include "PointFile.h"

#include "Controller/CameraEvent.h"
#include "IO/FileManager.h"

#include <fstream>

namespace TrenchBroom {
    namespace Model {
        String PointFile::path(const String& mapFilePath) {
            IO::FileManager fileManager;
            String mapFileBasePath = fileManager.deleteExtension(mapFilePath);
            return fileManager.appendExtension(mapFileBasePath, ".pts");
        }

        void PointFile::load(const String& mapFilePath) {
            static const float threshold = Math::radians(15);
            
            String pointFilePath = path(mapFilePath);
            std::fstream stream(pointFilePath.c_str(), std::ios_base::in);
            assert(stream.is_open());

            String line;
            if (!stream.eof()) {
                std::getline(stream, line);
                m_points.push_back(Vec3f(line));
                Vec3f lastPoint = m_points.back();
                
                if (!stream.eof()) {
                    std::getline(stream, line);
                    Vec3f curPoint(line);
                    Vec3f refDir = curPoint - lastPoint;
                    refDir.normalize();
                    
                    while (!stream.eof()) {
                        lastPoint = curPoint;
                        std::getline(stream, line);
                        curPoint = Vec3f(line);
                        
                        Vec3f dir = curPoint - lastPoint;
                        dir.normalize();
                        if (std::acos(dir.dot(refDir)) > threshold) {
                            m_points.push_back(lastPoint);
                            refDir = dir;
                        }
                    }
                    
                    m_points.push_back(curPoint);
                }
            }
        }
        
        PointFile::PointFile(const String& mapFilePath) :
        m_current(0) {
            assert(exists(mapFilePath));
            load(mapFilePath);
        }
        
        bool PointFile::exists(const String& mapFilePath) {
            IO::FileManager fileManager;
            return fileManager.exists(path(mapFilePath));
        }
    }
}
