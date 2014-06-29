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

#include "MoveTexturesCommand.h"

#include "Model/BrushFace.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType MoveTexturesCommand::Type = Command::freeType();

        MoveTexturesCommand::Ptr MoveTexturesCommand::moveTextures(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const Vec3& up, const Vec3& right, const Vec2f& offset) {
            return Ptr(new MoveTexturesCommand(document, faces, up, right, offset));
        }
        
        MoveTexturesCommand::MoveTexturesCommand(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const Vec3& up, const Vec3& right, const Vec2f& offset) :
        Command(Type, StringUtils::safePlural("Move ", faces.size(), "Texture", "Textures"), true, true),
        m_document(document),
        m_faces(faces),
        m_up(up),
        m_right(right),
        m_offset(offset) {}
        
        bool MoveTexturesCommand::doPerformDo() {
            moveTextures(m_offset);
            return true;
        }
        
        bool MoveTexturesCommand::doPerformUndo() {
            moveTextures(-m_offset);
            return true;
        }

        bool MoveTexturesCommand::doCollateWith(Command::Ptr command) {
            const Ptr other = cast<MoveTexturesCommand>(command);
            if (other->m_up != m_up ||
                other->m_right != m_right)
                return false;
            
            m_offset += other->m_offset;
            return true;
        }

        void MoveTexturesCommand::moveTextures(const Vec2f& offset) {
            View::MapDocumentSPtr document = lock(m_document);
            Model::BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                document->faceWillChangeNotifier(face);
                face->moveTexture(m_up, m_right, offset);
                document->faceDidChangeNotifier(face);
            }
        }
    }
}
