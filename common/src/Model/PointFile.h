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

#ifndef TrenchBroom_PointFile
#define TrenchBroom_PointFile

#include "TrenchBroom.h"
#include "VecMath.h"

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace Model {
        class PointFile {
        private:
            Vec3f::List m_points;
            size_t m_current;
        public:
            PointFile();
            PointFile(const IO::Path& mapFilePath);
            
            bool empty() const;
            bool hasNextPoint() const;
            bool hasPreviousPoint() const;
            
            const Vec3f::List& points() const;
            const Vec3f& currentPoint() const;
            const Vec3f currentDirection() const;
            void advance();
            void retreat();

            static IO::Path pointFilePath(const IO::Path& mapFilePath);
        private:
            void load(const IO::Path& pointFilePath);
        };
    }
}

#endif /* defined(TrenchBroom_PointFile) */
