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

#include "Ensure.h"
#include "Exceptions.h"
#include "IO/File.h"
#include "IO/Reader.h"
#include "IO/FileSystem.h"
#include "IO/ImageLoader.h"

#include <kdl/string_format.h>

namespace TrenchBroom {
    namespace Assets {
        Palette::Data::Data(std::vector<unsigned char>&& data) :
        m_data(std::move(data)) {
            ensure(!m_data.empty(), "palette is empty");
        }

        Palette::Palette() {}

        Palette::Palette(std::vector<unsigned char> data) :
        m_data(std::make_shared<Data>(std::move(data))) {}

        Palette Palette::loadFile(const IO::FileSystem& fs, const IO::Path& path) {
            try {
                auto file = fs.openFile(path);
                auto reader = file->reader().buffer();
                const auto extension = kdl::str_to_lower(path.extension());
                if (extension == "lmp") {
                    return loadLmp(reader);
                } else if (extension == "pcx") {
                    return loadPcx(reader);
                } else if (extension == "bmp") {
                    return loadBmp(reader);
                } else {
                    throw AssetException("Could not load palette file '" + path.asString() + "': Unknown palette format");
                }
            } catch (const FileSystemException& e) {
                throw AssetException("Could not load palette file '" + path.asString() + "': " + e.what());
            }
        }

        Palette Palette::loadLmp(IO::Reader& reader) {
            auto data = std::vector<unsigned char>(reader.size());
            reader.read(data.data(), data.size());
            return Palette(std::move(data));
        }

        Palette Palette::loadPcx(IO::Reader& reader) {
            auto data = std::vector<unsigned char>(768);
            reader.seekFromEnd(data.size());
            reader.read(data.data(), data.size());
            return Palette(std::move(data));
        }

        Palette Palette::loadBmp(IO::Reader& reader) {
            auto bufferedReader = reader.buffer();
            IO::ImageLoader imageLoader(IO::ImageLoader::BMP, std::begin(bufferedReader), std::end(bufferedReader));
            auto data = imageLoader.hasPalette() ? imageLoader.loadPalette() : imageLoader.loadPixels(IO::ImageLoader::RGB);
            return Palette(std::move(data));
        }

        Palette Palette::fromRaw(IO::Reader& reader) {
            auto data = std::vector<unsigned char>(reader.size());
            reader.read(data.data(), data.size());
            return Palette(std::move(data));
        }

        bool Palette::initialized() const {
            return m_data.get() != nullptr;
        }
    }
}
