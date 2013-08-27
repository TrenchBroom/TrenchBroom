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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__EntityModelManager__
#define __TrenchBroom__EntityModelManager__

#include "IO/Path.h"
#include "Model/ModelTypes.h"

#include <map>
#include <set>

namespace TrenchBroom {
    namespace Assets {
        class EntityModel;
        
        class EntityModelManager {
        private:
            typedef std::map<IO::Path, EntityModel*> Cache;
            typedef std::set<IO::Path> Mismatches;

            Model::GamePtr m_game;
            mutable Cache m_models;
            mutable Mismatches m_mismatches;
        public:
            ~EntityModelManager();
            void clear();
            void reset(Model::GamePtr game);
            EntityModel* model(const IO::Path& path) const;
        };
    }
}

#endif /* defined(__TrenchBroom__EntityModelManager__) */
