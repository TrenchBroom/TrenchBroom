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

#ifndef TrenchBroom_UVShearTool
#define TrenchBroom_UVShearTool

#include "Model/Hit.h"
#include "View/Tool.h"
#include "View/ToolAdapter.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace View {
        class UVViewHelper;
        
        class UVShearTool : public ToolAdapterBase<PickingPolicy, NoKeyPolicy, NoMousePolicy, MouseDragPolicy, NoRenderPolicy, NoDropPolicy>, public Tool {
        private:
            static const Model::Hit::HitType XHandleHit;
            static const Model::Hit::HitType YHandleHit;
        private:
            MapDocumentWPtr m_document;
            UVViewHelper& m_helper;
            
            Vec2b m_selector;
            Vec3 m_xAxis;
            Vec3 m_yAxis;
            Vec2f m_initialHit;
            Vec2f m_lastHit;
        public:
            UVShearTool(MapDocumentWPtr document, UVViewHelper& helper);
        private:
            Tool* doGetTool();
            
            void doPick(const InputState& inputState, Model::PickResult& pickResult);
            
            bool doStartMouseDrag(const InputState& inputState);
            bool doMouseDrag(const InputState& inputState);
            void doEndMouseDrag(const InputState& inputState);
            void doCancelMouseDrag();
            
            Vec2f getHit(const Ray3& pickRay) const;
            
            bool doCancel();
        };
    }
}

#endif /* defined(TrenchBroom_UVShearTool) */
