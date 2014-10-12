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

#include "MapWriter.h"

#include "Exceptions.h"
#include "IO/DiskFileSystem.h"
#include "IO/Path.h"
#include "Model/Layer.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace IO {
        void MapWriter::writeToFileAtPath(Model::World* map, const Path& path, const bool overwrite) {
            if (IO::Disk::fileExists(IO::Disk::fixPath(path)) && !overwrite)
                throw FileSystemException("File already exists: " + path.asString());
            
            // ensure that the directory actually exists or is created if it doesn't
            const Path directoryPath = path.deleteLastComponent();
            WritableDiskFileSystem fs(directoryPath, true);
            
            FILE* stream = fopen(path.asString().c_str(), "w");
            if (stream == NULL)
                throw FileSystemException("Cannot open file: " + path.asString());
            
            try {
                size_t lineNumber = writeDefaultLayer(map, map->defaultLayer(), 1, stream);
                
                
                
                fclose(stream);
            } catch (...) {
                fclose(stream);
                throw;
            }
        }
    }
}
