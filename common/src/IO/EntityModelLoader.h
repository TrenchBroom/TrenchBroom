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

#ifndef TrenchBroom_EntityModelLoader
#define TrenchBroom_EntityModelLoader

#include <memory>

namespace TrenchBroom {
    class Logger;

    namespace Assets {
        class EntityModel;
    }

    namespace IO {
        class Path;

        class EntityModelLoader {
        public:
            virtual ~EntityModelLoader();
            std::unique_ptr<Assets::EntityModel> initializeModel(const Path& path, Logger& logger) const;
            void loadFrame(const Path& path, size_t frameIndex, Assets::EntityModel& model, Logger& logger) const;
        private:
            virtual std::unique_ptr<Assets::EntityModel> doInitializeModel(const Path& path, Logger& logger) const = 0;
            virtual void doLoadFrame(const Path& path, size_t frameIndex, Assets::EntityModel& model, Logger& logger) const = 0;
        };
    }
}

#endif /* defined(TrenchBroom_EntityModelLoader) */
