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

#include "QuakeFS.h"
#include "IO/DiskFS.h"
#include "IO/FileSystem.h"
#include "IO/PakFS.h"
#include "IO/Path.h"

#include <algorithm>

namespace TrenchBroom {
    namespace IO {
        QuakeFS::QuakeFS(const Path& quakePath, const Path& mod) {
            FileSystem fs;
            if (!quakePath.isEmpty() && fs.exists(quakePath)) {
                addMod(quakePath, Path("id1"));
                if (!mod.isEmpty())
                    addMod(quakePath, mod);
            }
        }
        
        const MappedFile::Ptr QuakeFS::doFindFile(const Path& path) const {
            return m_fs.findFile(path);
        }

        void QuakeFS::addMod(const Path& quakePath, const Path& mod) {
            FileSystem fs;
            const Path csPath = fs.findCaseSensitivePath(quakePath + mod);
            m_fs.addFileSystem(new DiskFS(csPath));
            
            const Path::List pakFiles = findPakFiles(csPath);
            Path::List::const_iterator it, end;
            for (it = pakFiles.begin(), end = pakFiles.end(); it != end; ++it)
                m_fs.addFileSystem(new PakFS(csPath + *it));
        }

        const Path::List QuakeFS::findPakFiles(const Path& path) {
            Path::List result;
            FileSystem fs;
            if (!fs.isDirectory(path))
                return result;
            Path::List pakFiles = fs.directoryContents(path, FileSystem::FSFiles, "pak");
            std::sort(pakFiles.begin(), pakFiles.end());
            return pakFiles;
        }
    }
}
