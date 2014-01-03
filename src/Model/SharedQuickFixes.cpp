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

#include "SharedQuickFixes.h"

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "View/ControllerFacade.h"

namespace TrenchBroom {
    namespace Model {
        const QuickFixType DeleteObjectQuickFix::Type = QuickFix::freeType();

        const DeleteObjectQuickFix& DeleteObjectQuickFix::instance() {
            static const DeleteObjectQuickFix instance;
            return instance;
        }

        void DeleteObjectQuickFix::apply(Entity* entity, View::ControllerSPtr controller) const {
            assert(entity != NULL);
            controller->deselectObject(*entity);
            controller->removeObject(*entity);
        }
        
        void DeleteObjectQuickFix::apply(Brush* brush, View::ControllerSPtr controller) const {
            assert(brush != NULL);
            controller->deselectObject(*brush);
            controller->removeObject(*brush);
        }
        
        DeleteObjectQuickFix::DeleteObjectQuickFix() :
        QuickFix(Type, "Delete affected object") {}
        
        const QuickFixType SnapPlanePointsToIntegerQuickFix::Type = QuickFix::freeType();

        const SnapPlanePointsToIntegerQuickFix& SnapPlanePointsToIntegerQuickFix::instance() {
            static const SnapPlanePointsToIntegerQuickFix instance;
            return instance;
        }
        
        void SnapPlanePointsToIntegerQuickFix::apply(Brush* brush, View::ControllerSPtr controller) const {
            controller->snapPlanePoints(*brush);
        }
        
        SnapPlanePointsToIntegerQuickFix::SnapPlanePointsToIntegerQuickFix() :
        QuickFix(Type, "Snap plane points to integer") {}
        
        const QuickFixType FindIntegerPlanePointsQuickFix::Type = QuickFix::freeType();

        const FindIntegerPlanePointsQuickFix& FindIntegerPlanePointsQuickFix::instance() {
            static const FindIntegerPlanePointsQuickFix instance;
            return instance;
        }
        
        void FindIntegerPlanePointsQuickFix::apply(Brush* brush, View::ControllerSPtr controller) const {
            controller->findPlanePoints(*brush);
        }
        
        FindIntegerPlanePointsQuickFix::FindIntegerPlanePointsQuickFix() :
        QuickFix(Type, "Find integer plane points") {}
        
        const QuickFixType SnapVerticesToIntegerQuickFix::Type = QuickFix::freeType();

        const SnapVerticesToIntegerQuickFix& SnapVerticesToIntegerQuickFix::instance() {
            static const SnapVerticesToIntegerQuickFix instance;
            return instance;
        }
        
        void SnapVerticesToIntegerQuickFix::apply(Brush* brush, View::ControllerSPtr controller) const {
        }
        
        SnapVerticesToIntegerQuickFix::SnapVerticesToIntegerQuickFix() :
        QuickFix(Type, "Snap vertices to integer") {}
        
        const QuickFixType DeleteEntityPropertyQuickFix::Type = QuickFix::freeType();

        const DeleteEntityPropertyQuickFix& DeleteEntityPropertyQuickFix::instance() {
            static const DeleteEntityPropertyQuickFix instance;
            return instance;
        }
        
        void DeleteEntityPropertyQuickFix::apply(Entity* entity, const PropertyKey& key, View::ControllerSPtr controller) const {
            controller->removeEntityProperty(EntityList(1, entity), key);
        }

        DeleteEntityPropertyQuickFix::DeleteEntityPropertyQuickFix() :
        QuickFix(Type, "Delete affected entity property") {}

        const QuickFixType MoveBrushesToWorldspawnQuickFix::Type = QuickFix::freeType();
        
        const MoveBrushesToWorldspawnQuickFix& MoveBrushesToWorldspawnQuickFix::instance() {
            static const MoveBrushesToWorldspawnQuickFix instance;
            return instance;
        }
        
        void MoveBrushesToWorldspawnQuickFix::apply(const BrushList& brushes, View::ControllerSPtr controller) const {
            controller->moveBrushesToWorldspawn(brushes);
        }
        
        MoveBrushesToWorldspawnQuickFix::MoveBrushesToWorldspawnQuickFix() :
        QuickFix(Type, "Move brushes to worldspawn") {}
    }
}
