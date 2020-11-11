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

#ifndef TrenchBroom_IOUtils_h
#define TrenchBroom_IOUtils_h

#include "Macros.h"

#include <cstdio> // for FILE
#include <iosfwd>
#include <fstream>
#include <string>

namespace TrenchBroom {
    namespace IO {
        class Path;

        FILE* openPathAsFILE(const IO::Path& path, const std::string& mode);
        std::fstream openPathAsFstream(const IO::Path& path, std::ios::openmode mode);

        class OpenFile {
        public:
            FILE* file;
        public:
            OpenFile(const Path& path, bool write);
            ~OpenFile();

            deleteCopyAndMove(OpenFile)
        };

        size_t fileSize(std::FILE* file);

        std::string readGameComment(std::istream& stream);
        std::string readFormatComment(std::istream& stream);
        std::string readInfoComment(std::istream& stream, const std::string& name);

        void writeGameComment(FILE* stream, const std::string& gameName, const std::string& mapFormat);
    }
}

#endif
