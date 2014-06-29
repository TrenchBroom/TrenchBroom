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

#ifndef __TrenchBroom__TransformObjectsCommand__
#define __TrenchBroom__TransformObjectsCommand__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "StringUtils.h"
#include "SharedPointer.h"
#include "Controller/Command.h"
#include "Model/ModelTypes.h"
#include "Model/Snapshot.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class TransformObjectsCommand : public Command {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<TransformObjectsCommand> Ptr;
        private:
            View::MapDocumentWPtr m_document;
            Mat4x4 m_transformation;
            bool m_lockTextures;
            Model::ObjectList m_objects;
            Model::Snapshot m_snapshot;
        public:
            static Ptr transformObjects(View::MapDocumentWPtr document, const Mat4x4& transformation, const bool lockTextures, const String& action, const Model::ObjectList& objects);
        private:
            TransformObjectsCommand(View::MapDocumentWPtr document, const Mat4x4& transformation, const bool lockTextures, const String& action, const Model::ObjectList& objects);
            static String makeName(const String& action, const Model::ObjectList& objects);
            
            bool doPerformDo();
            bool doPerformUndo();
            bool doCollateWith(Command::Ptr command);
        };
    }
}

#endif /* defined(__TrenchBroom__TransformObjectsCommand__) */
