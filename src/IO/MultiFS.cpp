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

#include "MultiFS.h"

#include "CollectionUtils.h"

namespace TrenchBroom {
    namespace IO {
        MultiFS::~MultiFS() {
            VectorUtils::clearAndDelete(m_fileSystems);
        }
        
        void MultiFS::addFileSystem(GameFS* fileSystem) {
            m_fileSystems.push_back(fileSystem);
        }

        const MappedFile::Ptr MultiFS::doFindFile(const Path& path) {
            FSList::const_reverse_iterator it, end;
            for (it = m_fileSystems.rbegin(), end = m_fileSystems.rend(); it != end; ++it) {
                GameFS* fs = *it;
                MappedFile::Ptr file = fs->findFile(path);
                if (file != NULL)
                    return file;
            }
            return MappedFile::Ptr();
        }
    }
}
