/*
 Copyright (C) 2010-2012 Kristian Duske
 
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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__ClipTool__
#define __TrenchBroom__ClipTool__

#include "Controller/Tool.h"
#include "Model/Filter.h"
#include "Utility/VecMath.h"

#include <cassert>

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Model {
        namespace HitType {
            static const Type ClipHandleHit    = 1 << 5;
        }
        
        class ClipHandleHit : public Hit {
        private:
            unsigned int m_index;
        public:
            ClipHandleHit(const Vec3f& hitPoint, float distance, unsigned int index);
            bool pickable(Filter& filter) const;
            
            inline unsigned int index() const {
                return m_index;
            }
        };
    }

    namespace Renderer {
        class BrushFigure;
        class Vbo;
        class RenderContext;
    }
    
    namespace View {
        class DocumentViewHolder;
    }
    
    namespace Controller {
        class Command;
        
        class ClipTool : public Tool {
        public:
            typedef enum {
                CMFront,
                CMBack,
                CMBoth
            } ClipSide;
        private:
            class ClipFilter : public Model::Filter {
            protected:
                Model::Filter& m_defaultFilter;
            public:
                ClipFilter(Model::Filter& defaultFilter) :
                m_defaultFilter(defaultFilter) {}
                
                virtual inline bool entityVisible(const Model::Entity& entity) const {
                    return m_defaultFilter.entityVisible(entity);
                }
                
                virtual inline bool entityPickable(const Model::Entity& entity) const {
                    return false;
                }
                
                virtual inline bool brushVisible(const Model::Brush& brush) const {
                    return m_defaultFilter.brushVisible(brush);
                }
                
                virtual inline bool brushPickable(const Model::Brush& brush) const {
                    return m_defaultFilter.brushPickable(brush);
                }
                
                virtual inline bool brushVerticesPickable(const Model::Brush& brush) const {
                    return false;
                }
            };

            ClipFilter m_filter;
            Vec3f m_points[3];
            Vec3f::List m_normals[3];
            unsigned int m_numPoints;
            int m_hitIndex;
            bool m_directHit;
            
            ClipSide m_clipSide;
            Model::EntityBrushesMap m_frontBrushes;
            Model::EntityBrushesMap m_backBrushes;
            Renderer::BrushFigure* m_frontBrushFigure;
            Renderer::BrushFigure* m_backBrushFigure;
            
            Vec3f selectNormal(const Vec3f::List& normals1, const Vec3f::List& normals2) const;
            void updateBrushes();
            Vec3f::List getNormals(const Vec3f& hitPoint, const Model::Face& hitFace) const;
            bool isPointIdenticalWithExistingPoint(const Vec3f& point) const;
            bool isPointLinearlyDependent(const Vec3f& point) const;
        protected:
            bool handleActivate(InputState& inputState);
            bool handleDeactivate(InputState& inputState);
            bool handleIsModal(InputState& inputState);
            
            void handlePick(InputState& inputState);
            void handleRender(InputState& inputState, Renderer::Vbo& vbo, Renderer::RenderContext& renderContext);
            void handleFreeRenderResources();

            bool handleMouseUp(InputState& inputState);

            bool handleStartDrag(InputState& inputState);
            bool handleDrag(InputState& inputState);
            void handleEndDrag(InputState& inputState);

            bool handleNavigateUp(InputState& inputState);
            void handleUpdate(const Command& command, InputState& inputState);
        public:
            ClipTool(View::DocumentViewHolder& documentViewHolder, InputController& inputController);
            
            inline unsigned int numPoints() const {
                assert(active());
                return m_numPoints;
            }
            
            void toggleClipSide();
            void deleteLastPoint();
            void performClip();
        };
    }
}

#endif /* defined(__TrenchBroom__ClipTool__) */
