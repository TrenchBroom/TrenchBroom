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

#ifndef TrenchBroom_PortalFile
#define TrenchBroom_PortalFile

#include "TrenchBroom.h"
#include <vecmath/VecMath.h>

namespace TrenchBroom {
    namespace IO {
        class Path;
    }
    
    namespace Model {
        class PortalFile {
        private:
            vm::polygon3f::List m_portals;
        public:
            PortalFile();
            /**
             * Constructor throws an exception if portalFilePath couldn't be read.
             */
            explicit PortalFile(const IO::Path& portalFilePath);
            
            const vm::polygon3f::List& portals() const;
        private:
            void load(const IO::Path& portalFilePath);
        };
    }
}

#endif /* defined(TrenchBroom_PortalFile) */
