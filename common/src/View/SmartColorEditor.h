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

#ifndef __TrenchBroom__SmartColorEditor__
#define __TrenchBroom__SmartColorEditor__

#include "SharedPointer.h"
#include "StringUtils.h"
#include "Model/ModelTypes.h"
#include "View/SmartPropertyEditor.h"
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
        
        class SmartColorEditor : public SmartPropertyEditor {
        public:
            typedef enum {
                ColorRange_Float,
                ColorRange_Byte,
                ColorRange_Mixed
            } ColorRange;

            class Color;
            typedef TrenchBroom::shared_ptr<const Color> ColorPtr;
            
            class Color : public TrenchBroom::enable_shared_from_this<Color> {
            public:
                virtual ~Color();
                
                static ColorPtr parseColor(const String& str);
                static ColorRange detectRange(const String& str);
                static ColorPtr fromWxColor(const wxColor& wxColor, ColorRange range);
                
                virtual ColorPtr toFloatColor() const = 0;
                virtual ColorPtr toByteColor() const = 0;
                ColorPtr toColor(ColorRange range) const;
                virtual wxColor toWxColor() const = 0;
                
                virtual String asString() const = 0;
            private:
                static ColorRange detectRange(const StringList& components);
            };
            
            template <typename T>
            class Color_ : public Color {
            protected:
                T m_v[3];
            public:
                T r() const {
                    return m_v[0];
                }
                
                T g() const {
                    return m_v[1];
                }
                
                T b() const {
                    return m_v[2];
                }
            };
            
            class FloatColor : public Color_<float> {
            public:
                FloatColor(float r, float g, float b);
                ColorPtr toFloatColor() const;
                ColorPtr toByteColor() const;
                wxColor toWxColor() const;
                String asString() const;
            private:
                void print(StringStream& str, const float f) const;
            };
            
            class ByteColor : public Color_<int> {
            public:
                ByteColor(int r, int g, int b);
                ColorPtr toFloatColor() const;
                ColorPtr toByteColor() const;
                wxColor toWxColor() const;
                String asString() const;
            };
        private:
            static const size_t ColorHistoryCellSize = 15;
            typedef std::vector<wxColour> wxColorList;
            typedef TrenchBroom::shared_ptr<Color> NonConstColorPtr;

            wxPanel* m_panel;
            wxRadioButton* m_floatRadio;
            wxRadioButton* m_byteRadio;
            wxColourPickerCtrl* m_colorPicker;
            ColorTable* m_colorHistory;
        public:
            SmartColorEditor(View::MapDocumentWPtr document, View::ControllerWPtr controller);
            
            void OnFloatRangeRadioButton(wxCommandEvent& event);
            void OnByteRangeRadioButton(wxCommandEvent& event);
            void OnColorPickerChanged(wxColourPickerEvent& event);
            void OnColorTableSelected(ColorTableSelectedCommand& event);
        private:
            wxWindow* doCreateVisual(wxWindow* parent);
            void doDestroyVisual();
            void doUpdateVisual(const Model::EntityList& entities);

            void updateColorRange(const Model::EntityList& entities);
            void updateColorPicker(const wxColor& color);
            void updateColorHistory(const wxColorList& selectedColors);
            wxColorList collectColors(const Model::EntityList& entities) const;
            void setColor(const wxColor& wxColor) const;
        };
    }
}

#endif /* defined(__TrenchBroom__SmartColorEditor__) */
