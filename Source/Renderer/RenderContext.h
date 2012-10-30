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

#ifndef __TrenchBroom__RenderContext__
#define __TrenchBroom__RenderContext__

#include "Renderer/Transformation.h"

namespace TrenchBroom {
    namespace Model {
        class Filter;
    }
    
    namespace Utility {
        class Console;
    }
    
    namespace View {
        class ViewOptions;
    }
    
    namespace Renderer {
        class Camera;

        class RenderContext {
        private:
            Camera& m_camera;
            Model::Filter& m_filter;
            Transformation m_transformation;
            View::ViewOptions& m_viewOptions;
            Utility::Console& m_console;
        public:
            RenderContext(Camera& camera, Model::Filter& filter, View::ViewOptions& viewOptions, Utility::Console& console) :
            m_camera(camera),
            m_filter(filter),
            m_transformation(m_camera.matrix(), false),
            m_viewOptions(viewOptions),
            m_console(console) {}
            
            inline Camera& camera() const {
                return m_camera;
            }
            
            inline const Model::Filter& filter() const {
                return m_filter;
            }
            
            inline Transformation& transformation() {
                return m_transformation;
            }
            
            inline View::ViewOptions& viewOptions() const {
                return m_viewOptions;
            }
            
            inline Utility::Console& console() const {
                return m_console;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__RenderContext__) */
