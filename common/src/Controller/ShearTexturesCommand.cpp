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

#include "ShearTexturesCommand.h"

#include "Model/BrushFace.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType ShearTexturesCommand::Type = Command::freeType();

        ShearTexturesCommand::Ptr ShearTexturesCommand::shearTextures(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const Vec2f& factors) {
            return Ptr(new ShearTexturesCommand(document, faces, factors));
        }

        ShearTexturesCommand::ShearTexturesCommand(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const Vec2f& factors) :
        DocumentCommand(Type, "Shear texture", true, document),
        m_faces(faces),
        m_factors(factors) {}
        
        bool ShearTexturesCommand::doPerformDo() {
            View::MapDocumentSPtr document = lockDocument();
            m_snapshot = Model::Snapshot(m_faces);
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                document->faceWillChangeNotifier(face);
                face->shearTexture(m_factors);
                document->faceDidChangeNotifier(face);
            }
            return true;
        }
        
        bool ShearTexturesCommand::doPerformUndo() {
            View::MapDocumentSPtr document = lockDocument();
            
            document->faceWillChangeNotifier(m_faces.begin(), m_faces.end());
            m_snapshot.restore(document->worldBounds());
            document->faceDidChangeNotifier(m_faces.begin(), m_faces.end());
            return true;
        }
        
        bool ShearTexturesCommand::doIsRepeatable(View::MapDocumentSPtr document) const {
            return document->hasSelectedFaces();
        }
        
        Command* ShearTexturesCommand::doRepeat(View::MapDocumentSPtr document) const {
            return new ShearTexturesCommand(document, document->selectedFaces(), m_factors);
        }

        bool ShearTexturesCommand::doCollateWith(Command::Ptr command) {
            Ptr other = Command::cast<ShearTexturesCommand>(command);
            m_factors += other->m_factors;
            return true;
        }
    }
}
