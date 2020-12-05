/*
 Copyright (C) 2010-2017 Kristian Duske
 Copyright (C) 2018 Eric Wasylishen

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

#pragma once

#include "FloatType.h"
#include "Model/Hit.h"
#include "Model/HitType.h"
#include "View/Tool.h"

#include <vecmath/forward.h>
#include <vecmath/vec.h>
#include <vecmath/bbox.h>

#include <memory>

namespace TrenchBroom {
    namespace Model {
        class PickResult;
    }

    namespace Renderer {
        class Camera;
    }

    namespace View {
        class MapDocument;

        class ShearObjectsTool : public Tool {
        public:
            static const Model::HitType::Type ShearToolSideHitType;
        private:
            std::weak_ptr<MapDocument> m_document;
            bool m_resizing;
            bool m_constrainVertical;
            vm::bbox3 m_bboxAtDragStart;
            Model::Hit m_dragStartHit;
            vm::vec3 m_dragCumulativeDelta;

        public:
            explicit ShearObjectsTool(std::weak_ptr<MapDocument> document);
            ~ShearObjectsTool() override;

            bool applies() const;

            void pickBackSides(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult);
            void pick2D(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult);
            void pick3D(const vm::ray3& pickRay, const Renderer::Camera& camera, Model::PickResult& pickResult);

        public:
            vm::bbox3 bounds() const;

            bool hasDragPolygon() const;
            vm::polygon3f dragPolygon() const;

            /**
             * If inside a drag, returns the bbox at the start of the drag.
             * Otherwise, returns the current bounds(). for rendering sheared bbox.
             */
            vm::bbox3 bboxAtDragStart() const;

            void startShearWithHit(const Model::Hit& hit);
            void commitShear();
            void cancelShear();
            void shearByDelta(const vm::vec3 &delta);

            const Model::Hit& dragStartHit() const;

            vm::mat4x4 bboxShearMatrix() const;
            vm::polygon3f shearHandle() const;

            void updatePickedSide(const Model::PickResult& pickResult);

            bool constrainVertical() const;
            void setConstrainVertical(bool constrainVertical);
        };
    }
}

#endif /* defined(TrenchBroom_ShearObjectsTool) */
