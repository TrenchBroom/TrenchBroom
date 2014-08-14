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

#ifndef __TrenchBroom__ResizeBrushesCommand__
#define __TrenchBroom__ResizeBrushesCommand__

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "Controller/DocumentCommand.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class ResizeBrushesCommand : public DocumentCommand {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<ResizeBrushesCommand> Ptr;
        private:
            Model::BrushFaceList m_faces;
            Model::BrushList m_brushes;
            Vec3 m_delta;
            bool m_lockTextures;
        public:
            static ResizeBrushesCommand::Ptr resizeBrushes(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const Vec3& delta, const bool lockTextures);
           
            const Model::BrushList& brushes() const;
        private:
            ResizeBrushesCommand(View::MapDocumentWPtr document, const Model::BrushFaceList& faces, const Model::BrushList& brushes, const Vec3& delta, const bool lockTextures);
            static Model::BrushList collectBrushes(const Model::BrushFaceList& faces);
            static String makeName(const Model::BrushList& brushes);

            bool doPerformDo();
            bool doPerformUndo();
            
            Command* doClone(View::MapDocumentSPtr document) const;
            bool doCollateWith(Command::Ptr command);
            
            bool moveBoundary(const Vec3& delta);
        };
    }
}

#endif /* defined(__TrenchBroom__ResizeBrushesCommand__) */
