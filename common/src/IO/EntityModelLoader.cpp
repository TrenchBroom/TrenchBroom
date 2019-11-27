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

#include "EntityModelLoader.h"

#include "Assets/EntityModel.h"

namespace TrenchBroom {
    namespace IO {
        EntityModelLoader::~EntityModelLoader() = default;

        std::unique_ptr<Assets::EntityModel> EntityModelLoader::initializeModel(const IO::Path& path, Logger& logger) const {
            return doInitializeModel(path, logger);
        }

        void EntityModelLoader::loadFrame(const IO::Path& path, const size_t frameIndex, Assets::EntityModel& model, Logger& logger) const {
            return doLoadFrame(path, frameIndex, model, logger);
        }
    }
}
