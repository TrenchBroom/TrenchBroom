/*
 Copyright (C) 2010-2013 Kristian Duske
 
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
        class SmartSpawnflagsEditor : public SmartPropertyEditor {
        private:
            typedef enum {
                Unset,
                On,
                Off,
                Mixed
            } FlagValue;
            
            typedef std::vector<FlagValue> FlagList;
            typedef std::vector<wxCheckBox*> CheckBoxList;
            
            static const size_t NumFlags = 24;
            static const size_t NumCols = 3;
            
            wxScrolledWindow* m_scrolledWindow;
            CheckBoxList m_flags;
            wxPoint m_lastScrollPos;
        public:
            SmartSpawnflagsEditor(View::MapDocumentPtr document, View::ControllerPtr controller);
            
            void OnCheckBoxClicked(wxCommandEvent& event);
        private:
            wxWindow* doCreateVisual(wxWindow* parent);
            void doDestroyVisual();
            void doUpdateVisual();
            void resetScrollPos();
            
            FlagList getFlagValuesFromEntities(const Model::EntityList& entities) const;
            void setFlagValue(const Model::Entity& entity, FlagList& flags) const;
            void setFlagCheckBox(const size_t index, const FlagList& flags, const Assets::EntityDefinition* definition);
            void getColorAndLabelForFlag(const size_t flag, const Assets::EntityDefinition* definition, wxColour& colour, wxString& label) const;
            
            int getFlagFromEvent(wxCommandEvent& event) const;
            Model::PropertyValue getPropertyValueForFlag(const Model::Entity* entity, int flag, bool set) const;
        };
    }
}

#endif /* defined(__TrenchBroom__SmartSpawnflagsEditor__) */
