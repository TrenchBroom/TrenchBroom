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

#ifndef __TrenchBroom__SmartSpawnflagsEditor__
#define __TrenchBroom__SmartSpawnflagsEditor__

#include "Assets/AssetTypes.h"
#include "Model/ModelTypes.h"
#include "View/SmartPropertyEditor.h"
#include "View/ViewTypes.h"

#include <wx/gdicmn.h>

class wxCheckBox;
class wxColor;
class wxCommandEvent;
class wxScrolledWindow;
class wxString;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class FlagsEditor;
        class FlagChangedCommand;
        
        class SmartSpawnflagsEditor : public SmartPropertyEditor {
        private:
            static const size_t NumFlags = 24;
            static const size_t NumCols = 3;
            
            wxScrolledWindow* m_scrolledWindow;
            wxPoint m_lastScrollPos;
            FlagsEditor* m_flagsEditor;
            bool m_ignoreUpdates;
        public:
            SmartSpawnflagsEditor(View::MapDocumentWPtr document, View::ControllerWPtr controller);
            
            void OnFlagChanged(FlagChangedCommand& event);
        private:
            wxWindow* doCreateVisual(wxWindow* parent);
            void doDestroyVisual();
            void doUpdateVisual(const Model::EntityList& entities);
            void resetScrollPos();
            
            void getFlagsFromEntities(const Model::EntityList& entities, wxArrayString& labels) const;
            void getFlagValuesFromEntities(const Model::EntityList& entities, int& setFlags, int& mixedFlags) const;
            int getFlagValueFromEntity(const Model::Entity& entity) const;
        };
    }
}

#endif /* defined(__TrenchBroom__SmartSpawnflagsEditor__) */
