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

#ifndef __TrenchBroom__SharedQuickFixes__
#define __TrenchBroom__SharedQuickFixes__

#include "Model/QuickFix.h"

#include "StringUtils.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Model {
        class DeleteObjectQuickFix : public QuickFix {
        public:
            static const QuickFixType Type;
            static const DeleteObjectQuickFix& instance();
            
            void apply(Entity* entity, View::ControllerSPtr controller) const;
            void apply(Brush* brush, View::ControllerSPtr controller) const;
        private:
            DeleteObjectQuickFix();
        };
        
        class SnapPlanePointsToIntegerQuickFix : public QuickFix {
        public:
            static const QuickFixType Type;
            static const SnapPlanePointsToIntegerQuickFix& instance();
            
            void apply(Brush* brush, View::ControllerSPtr controller) const;
        private:
            SnapPlanePointsToIntegerQuickFix();
        };
        
        class FindIntegerPlanePointsQuickFix : public QuickFix {
        public:
            static const QuickFixType Type;
            static const FindIntegerPlanePointsQuickFix& instance();
            
            void apply(Brush* brush, View::ControllerSPtr controller) const;
        private:
            FindIntegerPlanePointsQuickFix();
        };
        
        class SnapVerticesToIntegerQuickFix : public QuickFix {
        public:
            static const QuickFixType Type;
            static const SnapVerticesToIntegerQuickFix& instance();
            
            void apply(Brush* brush, View::ControllerSPtr controller) const;
        private:
            SnapVerticesToIntegerQuickFix();
        };
        
        class DeleteEntityPropertyQuickFix : public QuickFix {
        public:
            static const QuickFixType Type;
            static const DeleteEntityPropertyQuickFix& instance();
            
            void apply(Entity* entity, const PropertyKey& key, View::ControllerSPtr controller) const;
        private:
            DeleteEntityPropertyQuickFix();
        };
        
        class MoveBrushesToWorldspawnQuickFix : public QuickFix {
        public:
            static const QuickFixType Type;
            static const MoveBrushesToWorldspawnQuickFix& instance();
            
            void apply(const BrushList& brushes, View::ControllerSPtr controller) const;
        private:
            MoveBrushesToWorldspawnQuickFix();
        };
    }
}

#endif /* defined(__TrenchBroom__SharedQuickFixes__) */
