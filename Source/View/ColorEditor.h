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

#ifndef __TrenchBroom__ColorEditor__
#define __TrenchBroom__ColorEditor__

#include "View/SmartPropertyEditor.h"

#include "Utility/Color.h"
#include "Utility/VecMath.h"

#include <wx/wx.h>

#include <functional>
#include <vector>

class wxColourPickerCtrl;
class wxColourPickerEvent;
class wxPanel;
class wxSlider;
class wxStaticText;

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace View {
        inline wxColour convertColor(Vec3f color) {
            if (color.x <= 1.0f && color.y <= 1.0f && color.z <= 1.0f)
                color *= 255.0f;
            return wxColour(static_cast<unsigned char>(color.x),
                            static_cast<unsigned char>(color.y),
                            static_cast<unsigned char>(color.z));
        }
        
        class ColorHistory : public wxPanel {
        public:
            typedef std::vector<wxColour> ColorList;
        private:
            class CallbackPtrBase {
            public:
                virtual ~CallbackPtrBase() {}
                virtual void call(const Vec3f& colour) = 0;
            };
            
            template <typename T>
            class CallbackPtr : public CallbackPtrBase {
            private:
                T* m_target;
                void (T::*m_callback)(const Vec3f&);
            public:
                CallbackPtr(T* target, void(T::*callback)(const Vec3f&)) :
                m_target(target),
                m_callback(callback) {}
                
                void call(const Vec3f& colour) {
                    (m_target->*m_callback)(colour);
                }
            };
            
            const int m_rows;
            const int m_cols;
            const int m_margin;
            
            Vec3f::List m_colors;
            CallbackPtrBase* m_callback;
        public:
            ColorHistory(wxWindow* parent, wxWindowID winId, int rows, int cols, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL | wxBORDER_SUNKEN);
            ~ColorHistory();
            
            inline void setColors(const Vec3f::List& colors) {
                m_colors = colors;
            }
            
            template <typename T>
            inline void setCallback(T* target, void(T::*callback)(const Vec3f&)) {
                delete m_callback;
                m_callback = new CallbackPtr<T>(target, callback);
            }
            
            void OnPaint(wxPaintEvent& event);
            void OnMouseUp(wxMouseEvent& event);

            DECLARE_EVENT_TABLE()
        };
        
        class ColorEditor : public SmartPropertyEditor {
        private:
            static const unsigned int ColorHistorySize = 8;

            class YIQOrder {
            public:
                inline bool operator()(const Vec3f& lhs, const Vec3f& rhs) const {
                    if (lhs == rhs)
                        return false;
                    
                    Vec3f lyiq, ryiq;
                    Color::rgbToYIQ(lhs.x, lhs.y, lhs.z, lyiq.x, lyiq.y, lyiq.z);
                    Color::rgbToYIQ(rhs.x, rhs.y, rhs.z, ryiq.x, ryiq.y, ryiq.z);
                    
                    if (lyiq.x < ryiq.x)
                        return true;
                    if (lyiq.x > ryiq.x)
                        return false;
                    if (lyiq.y < ryiq.y)
                        return true;
                    if (lyiq.y > ryiq.y)
                        return false;
                    if (lyiq.z < ryiq.z)
                        return true;
                    return false;
                }
            };

            wxPanel* m_panel;
            wxSlider* m_redSlider;
            wxSlider* m_greenSlider;
            wxSlider* m_blueSlider;
            wxColourPickerCtrl* m_colorPicker;
            ColorHistory* m_colorHistory;
            
            void updateColorHistory();
        protected:
            virtual wxWindow* createVisual(wxWindow* parent);
            virtual void destroyVisual();
            virtual void updateVisual();
        public:
            ColorEditor(SmartPropertyEditorManager& manager);
            
            void OnColorPickerChanged(wxColourPickerEvent& event);
            void OnColorSliderChanged(wxScrollEvent& event);
            void OnColorHistorySelected(const Vec3f& color);
        };
    }
}

#endif /* defined(__TrenchBroom__ColorEditor__) */
