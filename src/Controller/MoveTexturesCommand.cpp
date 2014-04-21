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

        MoveTexturesCommand::Ptr MoveTexturesCommand::moveTextures(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const Vec3& up, const Vec3& right, const Math::Direction direction, const float distance) {
            return Ptr(new MoveTexturesCommand(document, faces, up, right, direction, distance));
        }
        
        MoveTexturesCommand::MoveTexturesCommand(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const Vec3& up, const Vec3& right, const Math::Direction direction, const float distance) :
        Command(Type, makeName(faces, direction), true, true),
        m_document(document),
        m_faces(faces),
        m_up(up),
        m_right(right),
        m_direction(direction),
        m_distance(distance) {}
        
        bool MoveTexturesCommand::doPerformDo() {
            moveTextures(m_distance);
            return true;
        }
        
        bool MoveTexturesCommand::doPerformUndo() {
            moveTextures(-m_distance);
            return true;
        }

        void MoveTexturesCommand::moveTextures(const float distance) {
            View::MapDocumentSPtr document = lock(m_document);
            Model::BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                document->faceWillChangeNotifier(face);
                face->moveTexture(m_up, m_right, m_direction, distance);
                document->faceDidChangeNotifier(face);
            }
        }

        String MoveTexturesCommand::makeName(const Model::BrushFaceList& faces, const Math::Direction direction) {
            StringStream buffer;
            buffer << "Move " << (faces.size() == 1 ? "Texture" : "Textures");
            switch (direction) {
                case Math::Direction_Up:
                    buffer << "Up";
                    break;
                case Math::Direction_Down:
                    buffer << "Down";
                    break;
                case Math::Direction_Left:
                    buffer << "Left";
                    break;
                case Math::Direction_Right:
                    buffer << "Right";
                    break;
                case Math::Direction_Forward:
                case Math::Direction_Backward:
                    break;
            }
            return buffer.str();
        }
    }
}
