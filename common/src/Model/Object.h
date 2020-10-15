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

#ifndef TrenchBroom_Object
#define TrenchBroom_Object

#include "FloatType.h"

#include <kdl/result_forward.h>

#include <iosfwd>
#include <string>

namespace TrenchBroom {
    namespace Model {
        class GroupNode;
        class LayerNode;
        class Node;

        struct TransformError {
            std::string msg;
            
            friend std::ostream& operator<<(std::ostream& str, const TransformError& e);
        };

        class Object {
        protected:
            Object();
        public:
            virtual ~Object();

            Node* container();
            const Node* container() const;

            LayerNode* layer();
            const LayerNode* layer() const;

            GroupNode* group();
            const GroupNode* group() const;

            bool grouped() const;
            bool groupOpened() const;

            /**
             * Transforms this object by the given transformation.
             *
             * If the transformation fails, then this object may be partially transformed, but it will be in a valid
             * state. In that case, an error is returned.
             *
             * @param worldBounds the world bounds
             * @param transformation the transformation to apply
             * @param lockTextures whether textures should be locked
             * @return nothing or an error indicating why the operation failed
             */
            kdl::result<void, TransformError> transform(const vm::bbox3& worldBounds, const vm::mat4x4& transformation, bool lockTextures);
            bool contains(const Node* object) const;
            bool intersects(const Node* object) const;
        private: // subclassing interface
            virtual Node* doGetContainer() = 0;
            virtual LayerNode* doGetLayer() = 0;
            virtual GroupNode* doGetGroup() = 0;

            virtual kdl::result<void, TransformError> doTransform(const vm::bbox3& worldBounds, const vm::mat4x4& transformation, bool lockTextures) = 0;
            virtual bool doContains(const Node* node) const = 0;
            virtual bool doIntersects(const Node* node) const = 0;
        };
    }
}

#endif /* defined(TrenchBroom_Object) */
