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
#include "SharedPointer.h"
#include "Model/ModelTypes.h"
#include "View/SnapshotCommand.h"

#include <vecmath/mat.h>
#include <vecmath/util.h>

namespace TrenchBroom {
    namespace View {
        class MapDocumentCommandFacade;

        class TransformObjectsCommand : public SnapshotCommand {
        public:
            static const CommandType Type;
            using Ptr = std::shared_ptr<TransformObjectsCommand>;
        private:
            typedef enum {
                Action_Translate,
                Action_Rotate,
                Action_Flip,
                Action_Shear,
                Action_Scale
            } Action;

            Action m_action;
            vm::mat4x4 m_transform;
            bool m_lockTextures;
        public:
            static Ptr translate(const vm::vec3& delta, bool lockTextures);
            static Ptr rotate(const vm::vec3& center, const vm::vec3& axis, FloatType angle, bool lockTextures);
            static Ptr scale(const vm::bbox3& oldBBox, const vm::bbox3& newBBox, bool lockTextures);
            static Ptr scale(const vm::vec3& center, const vm::vec3& scaleFactors, bool lockTextures);
            static Ptr shearBBox(const vm::bbox3& box, const vm::vec3& sideToShear, const vm::vec3& delta, bool lockTextures);
            static Ptr flip(const vm::vec3& center, vm::axis::type axis, bool lockTextures);
        private:
            TransformObjectsCommand(Action action, const String& name, const vm::mat4x4& transform, bool lockTextures);

            bool doPerformDo(MapDocumentCommandFacade* document) override;

            bool doIsRepeatable(MapDocumentCommandFacade* document) const override;
            UndoableCommand::Ptr doRepeat(MapDocumentCommandFacade* document) const override;

            bool doCollateWith(UndoableCommand::Ptr command) override;
        };
    }
}

#endif /* defined(TrenchBroom_TransformObjectsCommand) */
