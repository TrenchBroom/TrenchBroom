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

#ifndef __TrenchBroom__ShearTexturesCommand__
#define __TrenchBroom__ShearTexturesCommand__

#include "SharedPointer.h"
#include "StringUtils.h"
#include "Controller/DocumentCommand.h"
#include "Model/ModelTypes.h"
#include "Model/Snapshot.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class ShearTexturesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<ShearTexturesCommand> Ptr;
        private:
            const Model::BrushFaceList m_faces;
            Model::Snapshot m_snapshot;

            Vec2f m_factors;
        public:
            static Ptr shearTextures(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const Vec2f& factors);
        private:
            ShearTexturesCommand(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const Vec2f& factors);

            bool doPerformDo();
            bool doPerformUndo();
            
            bool doIsRepeatable(View::MapDocumentSPtr document) const;
            Command* doRepeat(View::MapDocumentSPtr document) const;

            bool doCollateWith(Command::Ptr command);
        };
    }
}

#endif /* defined(__TrenchBroom__ShearTexturesCommand__) */
