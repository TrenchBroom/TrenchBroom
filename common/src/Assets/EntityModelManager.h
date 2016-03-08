/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_EntityModelManager
#define TrenchBroom_EntityModelManager

#include "Assets/ModelDefinition.h"
#include "IO/Path.h"
#include "Model/ModelTypes.h"

#include <map>
#include <set>
#include <vector>

namespace TrenchBroom {
    class Logger;
    
    namespace IO {
        class EntityModelLoader;
    }
    
    namespace Renderer {
        class TexturedIndexRangeRenderer;
        class Vbo;
    }
    
    namespace Assets {
        class EntityModel;
        
        class EntityModelManager {
        private:
            typedef std::map<IO::Path, EntityModel*> ModelCache;
            typedef std::set<IO::Path> ModelMismatches;
            typedef std::vector<EntityModel*> ModelList;
            
            typedef std::map<Assets::ModelSpecification, Renderer::TexturedIndexRangeRenderer*> RendererCache;
            typedef std::set<Assets::ModelSpecification> RendererMismatches;
            typedef std::vector<Renderer::TexturedIndexRangeRenderer*> RendererList;
            
            Logger* m_logger;
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
            EntityModelManager(Logger* logger, int minFilter, int magFilter);
            ~EntityModelManager();
            
            void clear();

            void setTextureMode(int minFilter, int magFilter);
            void setLoader(const IO::EntityModelLoader* loader);
            
            EntityModel* model(const IO::Path& path) const;
            Renderer::TexturedIndexRangeRenderer* renderer(const Assets::ModelSpecification& spec) const;
        private:
            EntityModel* loadModel(const IO::Path& path) const;
        public:
            void prepare(Renderer::Vbo& vbo);
        private:
            void resetTextureMode();
            void prepareModels();
            void prepareRenderers(Renderer::Vbo& vbo);
        };
    }
}

#endif /* defined(TrenchBroom_EntityModelManager) */
