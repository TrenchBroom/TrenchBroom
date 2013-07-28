/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "EntityModelManager.h"

#include "CollectionUtils.h"
#include "Assets/EntityModel.h"
#include "Model/Game.h"

namespace TrenchBroom {
    namespace Assets {
        EntityModelManager::~EntityModelManager() {
            clear();
        }
        
        void EntityModelManager::clear() {
            MapUtils::clearAndDelete(m_models);
            m_mismatches.clear();
        }

        void EntityModelManager::reset(Model::GamePtr game) {
            if (m_game == game)
                return;
            clear();
            m_game = game;
        }
        
        EntityModel* EntityModelManager::model(const IO::Path& path) const {
            Cache::const_iterator it = m_models.find(path);
            if (it != m_models.end())
                return it->second;

            if (m_mismatches.count(path) > 0)
                return NULL;
            
            EntityModel* model = m_game->loadModel(path);
            if (model == NULL)
                m_mismatches.insert(path);
            else
                m_models[path] = model;
            return model;
        }
    }
}
