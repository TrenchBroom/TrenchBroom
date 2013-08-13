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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__GameFS__
#define __TrenchBroom__GameFS__

#include "IO/FileSystem.h"
#include "IO/Path.h"

namespace TrenchBroom {
    namespace IO {
        class GameFS {
        public:
            virtual ~GameFS();
            
            const MappedFile::Ptr findFile(const Path& path) const;
            String location() const;
        private:
            virtual const MappedFile::Ptr doFindFile(const Path& path) const = 0;
            virtual String doGetLocation() const = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__GameFS__) */
