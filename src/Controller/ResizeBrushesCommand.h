/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
#include "Controller/Command.h"
#include "Model/ModelTypes.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Controller {
        class ResizeBrushesCommand : public Command {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<ResizeBrushesCommand> Ptr;
        private:
            View::MapDocumentPtr m_document;
            Model::BrushFaceList m_faces;
            Model::BrushList m_brushes;
            Vec3 m_delta;
            bool m_lockTextures;
        public:
            static ResizeBrushesCommand::Ptr resizeBrushes(View::MapDocumentPtr document, const Model::BrushFaceList& faces, const Vec3& delta, const bool lockTextures);
           
            const Model::BrushList& brushes() const;
        private:
            ResizeBrushesCommand(View::MapDocumentPtr document, const Model::BrushFaceList& faces, const Model::BrushList& brushes, const Vec3& delta, const bool lockTextures);
            static Model::BrushList collectBrushes(const Model::BrushFaceList& faces);
            static String makeName(const Model::BrushList& brushes);

            bool doPerformDo();
            bool doPerformUndo();
            Model::ObjectList doAffectedObjects() const;
            
            bool moveBoundary(const Vec3& delta);
        };
    }
}

#endif /* defined(__TrenchBroom__ResizeBrushesCommand__) */
