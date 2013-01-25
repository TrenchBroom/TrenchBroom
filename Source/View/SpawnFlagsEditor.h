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

#ifndef __TrenchBroom__SpawnFlagsEditor__
#define __TrenchBroom__SpawnFlagsEditor__

#include "View/SmartPropertyEditor.h"

class wxScrolledWindow;
class wxCheckBox;

namespace TrenchBroom {
    namespace View {
        class SpawnFlagsEditor : public SmartPropertyEditor {
        private:
            typedef enum {
                Unset,
                On,
                Off,
                Mixed
            } FlagValue;
            
            wxScrolledWindow* m_scrolledWindow;
            wxCheckBox* m_flags[24];
        protected:
            virtual wxWindow* createVisual(wxWindow* parent);
            virtual void destroyVisual();
            virtual void updateVisual();
        public:
            SpawnFlagsEditor(SmartPropertyEditorManager& manager);
            
            void OnCheckBoxClicked(wxCommandEvent& event);
        };
    }
}

#endif /* defined(__TrenchBroom__SpawnFlagsEditor__) */
