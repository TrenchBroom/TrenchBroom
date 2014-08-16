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

#include "RotateTexturesCommand.h"

#include "Model/BrushFace.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType RotateTexturesCommand::Type = Command::freeType();
        
        RotateTexturesCommand::Ptr RotateTexturesCommand::rotateTextures(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const float angle) {
            return Ptr(new RotateTexturesCommand(document, faces, angle));
        }
        
        RotateTexturesCommand::RotateTexturesCommand(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const float angle) :
        DocumentCommand(Type, makeName(faces), true, document),
        m_faces(faces),
        m_angle(angle) {}
        
        bool RotateTexturesCommand::doPerformDo() {
            rotateTextures(m_angle);
            return true;
        }
        
        bool RotateTexturesCommand::doPerformUndo() {
            rotateTextures(-m_angle);
            return true;
        }
        
        bool RotateTexturesCommand::doIsRepeatable(View::MapDocumentSPtr document) const {
            return document->hasSelectedFaces();
        }
        
        Command* RotateTexturesCommand::doRepeat(View::MapDocumentSPtr document) const {
            return new RotateTexturesCommand(document, document->selectedFaces(), m_angle);
        }

        bool RotateTexturesCommand::doCollateWith(Command::Ptr command) {
            Ptr other = Command::cast<RotateTexturesCommand>(command);
            m_angle += other->m_angle;
            return true;
        }

        void RotateTexturesCommand::rotateTextures(const float angle) {
            View::MapDocumentSPtr document = lockDocument();
            Model::BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                document->faceWillChangeNotifier(face);
                face->rotateTexture(angle);
                document->faceDidChangeNotifier(face);
            }
        }
        
        String RotateTexturesCommand::makeName(const Model::BrushFaceList& faces) {
            return faces.size() == 1 ? "Rotate Texture" : "Rotate Textures";
        }
    }
}
