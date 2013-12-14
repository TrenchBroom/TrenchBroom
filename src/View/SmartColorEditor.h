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

#ifndef __TrenchBroom__SmartColorEditor__
#define __TrenchBroom__SmartColorEditor__

#include "View/SmartPropertyEditor.h"
#include "View/ViewTypes.h"

class wxRadioButton;
class wxColourPickerCtrl;
class wxPanel;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class ColorTable;
        
        class SmartColorEditor : public SmartPropertyEditor {
        private:
            static const size_t ColorHistoryCellSize = 15;
            
            typedef enum {
                Float,
                Byte,
                Mixed
            } ColorRange;
            
            wxPanel* m_panel;
            wxRadioButton* m_floatRadio;
            wxRadioButton* m_byteRadio;
            wxColourPickerCtrl* m_colorPicker;
            ColorTable* m_colorHistory;
        public:
            SmartColorEditor(View::MapDocumentPtr document, View::ControllerPtr controller);
        private:
            wxWindow* doCreateVisual(wxWindow* parent);
            void doDestroyVisual();
            void doUpdateVisual(const Model::EntityList& entities);
            void updateColorRange(const Model::EntityList& entities);
            ColorRange detectColorRange(const Model::EntityList& entities) const;
            ColorRange detectColorRange(const Model::Entity& entity) const;
            ColorRange combineColorRanges(ColorRange oldRange, ColorRange newRange) const;
            void updateColorHistory();
        };
    }
}

#endif /* defined(__TrenchBroom__SmartColorEditor__) */
