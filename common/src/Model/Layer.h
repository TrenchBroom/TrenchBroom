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

#ifndef __TrenchBroom__Layer__
#define __TrenchBroom__Layer__

#include "Notifier.h"
#include "StringUtils.h"
#include "Model/ModelTypes.h"
#include "Model/Object.h"
#include "Model/ObjectParent.h"
#include "Model/ObjectSection.h"

namespace TrenchBroom {
    namespace Model {
        class Layer : public ObjectSection, public ObjectParent {
        public:
            typedef int Attr_Type;
            static const Attr_Type Attr_Name        = 1 << 0;
            static const Attr_Type Attr_Objects     = 1 << 1;
            static const Attr_Type Attr_Visible     = 1 << 2;
            static const Attr_Type Attr_Locked      = 1 << 3;
            static const Attr_Type Attr_Any         = Attr_Name | Attr_Objects | Attr_Visible | Attr_Locked;
            static const Attr_Type Attr_Editing     = Attr_Visible | Attr_Locked;
        private:
            String m_name;
            bool m_visible;
            bool m_locked;
            ObjectList m_objects;
            EntityList m_entities;
            BrushList m_worldBrushes;
        public:
            Notifier2<Layer*, Attr_Type> layerWillChangeNotifier;
            Notifier2<Layer*, Attr_Type> layerDidChangeNotifier;
            Notifier2<Layer*, Object*> objectWasAddedNotifier;
            Notifier2<Layer*, Object*> objectWasRemovedNotifier;
        public:
            Layer(const String& name);
            
            const String& name() const;
            void setName(const String& name);
            
            bool visible() const;
            void setVisible(bool visible);
            
            bool locked() const;
            void setLocked(bool locked);
        private:
            void entityWillBeAdded(Entity* entity);
            void entityWasAdded(Entity* entity);
            void entityWillBeRemoved(Entity* entity);
            void entityWasRemoved(Entity* entity);
            void brushWillBeAdded(Brush* brush);
            void brushWasAdded(Brush* brush);
            void brushWillBeRemoved(Brush* brush);
            void brushWasRemoved(Brush* brush);
            
            Layer* doGetLayerForChild();
        };
    }
}

#endif /* defined(__TrenchBroom__Layer__) */
