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

#ifndef __TrenchBroom__PakFS__
#define __TrenchBroom__PakFS__

#include "StringUtils.h"
#include "IO/FileSystem.h"
#include "IO/GameFS.h"

#include <map>

namespace TrenchBroom {
    namespace IO {
        class PakFS : public GameFS {
        private:
            typedef std::map<Path, MappedFile::Ptr> PakDirectory;
            MappedFile::Ptr m_file;
            PakDirectory m_directory;
        public:
            PakFS(const Path& path);
        private:
            const MappedFile::Ptr doFindFile(const Path& path) const;
            void readDirectory();
        };
    }
}

#endif /* defined(__TrenchBroom__PakFS__) */
