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

#ifndef __TrenchBroom__ModelFilter__
#define __TrenchBroom__ModelFilter__

#include "Notifier.h"
#include "Model/BrushContentType.h"
#include "Model/ModelTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinition;
        class EntityDefinitionGroup;
    }
    
    namespace Model {
        class ModelFilter {
        public:
            typedef enum {
                EntityLinkMode_All,
                EntityLinkMode_Transitive,
                EntityLinkMode_Direct,
                EntityLinkMode_None
            } EntityLinkMode;
        private:
            bool m_showPointEntities;
            bool m_showBrushes;
            BrushContentType::FlagType m_hiddenBrushContentTypes;
            Bitset m_hiddenEntityDefinitions;
            EntityLinkMode m_entityLinkMode;
        public:
            Notifier0 filterDidChangeNotifier;
        public:
            ModelFilter();
            virtual ~ModelFilter();
            
            bool showPointEntities() const;
            void setShowPointEntities(bool showPointEntities);
            
            bool showBrushes() const;
            void setShowBrushes(bool showBrushes);
            
            BrushContentType::FlagType hiddenBrushContentTypes() const;
            void setHiddenBrushContentTypes(BrushContentType::FlagType brushContentTypes);
            
            bool entityDefinitionHidden(const Assets::EntityDefinition* definition) const;
            void setEntityDefinitionHidden(const Assets::EntityDefinition* definition, bool hidden);
            
            EntityLinkMode entityLinkMode() const;
            void setEntityLinkMode(EntityLinkMode entityLinkMode);
            
            virtual bool visible(const Object* object) const;
            virtual bool visible(const BrushFace* face) const;
            virtual bool locked(const Object* object) const;
            virtual bool pickable(const Object* object) const;
            virtual bool pickable(const BrushFace* face) const;
            virtual bool selectable(const Object* object) const;
            virtual bool selectable(const BrushFace* face) const;
        private:
            ModelFilter(const ModelFilter& other);
            ModelFilter& operator=(const ModelFilter& other);
        };
    }
}

#endif /* defined(__TrenchBroom__ModelFilter__) */
