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

#pragma once

#include "FloatType.h"
#include "ToolChain.h"
#include "Model/HitFilter.h"
#include "Model/HitType.h"
#include "View/InputState.h"

#include <vecmath/vec.h>
#include <vecmath/line.h>
#include <vecmath/plane.h>

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Hit;
        class HitQuery;
        class PickResult;
    }

    namespace Renderer {
        class RenderBatch;
        class RenderContext;
    }

    namespace View {
        class InputState;
        class Tool;

        class PickingPolicy {
        public:
            virtual ~PickingPolicy();
        public:
            virtual void doPick(const InputState& inputState, Model::PickResult& pickResult) = 0;
        };

        class NoPickingPolicy : public PickingPolicy {
        public:
            ~NoPickingPolicy() override;
        public:
            void doPick(const InputState& inputState, Model::PickResult& pickResult) override;
        };

        class KeyPolicy {
        public:
            virtual ~KeyPolicy();
        public:
            virtual void doModifierKeyChange(const InputState& inputState) = 0;
        };

        class NoKeyPolicy : public KeyPolicy {
        public:
            ~NoKeyPolicy() override;
        public:
            void doModifierKeyChange(const InputState& inputState) override;
        };

        class MousePolicy {
        public:
            virtual ~MousePolicy();
        public:
            virtual void doMouseDown(const InputState& inputState);
            virtual void doMouseUp(const InputState& inputState);
            virtual bool doMouseClick(const InputState& inputState);
            virtual bool doMouseDoubleClick(const InputState& inputState);
            virtual void doMouseMove(const InputState& inputState);
            virtual void doMouseScroll(const InputState& inputState);
        };

        using NoMousePolicy = MousePolicy;

        class RenderPolicy {
        public:
            virtual ~RenderPolicy();
        public:
            virtual void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const;
            virtual void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch);
        };

        using NoRenderPolicy = RenderPolicy;

        class DropPolicy {
        public:
            virtual ~DropPolicy();
        public:
            virtual bool doDragEnter(const InputState& inputState, const std::string& payload) = 0;
            virtual bool doDragMove(const InputState& inputState) = 0;
            virtual void doDragLeave(const InputState& inputState) = 0;
            virtual bool doDragDrop(const InputState& inputState) = 0;
        };

        class NoDropPolicy : public DropPolicy {
        public:
            ~NoDropPolicy() override;
        public:
            bool doDragEnter(const InputState& inputState, const std::string& payload) override;
            bool doDragMove(const InputState& inputState) override;
            void doDragLeave(const InputState& inputState) override;
            bool doDragDrop(const InputState& inputState) override;
        };

        class DragTracker;

        class ToolController {
        public:
            virtual ~ToolController();

            Tool* tool();
            const Tool* tool() const;
            bool toolActive() const;

            virtual void pick(const InputState& inputState, Model::PickResult& pickResult) = 0;

            virtual void modifierKeyChange(const InputState& inputState) = 0;

            virtual void mouseDown(const InputState& inputState) = 0;
            virtual void mouseUp(const InputState& inputState) = 0;
            virtual bool mouseClick(const InputState& inputState) = 0;
            virtual bool mouseDoubleClick(const InputState& inputState) = 0;
            virtual void mouseMove(const InputState& inputState) = 0;
            virtual void mouseScroll(const InputState& inputState) = 0;

            virtual std::unique_ptr<DragTracker> acceptMouseDrag(const InputState& inputState);
            virtual bool anyToolDragging(const InputState& inputState) const = 0;

            virtual void setRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) = 0;
            virtual void render(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) = 0;

            virtual bool dragEnter(const InputState& inputState, const std::string& payload) = 0;
            virtual bool dragMove(const InputState& inputState) = 0;
            virtual void dragLeave(const InputState& inputState) = 0;
            virtual bool dragDrop(const InputState& inputState) = 0;

            virtual bool cancel() = 0;
        protected:
            void refreshViews();
        private:
            virtual Tool* doGetTool() = 0;
            virtual const Tool* doGetTool() const = 0;
        };

        template <class PickingPolicyType, class KeyPolicyType, class MousePolicyType, class RenderPolicyType, class DropPolicyType>
        class ToolControllerBase : public ToolController, protected PickingPolicyType, protected KeyPolicyType, protected MousePolicyType, protected RenderPolicyType, protected DropPolicyType {
        public:
            ~ToolControllerBase() override = default;

            void pick(const InputState& inputState, Model::PickResult& pickResult) override {
                static_cast<PickingPolicyType*>(this)->doPick(inputState, pickResult);
            }

            void modifierKeyChange(const InputState& inputState) override {
                static_cast<KeyPolicyType*>(this)->doModifierKeyChange(inputState);
            }

            void mouseDown(const InputState& inputState) override {
                static_cast<MousePolicyType*>(this)->doMouseDown(inputState);
            }

            void mouseUp(const InputState& inputState) override {
                static_cast<MousePolicyType*>(this)->doMouseUp(inputState);
            }

            bool mouseClick(const InputState& inputState) override {
                return static_cast<MousePolicyType*>(this)->doMouseClick(inputState);
            }

            bool mouseDoubleClick(const InputState& inputState) override {
                return static_cast<MousePolicyType*>(this)->doMouseDoubleClick(inputState);
            }

            void mouseMove(const InputState& inputState) override {
                static_cast<MousePolicyType*>(this)->doMouseMove(inputState);
            }

            void mouseScroll(const InputState& inputState) override {
                static_cast<MousePolicyType*>(this)->doMouseScroll(inputState);
            }

            bool anyToolDragging(const InputState& inputState) const override {
                return inputState.anyToolDragging();
            }

            void setRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) override {
                static_cast<RenderPolicyType*>(this)->doSetRenderOptions(inputState, renderContext);
            }

            void render(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override {
                static_cast<RenderPolicyType*>(this)->doRender(inputState, renderContext, renderBatch);
            }

            bool dragEnter(const InputState& inputState, const std::string& payload) override {
                return static_cast<DropPolicyType*>(this)->doDragEnter(inputState, payload);
            }

            bool dragMove(const InputState& inputState) override {
                return static_cast<DropPolicyType*>(this)->doDragMove(inputState);
            }

            void dragLeave(const InputState& inputState) override {
                static_cast<DropPolicyType*>(this)->doDragLeave(inputState);
            }

            bool dragDrop(const InputState& inputState) override {
                return static_cast<DropPolicyType*>(this)->doDragDrop(inputState);
            }

            bool cancel() override {
                return doCancel();
            }
        private:
            virtual bool doCancel() = 0;
        };

        class ToolControllerGroup : public ToolControllerBase<PickingPolicy, KeyPolicy, MousePolicy, RenderPolicy, DropPolicy> {
        private:
            ToolChain m_chain;
            ToolController* m_dropReceiver;
        public:
            ToolControllerGroup();
            ~ToolControllerGroup() override;
        protected:
            void addController(std::unique_ptr<ToolController> controller);
        protected:
            void doPick(const InputState& inputState, Model::PickResult& pickResult) override;

            void doModifierKeyChange(const InputState& inputState) override;

            void doMouseDown(const InputState& inputState) override;
            void doMouseUp(const InputState& inputState) override;
            bool doMouseClick(const InputState& inputState) override;
            bool doMouseDoubleClick(const InputState& inputState) override;
            void doMouseMove(const InputState& inputState) override;
            void doMouseScroll(const InputState& inputState) override;

            std::unique_ptr<DragTracker> acceptMouseDrag(const InputState& inputState) override;

            void doSetRenderOptions(const InputState& inputState, Renderer::RenderContext& renderContext) const override;
            void doRender(const InputState& inputState, Renderer::RenderContext& renderContext, Renderer::RenderBatch& renderBatch) override;

            bool doDragEnter(const InputState& inputState, const std::string& payload) override;
            bool doDragMove(const InputState& inputState) override;
            void doDragLeave(const InputState& inputState) override;
            bool doDragDrop(const InputState& inputState) override;

            bool doCancel() override;
        private: // subclassing interface
            virtual bool doShouldHandleMouseDrag(const InputState& inputState) const;
            virtual bool doShouldHandleDrop(const InputState& inputState, const std::string& payload) const;
        };
    }
}

