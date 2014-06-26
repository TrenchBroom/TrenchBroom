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

#ifndef __TrenchBroom__UVTextureGridTool__
#define __TrenchBroom__UVTextureGridTool__

#include "Hit.h"
#include "View/Tool.h"

namespace TrenchBroom {
    namespace View {
        class UVViewHelper;
        
        class UVViewTextureGridTool : public ToolImpl<NoActivationPolicy, PickingPolicy, NoMousePolicy, MouseDragPolicy, NoDropPolicy, RenderPolicy> {
        public:
            static const Hit::HitType XHandleHit;
            static const Hit::HitType YHandleHit;
        protected:
            UVViewHelper& m_helper;
            
            Vec2i m_handle;
            Vec2b m_selector;
            Vec2f m_lastHitPoint; // in non-scaled, non-translated texture coordinates
        protected:
            UVViewTextureGridTool(MapDocumentWPtr document, ControllerWPtr controller, UVViewHelper& helper);
        private:
            void doPick(const InputState& inputState, Hits& hits);

            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);

            Vec2i getScaleHandle(const Hit& xHit, const Hit& yHit) const;
            Vec2f getHitPoint(const Ray3& pickRay) const;

            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag(const InputState& inputState);
        private:
            virtual bool checkIfDragApplies(const InputState& inputState, const Hit& xHit, const Hit& yHit) const = 0;
            virtual String getActionName() const = 0;
            virtual Vec2f performDrag(const Vec2f& delta) = 0;
        protected:
            Vec2f getHandlePos() const;
            Vec2f getScaledTranslatedHandlePos() const;
        };
    }
}

#endif /* defined(__TrenchBroom__UVTextureGridTool__) */
