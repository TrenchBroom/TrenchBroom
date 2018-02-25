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

#ifndef TrenchBroom_ScaleObjectsToolPage
#define TrenchBroom_ScaleObjectsToolPage

#include "TrenchBroom.h"
#include "VecMath.h"
#include "View/ViewTypes.h"

#include <wx/panel.h>

class wxButton;
class wxChoice;
class wxComboBox;

namespace TrenchBroom {
    namespace View {
        class ScaleObjectsTool;
        class SpinControl;
        class SpinControlEvent;
        
        class ScaleObjectsToolPage : public wxPanel {
        private:
            MapDocumentWPtr m_document;
            ScaleObjectsTool* m_tool;

            wxComboBox* m_recentlyUsedCentersList;
            wxButton* m_resetCenterButton;
            
            SpinControl* m_angle;
            wxChoice* m_axis;
            wxButton* m_rotateButton;
        public:
            ScaleObjectsToolPage(wxWindow* parent, MapDocumentWPtr document, ScaleObjectsTool* tool);
            void setAxis(Math::Axis::Type axis);
            void setRecentlyUsedCenters(const Vec3::List& centers);
            void setCurrentCenter(const Vec3& center);
        private:
            void createGui();
            
            void OnIdle(wxIdleEvent& event);
            void OnCenterChanged(wxCommandEvent& event);
            void OnResetCenter(wxCommandEvent& event);
            void OnAngleChanged(SpinControlEvent& event);
            void OnUpdateRotateButton(wxUpdateUIEvent& event);
            void OnRotate(wxCommandEvent& event);
            Vec3 getAxis() const;
        };
    }
}

#endif /* defined(TrenchBroom_ScaleObjectsToolPage) */
