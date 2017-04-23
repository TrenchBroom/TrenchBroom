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

#include "Palette.h"

#include "Exceptions.h"
#include "StringUtils.h"
#include "IO/CharArrayReader.h"
#include "IO/FileSystem.h"

#include <algorithm>
#include <cstring>
#include <fstream>

namespace TrenchBroom {
    namespace Assets {
        Palette::Data::Data(const size_t size, unsigned char* data) :
        m_size(size),
        m_data(data) {
            ensure(m_size > 0, "size is 0");
            ensure(m_data != NULL, "data is null");
        }
        
        Palette::Data::~Data() {
            delete [] m_data;
        }

        Palette::Palette(const size_t size, unsigned char* data) :
        m_data(new Data(size, data)) {}

        Palette Palette::loadFile(const IO::FileSystem& fs, const IO::Path& path) {
            try {
                IO::MappedFile::Ptr file = fs.openFile(path);
                const String extension = StringUtils::toLower(path.extension());
                if (extension == "lmp")
                    return loadLmp(file);
                else if (extension == "pcx")
                    return loadPcx(file);
                else
                    throw new AssetException("Could not load palette file '" + path.asString() + "': Unknown palette format");
            } catch (const FileSystemException& e) {
                throw AssetException("Could not load palette file '" + path.asString() + "': " + e.what());
            }
        }
        
        Palette Palette::loadLmp(IO::MappedFile::Ptr file) {
            const size_t size = file->size();
            unsigned char* data = new unsigned char[size];
            
            IO::CharArrayReader reader(file->begin(), file->end());
            reader.read(data, size);
            
            return Palette(size, data);
        }
        
        Palette Palette::loadPcx(IO::MappedFile::Ptr file) {
            const size_t size = 768;
            unsigned char* data = new unsigned char[size];
            
            IO::CharArrayReader reader(file->begin(), file->end());
            reader.seekFromEnd(size);
            reader.read(data, size);
            
            return Palette(size, data);
        }
    }
}
