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

#ifndef TrenchBroom_MoveToolAdapter
#define TrenchBroom_MoveToolAdapter

#include "View/ToolAdapter.h"

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/MapDocument.h"
#include "View/MoveToolHelper.h"

#ifdef _MSC_VER
// We get a warning here because we pass 'this' to the member initializer, but it's okay because we don't use it in the member's initializer.
#pragma warning(push)
#pragma warning(disable : 4355)
#endif

namespace TrenchBroom {
    namespace View {
        class ToolActivationDelegate;
        
        template <class PickingPolicyType, class MousePolicyType>
        class MoveToolAdapter : public ToolAdapterBase<PickingPolicyType, KeyPolicy, MousePolicyType, DelegatingMouseDragPolicy, RenderPolicy, NoDropPolicy> {
        private:
            typedef ToolAdapterBase<PickingPolicyType, KeyPolicy, MousePolicyType, DelegatingMouseDragPolicy, RenderPolicy, NoDropPolicy> Super;
            MoveToolHelper* m_helper;
        protected:
            MoveToolAdapter(MoveToolHelper* helper) :
            m_helper(helper) {
                assert(m_helper != NULL);
            }
        public:
            virtual ~MoveToolAdapter() {
                delete m_helper;
            }
        private:
            void doModifierKeyChange(const InputState& inputState) {
                if (Super::dragging())
                    m_helper->resetRestricter(inputState);
            }

            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) {
                m_helper->render(inputState, renderContext, renderBatch);
            }
        };
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif /* defined(TrenchBroom_MoveToolAdapter) */
