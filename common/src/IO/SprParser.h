/*
 Copyright (C) 2021 Kristian Duske

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

#pragma once

#include "IO/EntityModelParser.h"

#include <memory>
#include <string>

namespace TrenchBroom {
    namespace Assets {
        class Palette;
    }

    namespace IO {
        class File;
        class FileSystem;

        class SprParser : public EntityModelParser {
        private:
            std::string m_name;
            const char* m_begin;
            const char* m_end;
            const Assets::Palette& m_palette;
        public:
            SprParser(std::string name, const char* begin, const char* end, const Assets::Palette& palette);
        private:
            std::unique_ptr<Assets::EntityModel> doInitializeModel(Logger& logger) override;
            void doLoadFrame(size_t frameIndex, Assets::EntityModel& model, Logger& logger) override;
        };
    }
}
