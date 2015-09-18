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

#ifndef TrenchBroom_CameraLinkHelper
#define TrenchBroom_CameraLinkHelper

#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
    }
    
    namespace View {
        class CameraLinkHelper {
        private:
            typedef std::vector<Renderer::Camera*> CameraList;
            CameraList m_cameras;
            bool m_ignoreNotifications;
        public:
            CameraLinkHelper();
            ~CameraLinkHelper();
            
            void addCamera(Renderer::Camera* camera);
            void removeCamera(Renderer::Camera* camera);
        private:
            void cameraDidChange(const Renderer::Camera* camera);
        };
        
        class CameraLinkableView {
        public:
            virtual ~CameraLinkableView();
            void linkCamera(CameraLinkHelper& linkHelper);
        private:
            virtual void doLinkCamera(CameraLinkHelper& linkHelper) = 0;
        };
    }
}

#endif /* defined(TrenchBroom_CameraLinkHelper) */
