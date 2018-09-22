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

#ifndef TrenchBroom_TransformObjectsCommand
#define TrenchBroom_TransformObjectsCommand

#include "TrenchBroom.h"
#include "VecMath.h"
#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/SnapshotCommand.h"

namespace TrenchBroom {
    namespace View {
        class MapDocumentCommandFacade;
        
        class TransformObjectsCommand : public SnapshotCommand {
        public:
            static const CommandType Type;
            typedef std::shared_ptr<TransformObjectsCommand> Ptr;
        private:
            typedef enum {
                Action_Translate,
                Action_Rotate,
                Action_Flip,
                Action_Shear,
                Action_Scale
            } Action;
            
            Action m_action;
            Mat4x4 m_transform;
            bool m_lockTextures;
        public:
            static Ptr translate(const Vec3& delta, bool lockTextures);
            static Ptr rotate(const Vec3& center, const Vec3& axis, FloatType angle, bool lockTextures);
            static Ptr scale(const BBox3& oldBBox, const BBox3& newBBox, const bool lockTextures);
            static Ptr scale(const Vec3& center, const Vec3& scaleFactors, const bool lockTextures);
            static Ptr shearBBox(const BBox3& box, const Vec3& sideToShear, const Vec3& delta, const bool lockTextures);
            static Ptr flip(const Vec3& center, Math::Axis::Type axis, bool lockTextures);
        private:
            TransformObjectsCommand(Action action, const String& name, const Mat4x4& transform, bool lockTextures);

            bool doPerformDo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const override;
            
            bool doCollateWith(UndoableCommand::Ptr command) override;
        };
    }
}

#endif /* defined(TrenchBroom_TransformObjectsCommand) */
