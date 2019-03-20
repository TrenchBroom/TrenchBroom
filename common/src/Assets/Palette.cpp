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
#include "IO/File.h"
#include "IO/Reader.h"
#include "IO/FileSystem.h"
#include "IO/ImageLoader.h"

#include <algorithm>
#include <cstring>
#include <fstream>

namespace TrenchBroom {
    namespace Assets {
        Palette::Data::Data(const size_t size, RawDataPtr&& data) :
        m_size(size),
        m_data(std::move(data)) {
            ensure(m_size > 0, "size is 0");
            ensure(m_data.get() != nullptr, "data is null");
        }

        Palette::Data::Data(const size_t size, unsigned char* data) :
        m_size(size),
        m_data(data) {
            ensure(m_size > 0, "size is 0");
            ensure(m_data.get() != nullptr, "data is null");
        }

        Palette::Palette() {}

        Palette::Palette(const size_t size, RawDataPtr&& data) :
        m_data(std::make_shared<Data>(size, std::move(data))) {}

        Palette::Palette(const size_t size, unsigned char* data) :
        m_data(std::make_shared<Data>(size, data)) {}

        Palette Palette::loadFile(const IO::FileSystem& fs, const IO::Path& path) {
            try {
                auto file = fs.openFile(path);
                auto reader = file->reader().buffer();
                const auto extension = StringUtils::toLower(path.extension());
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
            const auto size = reader.size();
            auto data = std::make_unique<unsigned char[]>(size);

            reader.read(data.get(), size);

            return Palette(size, std::move(data));
        }

        Palette Palette::loadPcx(IO::Reader& reader) {
            const auto size = 768;
            auto data = std::make_unique<unsigned char[]>(size);

            reader.seekFromEnd(size);
            reader.read(data.get(), size);

            return Palette(size, std::move(data));
        }

        Palette Palette::loadBmp(IO::Reader& reader) {
            auto bufferedReader = reader.buffer();
            IO::ImageLoader imageLoader(IO::ImageLoader::BMP, std::begin(bufferedReader), std::end(bufferedReader));
            const auto& pixels = imageLoader.hasPalette() ? imageLoader.palette() : imageLoader.pixels(IO::ImageLoader::RGB);

            const auto size = pixels.size();
            auto data = std::make_unique<unsigned char[]>(size);
            std::copy(std::begin(pixels), std::end(pixels), data.get());

            return Palette(size, std::move(data));
        }

        Palette Palette::fromRaw(IO::Reader& reader) {
            const auto size = reader.size();
            auto copy = std::make_unique<unsigned char[]>(size);
            reader.read(copy.get(), size);
            return Palette(size, std::move(copy));
        }

        bool Palette::initialized() const {
            return m_data.get() != nullptr;
        }
    }
}
