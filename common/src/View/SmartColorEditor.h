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

#ifndef TrenchBroom_SmartColorEditor
#define TrenchBroom_SmartColorEditor

#include "SharedPointer.h"
#include "StringUtils.h"
#include "Model/ModelTypes.h"
#include "View/SmartAttributeEditor.h"
#include "View/ViewTypes.h"

#include <wx/colour.h>

class wxColourPickerCtrl;
class wxColourPickerEvent;
class wxCommandEvent;
class wxPanel;
class wxRadioButton;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class ColorTable;
        class ColorTableSelectedCommand;
        
        class SmartColorEditor : public SmartAttributeEditor {
        private:
            static const size_t ColorHistoryCellSize = 15;
            typedef std::vector<wxColour> wxColorList;
            
            wxPanel* m_panel;
            wxRadioButton* m_floatRadio;
            wxRadioButton* m_byteRadio;
            wxColourPickerCtrl* m_colorPicker;
            ColorTable* m_colorHistory;
        public:
            SmartColorEditor(View::MapDocumentWPtr document);
            
            void OnFloatRangeRadioButton(wxCommandEvent& event);
            void OnByteRangeRadioButton(wxCommandEvent& event);
            void OnColorPickerChanged(wxColourPickerEvent& event);
            void OnColorTableSelected(ColorTableSelectedCommand& event);
        private:
            wxWindow* doCreateVisual(wxWindow* parent);
            void doDestroyVisual();
            void doUpdateVisual(const Model::AttributableNodeList& attributables);

            class CollectColorVisitor;
            void updateColorRange(const Model::AttributableNodeList& attributables);
            void updateColorHistory();

            void setColor(const wxColor& wxColor) const;
        };
    }
}

#endif /* defined(TrenchBroom_SmartColorEditor) */
