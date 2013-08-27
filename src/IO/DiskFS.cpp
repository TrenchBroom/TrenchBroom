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

#include "DiskFS.h"

#include "IO/FileSystem.h"

namespace TrenchBroom {
    namespace IO {
        DiskFS::DiskFS(const Path& basePath) :
        m_basePath(basePath) {}
        
        const MappedFile::Ptr DiskFS::doFindFile(const Path& path) const {
            const Path fullPath = m_basePath + path;
            FileSystem fs;
            if (!fs.exists(fullPath))
                return MappedFile::Ptr();
            return fs.mapFile(fullPath, std::ios::in);
        }

        String DiskFS::doGetLocation() const {
            return m_basePath.asString();
        }
    }
}
