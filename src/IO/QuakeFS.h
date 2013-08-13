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

#ifndef __TrenchBroom__QuakeFS__
#define __TrenchBroom__QuakeFS__

#include "IO/GameFS.h"
#include "IO/MultiFS.h"

namespace TrenchBroom {
    namespace IO {
        class Path;
        
        class QuakeFS : public GameFS {
        private:
            MultiFS m_fs;
        public:
            QuakeFS(const Path& quakePath, const Path& base, const Path& mod = Path(""));
        private:
            const MappedFile::Ptr doFindFile(const Path& path) const;
            String doGetLocation() const;
            void addMod(const Path& quakePath, const Path& mod);
            const Path::List findPakFiles(const Path& path);
        };
    }
}

#endif /* defined(__TrenchBroom__QuakeFS__) */
