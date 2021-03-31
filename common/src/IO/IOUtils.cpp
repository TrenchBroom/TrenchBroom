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

#include "IOUtils.h"

#include "Ensure.h"
#include "Exceptions.h"
#include "IO/Path.h"
#include "IO/PathQt.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>

#include <iostream>
#include <streambuf>
#include <string>

namespace TrenchBroom {
    namespace IO {
        FILE* openPathAsFILE(const IO::Path& path, const std::string& mode) {
            // Windows: fopen() doesn't handle UTF-8. We have to use the nonstandard _wfopen
            // to open a Unicode path. We will use Qt to help convert the IO::Path to a UTF-16 encoded
            // wchar array.
            //
            // - IO::Path contains UTF-8 (stored in std::string)
            // - pathAsQString() converts UTF-8 to UTF-16 (stored in QString)
            // - QString::toStdWString() returns a UTF-16 std::wstring on Windows
            //
            // All other platforms, just assume fopen() can handle UTF-8
#ifdef _WIN32
            return _wfopen(pathAsQString(path).toStdWString().c_str(),
                           QString::fromStdString(mode).toStdWString().c_str());
#else
            return fopen(path.asString().c_str(), mode.c_str());
#endif
        }

        std::ofstream openPathAsOutputStream(const IO::Path& path, const std::ios::openmode mode) {
#ifdef _WIN32
            return std::ofstream(pathAsQString(path).toStdWString().c_str(), mode);
#else
            return std::ofstream(path.asString().c_str(), mode);
#endif
        }

        std::ifstream openPathAsInputStream(const IO::Path& path, const std::ios::openmode mode) {
#ifdef _WIN32
            return std::ifstream(pathAsQString(path).toStdWString().c_str(), mode);
#else
            return std::ifstream(path.asString().c_str(), mode);
#endif
        }

        OpenFile::OpenFile(const Path& path, const bool write) :
        file(nullptr) {
            file = openPathAsFILE(path, write ? "w" : "r");
            if (file == nullptr) {
                throw FileSystemException("Cannot open file: " + path.asString());
            }
        }

        OpenFile::~OpenFile() {
            if (file != nullptr) {
                fclose(file);
            }
        }

        size_t fileSize(std::FILE* file) {
            ensure(file != nullptr, "file is null");
            const auto pos = std::ftell(file);
            if (pos < 0) {
                throw FileSystemException("ftell failed");
            }

            if (std::fseek(file, 0, SEEK_END) != 0) {
                throw FileSystemException("fseek failed");
            }

            const auto size = std::ftell(file);
            if (size < 0) {
                throw FileSystemException("ftell failed");
            }

            if (std::fseek(file, pos, SEEK_SET) != 0) {
                throw FileSystemException("fseek failed");
            }

            return static_cast<size_t>(size);
        }

        std::string readGameComment(std::istream& stream) {
            return readInfoComment(stream, "Game");
        }

        std::string readFormatComment(std::istream& stream) {
            return readInfoComment(stream, "Format");
        }

        std::string readInfoComment(std::istream& stream, const std::string& name) {
            static const size_t MaxChars = 64;
            const std::string expectedHeader = "// " + name + ": ";
            char buf[MaxChars];

            stream.getline(buf, MaxChars);
            if (stream.fail())
                return "";

            std::string line(buf);
            if (line.size() < expectedHeader.size())
                return "";

            if (line.substr(0, expectedHeader.size()) != expectedHeader)
                return "";

            auto result = line.substr(expectedHeader.size());
            if (result.size() > 0u && result.back() == '\r') {
                result = result.substr(0u, result.size() - 1u);
            }
            return result;
        }

        void writeGameComment(std::ostream& stream, const std::string& gameName, const std::string& mapFormat) {
            stream << "// Game: " << gameName << "\n"
                   << "// Format: " << mapFormat << "\n";
        }
    }
}
