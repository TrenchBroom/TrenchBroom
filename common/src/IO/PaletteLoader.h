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

#ifndef PaletteLoader_h
#define PaletteLoader_h

#include "Assets/Palette.h"
#include "IO/MappedFile.h"

#include <memory>

namespace TrenchBroom {
    namespace IO {
        class FileSystem;
        
        class PaletteLoader {
        public:
            typedef std::auto_ptr<PaletteLoader> Ptr;
        public:
            virtual ~PaletteLoader();
            
            Assets::Palette::Ptr loadPalette(MappedFile::Ptr textureFile) const;
        protected:
            Assets::Palette::Ptr loadPalette(const FileSystem& fs, const IO::Path& path) const;
            Assets::Palette::Ptr loadLmpPalette(MappedFile::Ptr file) const;
            Assets::Palette::Ptr loadPcxPalette(MappedFile::Ptr file) const;
        private:
            virtual Assets::Palette::Ptr doLoadPalette(MappedFile::Ptr textureFile) const = 0;
        };
        
        class FilePaletteLoader : public PaletteLoader {
        private:
            Assets::Palette::Ptr m_palette;
        public:
            FilePaletteLoader(const FileSystem& fs, const IO::Path& path);
        private:
            Assets::Palette::Ptr doLoadPalette(MappedFile::Ptr textureFile) const;
        };
        
        class DkWalPaletteLoader : public PaletteLoader {
        private:
            Assets::Palette::Ptr doLoadPalette(MappedFile::Ptr textureFile) const;
        };
    }
}

#endif /* PaletteLoader_h */
