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
#include "Controller/DocumentCommand.h"
#include "Model/ModelTypes.h"
#include "Model/Snapshot.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class TransformObjectsCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<TransformObjectsCommand> Ptr;
        private:
            typedef enum {
                Action_Translate,
                Action_Rotate,
                Action_Flip
            } Action;
            
            Action m_action;
            Mat4x4 m_transformation;
            bool m_lockTextures;
            Model::ObjectList m_objects;
            Model::Snapshot m_snapshot;
        public:
            static Ptr translateObjects(View::MapDocumentWPtr document, const Vec3& offset, bool lockTextures, const Model::ObjectList& objects);
            
            static Ptr rotateObjects(View::MapDocumentWPtr document, const Vec3& center, const Vec3& axis, FloatType angle, bool lockTextures, const Model::ObjectList& objects);
            
            static Ptr flipObjects(View::MapDocumentWPtr document, const Vec3& center, Math::Axis::Type axis, bool lockTextures, const Model::ObjectList& objects);
        private:
            TransformObjectsCommand(View::MapDocumentWPtr document, Action action, const Mat4x4& transformation, const bool lockTextures, const Model::ObjectList& objects);
            static String makeName(Action action, const Model::ObjectList& objects);
            
            bool doPerformDo();
            bool doPerformUndo();

            bool doIsRepeatable(View::MapDocumentSPtr document) const;
            Command* doRepeat(View::MapDocumentSPtr document) const;

            bool doCollateWith(Command::Ptr command);
        };
    }
}

#endif /* defined(__TrenchBroom__TransformObjectsCommand__) */
