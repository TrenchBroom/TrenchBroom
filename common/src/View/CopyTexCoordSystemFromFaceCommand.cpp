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

#include "CopyTexCoordSystemFromFaceCommand.h"

#include "Model/BrushFace.h"
#include "Model/Snapshot.h"
#include "View/MapDocumentCommandFacade.h"

namespace TrenchBroom {
    namespace View {
        const Command::CommandType CopyTexCoordSystemFromFaceCommand::Type = Command::freeType();

        CopyTexCoordSystemFromFaceCommand::Ptr CopyTexCoordSystemFromFaceCommand::command(const Model::TexCoordSystemSnapshot* coordSystemSanpshot, const Vec3f& sourceFaceNormal) {
            return Ptr(new CopyTexCoordSystemFromFaceCommand(coordSystemSanpshot, sourceFaceNormal));
        }

        CopyTexCoordSystemFromFaceCommand::CopyTexCoordSystemFromFaceCommand(const Model::TexCoordSystemSnapshot* coordSystemSnapshot, const Vec3f& sourceFaceNormal) :
        DocumentCommand(Type, "Copy Texture Alignment"),
        m_snapshot(nullptr),
        m_coordSystemSanpshot(coordSystemSnapshot->clone()),
        m_sourceFaceNormal(sourceFaceNormal) {}

        CopyTexCoordSystemFromFaceCommand::~CopyTexCoordSystemFromFaceCommand() {
            delete m_snapshot;
            m_snapshot = nullptr;
            
            delete m_coordSystemSanpshot;
            m_coordSystemSanpshot = nullptr;
        }

        bool CopyTexCoordSystemFromFaceCommand::doPerformDo(MapDocumentCommandFacade* document) {
            const Model::BrushFaceList faces = document->allSelectedBrushFaces();
            assert(!faces.empty());
            
            assert(m_snapshot == nullptr);
            m_snapshot = new Model::Snapshot(std::begin(faces), std::end(faces));
            
            document->performCopyTexCoordSystemFromFace(m_coordSystemSanpshot, m_sourceFaceNormal);
            return true;
        }
        
        bool CopyTexCoordSystemFromFaceCommand::doPerformUndo(MapDocumentCommandFacade* document) {
            document->restoreSnapshot(m_snapshot);
            delete m_snapshot;
            m_snapshot = NULL;
            return true;
        }
        
        bool CopyTexCoordSystemFromFaceCommand::doIsRepeatable(MapDocumentCommandFacade* document) const {
            return document->hasSelectedBrushFaces();
        }
        
        UndoableCommand::Ptr CopyTexCoordSystemFromFaceCommand::doRepeat(MapDocumentCommandFacade* document) const {
            return UndoableCommand::Ptr(new CopyTexCoordSystemFromFaceCommand(m_coordSystemSanpshot, m_sourceFaceNormal));
        }
        
        bool CopyTexCoordSystemFromFaceCommand::doCollateWith(UndoableCommand::Ptr command) {
            return false;
        }
    }
}
