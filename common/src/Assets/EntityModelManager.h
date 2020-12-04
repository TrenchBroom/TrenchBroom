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

#pragma once

#include "IO/Path.h"

#include <kdl/vector_set.h>

#include <map>
#include <memory>
#include <vector>

namespace TrenchBroom {
    class Logger;

    namespace IO {
        class EntityModelLoader;
    }

    namespace Model {
        class EntityNode;
    }

    namespace Renderer {
        class TexturedRenderer;
        class VboManager;
    }

    namespace Assets {
        class EntityModel;
        class EntityModelFrame;
        struct ModelSpecification;

        class EntityModelManager {
        private:
            using ModelCache = std::map<IO::Path, std::unique_ptr<EntityModel>>;
            using ModelMismatches = kdl::vector_set<IO::Path>;
            using ModelList = std::vector<EntityModel*>;

            using RendererCache = std::map<ModelSpecification, std::unique_ptr<Renderer::TexturedRenderer>>;
            using RendererMismatches = kdl::vector_set<ModelSpecification>;
            using RendererList = std::vector<Renderer::TexturedRenderer*>;

            Logger& m_logger;
            const IO::EntityModelLoader* m_loader;

            int m_minFilter;
            int m_magFilter;
            bool m_resetTextureMode;

            mutable ModelCache m_models;
            mutable ModelMismatches m_modelMismatches;
            mutable RendererCache m_renderers;
            mutable RendererMismatches m_rendererMismatches;

            mutable ModelList m_unpreparedModels;
            mutable RendererList m_unpreparedRenderers;
        public:
            EntityModelManager(int magFilter, int minFilter, Logger& logger);
            ~EntityModelManager();

            void clear();

            void setTextureMode(int minFilter, int magFilter);
            void setLoader(const IO::EntityModelLoader* loader);
            Renderer::TexturedRenderer* renderer(const ModelSpecification& spec) const;

            const EntityModelFrame* frame(const ModelSpecification& spec) const;
        private:
            EntityModel* model(const IO::Path& path) const;
            EntityModel* safeGetModel(const IO::Path& path) const;
            std::unique_ptr<EntityModel> loadModel(const IO::Path& path) const;
            void loadFrame(const ModelSpecification& spec, EntityModel& model) const;
        public:
            void prepare(Renderer::VboManager& vboManager);
        private:
            void resetTextureMode();
            void prepareModels();
            void prepareRenderers(Renderer::VboManager& vboManager);
        };
    }
}

#endif /* defined(TrenchBroom_EntityModelManager) */
