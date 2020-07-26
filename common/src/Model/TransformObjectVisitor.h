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

#ifndef TrenchBroom_TransformObjectVisitor
#define TrenchBroom_TransformObjectVisitor

#include "FloatType.h"
#include "Model/NodeVisitor.h"
#include "Model/Object.h"

#include <optional>

namespace TrenchBroom {
    namespace Model {
        /**
         * Transforms objects by a given transformation.
         *
         * The visitor stops if an error occurs during transformation. In such a case, it's the caller's responsibility
         * to restore the nodes modified so far to their previous state.
         */
        class TransformObjectVisitor : public NodeVisitor {
        private:
            const vm::bbox3& m_worldBounds;
            const vm::mat4x4& m_transformation;
            bool m_lockTextures;
            std::optional<TransformError> m_error;
        public:
            TransformObjectVisitor(const vm::bbox3& worldBounds, const vm::mat4x4& transformation, bool lockTextures);

            const std::optional<TransformError>& error() const;
        private:
            void doVisit(WorldNode* world) override;
            void doVisit(LayerNode* layer) override;
            void doVisit(GroupNode* group) override;
            void doVisit(EntityNode* entity) override;
            void doVisit(BrushNode* brush) override;

            void transform(Object* object);
        };
    }
}

#endif /* defined(TrenchBroom_TransformObjectVisitor) */
