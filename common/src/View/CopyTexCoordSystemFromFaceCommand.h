/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef TrenchBroom_CopyTexCoordSystemFromFaceCommand
#define TrenchBroom_CopyTexCoordSystemFromFaceCommand

#include "SharedPointer.h"
#include "View/DocumentCommand.h"
#include "Model/TexCoordSystem.h"

namespace TrenchBroom {
    namespace Model {
        class Snapshot;
    }
    
    namespace View {
        class CopyTexCoordSystemFromFaceCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::shared_ptr<CopyTexCoordSystemFromFaceCommand> Ptr;
        private:
            
            Model::Snapshot* m_snapshot;
            Model::TexCoordSystemSnapshot* m_coordSystemSanpshot;
            const Vec3f m_sourceFaceNormal;
        public:
            static Ptr command(const Model::TexCoordSystemSnapshot* coordSystemSanpshot, const Vec3f& sourceFaceNormal);
        private:
            CopyTexCoordSystemFromFaceCommand(const Model::TexCoordSystemSnapshot* coordSystemSanpshot, const Vec3f& sourceFaceNormal);
        public:
            ~CopyTexCoordSystemFromFaceCommand();
        private:
            bool doPerformDo(MapDocumentCommandFacade* document);
            bool doPerformUndo(MapDocumentCommandFacade* document);
            
            bool doIsRepeatable(MapDocumentCommandFacade* document) const;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const;
            
            bool doCollateWith(UndoableCommand::Ptr command);
        private:
            CopyTexCoordSystemFromFaceCommand(const CopyTexCoordSystemFromFaceCommand& other);
            CopyTexCoordSystemFromFaceCommand& operator=(const CopyTexCoordSystemFromFaceCommand& other);
        };
    }
}

#endif /* defined(TrenchBroom_CopyTexCoordSystemFromFaceCommand) */
