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

#ifndef __TrenchBroom__FixPlanePointsCommand__
#define __TrenchBroom__FixPlanePointsCommand__

#include "StringUtils.h"
#include "SharedPointer.h"
#include "Controller/Command.h"
#include "Model/Snapshot.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class FixPlanePointsCommand : public Command {
        public:
            static const CommandType Type;
            typedef TrenchBroom::shared_ptr<FixPlanePointsCommand> Ptr;
        private:
            typedef enum {
                Action_SnapPoints,
                Action_FindPoints
            } Action;
            
            View::MapDocumentWPtr m_document;
            Action m_action;
            Model::BrushList m_brushes;
            Model::Snapshot m_snapshot;
        public:
            static Ptr snapPlanePoints(View::MapDocumentWPtr document, const Model::BrushList& brushes);
            static Ptr findPlanePoints(View::MapDocumentWPtr document, const Model::BrushList& brushes);
        private:
            FixPlanePointsCommand(View::MapDocumentWPtr document, Action action, const Model::BrushList& brushes);
            static String makeName(Action action, const Model::BrushList& brushes);
            
            bool doPerformDo();
            bool doPerformUndo();
            bool doCollateWith(Command::Ptr command);
        };
    }
}

#endif /* defined(__TrenchBroom__FixPlanePointsCommand__) */
