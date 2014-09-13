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

#ifndef __TrenchBroom__ObjectSection__
#define __TrenchBroom__ObjectSection__

#include "Notifier.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class ObjectSection {
        private:
            ObjectList m_objects;
            EntityList m_entities;
            BrushList m_worldBrushes;
        public:
            virtual ~ObjectSection();
            
            const ObjectList& objects() const;
            const EntityList& entities() const;
            BrushList entityBrushes() const;
            const BrushList& worldBrushes() const;

            void addEntity(Entity* entity);
            void addBrush(Brush* brush);
            void removeEntity(Entity* entity);
            void removeBrush(Brush* brush);
        private:
            virtual void entityWillBeAdded(Entity* entity);
            virtual void entityWasAdded(Entity* entity);
            virtual void entityWillBeRemoved(Entity* entity);
            virtual void entityWasRemoved(Entity* entity);
            virtual void brushWillBeAdded(Brush* brush);
            virtual void brushWasAdded(Brush* brush);
            virtual void brushWillBeRemoved(Brush* brush);
            virtual void brushWasRemoved(Brush* brush);
        };
    }
}

#endif /* defined(__TrenchBroom__ObjectSection__) */
