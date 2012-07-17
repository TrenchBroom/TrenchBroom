/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_EntityRendererManager_h
#define TrenchBroom_EntityRendererManager_h

#include <map>
#include <string>
#include <vector>
#include <set>
#include "Model/Map/EntityDefinition.h"

namespace TrenchBroom {
    namespace Model {
        namespace Assets {
            class Palette;
        }
        
        class EntityDefinition;
        class Entity;
        class ModelProperty;
    }
    
    namespace Renderer {
        class Vbo;
        class EntityRenderer;
        
        class EntityRendererManager {
        private:
            typedef std::map<std::string, EntityRenderer*> EntityRendererCache;
            typedef std::set<std::string> MismatchCache;
            
            Vbo* m_vbo;
            Model::Assets::Palette& m_palette;
            EntityRendererCache m_entityRenderers;
            MismatchCache m_mismatches;
            std::string m_quakePath;

            const std::string entityRendererKey(Model::ModelPropertyPtr modelProperty, const std::vector<std::string>& searchPaths);
            EntityRenderer* entityRenderer(Model::ModelPropertyPtr modelProperty, const std::vector<std::string>& mods);
        public:
            EntityRendererManager(const std::string& quakePath, Model::Assets::Palette& palette);
            ~EntityRendererManager();
            
            EntityRenderer* entityRenderer(const Model::EntityDefinition& entityDefinition, const std::vector<std::string>& mods);
            EntityRenderer* entityRenderer(const Model::Entity& entity, const std::vector<std::string>& mods);
            void clear();
            void setQuakePath(const std::string& quakePath);
            
            void activate();
            void deactivate();
        };
    }
}

#endif
