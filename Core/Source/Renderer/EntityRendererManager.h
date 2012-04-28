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
#include "Model/Map/EntityDefinition.h"

using namespace std;

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
            typedef map<string, EntityRenderer*> EntityRendererCache;
            
            Vbo* m_vbo;
            Model::Assets::Palette& m_palette;
            EntityRendererCache m_entityRenderers;
            string m_quakePath;

            const string entityRendererKey(Model::ModelPropertyPtr modelProperty, const vector<string>& searchPaths);
            EntityRenderer* entityRenderer(Model::ModelPropertyPtr modelProperty, const vector<string>& mods);
        public:
            EntityRendererManager(const string& quakePath, Model::Assets::Palette& palette);
            ~EntityRendererManager();
            
            EntityRenderer* entityRenderer(const Model::EntityDefinition& entityDefinition, const vector<string>& mods);
            EntityRenderer* entityRenderer(const Model::Entity& entity, const vector<string>& mods);
            void clear();
            void setQuakePath(const string& quakePath);
            
            void activate();
            void deactivate();
        };
    }
}

#endif
