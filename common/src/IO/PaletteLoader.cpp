/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "PaletteLoader.h"

#include "IO/CharArrayReader.h"
#include "IO/FileSystem.h"

namespace TrenchBroom {
    namespace IO {
        PaletteLoader::~PaletteLoader() {}
        
        Assets::Palette::Ptr PaletteLoader::loadPalette(MappedFile::Ptr textureFile) const {
            return doLoadPalette(textureFile);
        }

        Assets::Palette::Ptr PaletteLoader::loadPalette(const FileSystem& fs, const IO::Path& path) const {
            try {
                MappedFile::Ptr file = fs.openFile(path);
                const String extension = StringUtils::toLower(path.extension());
                if (extension == "lmp")
                    return loadLmpPalette(file);
                else if (extension == "pcx")
                    return loadPcxPalette(file);
                else
                    throw new AssetException("Could not load palette file '" + path.asString() + "': Unknown palette format");
            } catch (const FileSystemException& e) {
                throw AssetException("Could not load palette file '" + path.asString() + "': " + e.what());
            }
        }

        Assets::Palette::Ptr PaletteLoader::loadLmpPalette(MappedFile::Ptr file) const {
            const size_t size = file->size();
            unsigned char* data = new unsigned char[size];

            CharArrayReader reader(file->begin(), file->end());
            reader.read(data, size);
            
            return Assets::Palette::Ptr(new Assets::Palette(data, size));
        }
        
        Assets::Palette::Ptr PaletteLoader::loadPcxPalette(MappedFile::Ptr file) const {
            const size_t size = 768;
            unsigned char* data = new unsigned char[size];

            CharArrayReader reader(file->begin(), file->end());
            reader.seekFromEnd(size);
            reader.read(data, size);
            
            return Assets::Palette::Ptr(new Assets::Palette(data, size));
        }

        FilePaletteLoader::FilePaletteLoader(const FileSystem& fs, const IO::Path& path) :
        m_palette(loadPalette(fs, path)) {}
        
        Assets::Palette::Ptr FilePaletteLoader::doLoadPalette(MappedFile::Ptr textureFile) const {
            return m_palette;
        }
        
        Assets::Palette::Ptr DkWalPaletteLoader::doLoadPalette(MappedFile::Ptr textureFile) const {
            static const size_t PaletteOffset = 120;
            
            const size_t size = 768;
            unsigned char* data = new unsigned char[size];

            CharArrayReader reader(textureFile->begin(), textureFile->end());
            reader.seekFromBegin(PaletteOffset);
            
            reader.read(data, size);
            return Assets::Palette::Ptr(new Assets::Palette(data, size));
        }

    }
}
