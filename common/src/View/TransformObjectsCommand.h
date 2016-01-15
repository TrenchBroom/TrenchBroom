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

#ifndef TrenchBroom_TransformObjectsCommand
#define TrenchBroom_TransformObjectsCommand

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/DocumentCommand.h"

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }
    
    namespace View {
        class MapDocumentCommandFacade;
        
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
            Mat4x4 m_transform;
            bool m_lockTextures;
            
            Model::Snapshot* m_snapshot;
        public:
            static Ptr translate(const Vec3& delta, bool lockTextures);
            static Ptr rotate(const Vec3& center, const Vec3& axis, FloatType angle, bool lockTextures);
            static Ptr flip(const Vec3& center, Math::Axis::Type axis, bool lockTextures);
            ~TransformObjectsCommand();
        private:
            TransformObjectsCommand(Action action, const Mat4x4& transform, bool lockTextures);
            static String makeName(Action action);

            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            
            void takeSnapshot(const Model::NodeList& nodes);
            void deleteSnapshot();
            
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const;
            
            bool doCollateWith(UndoableCommand::Ptr command);
        };
    }
}

#endif /* defined(TrenchBroom_TransformObjectsCommand) */
