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

#ifndef TrenchBroom_BitmapButton
#define TrenchBroom_BitmapButton

#include <wx/window.h>

class wxBitmap;

namespace TrenchBroom {
    namespace View {
        class BitmapButton : public wxWindow {
        protected:
            BitmapButton(wxWindow* parent, wxWindowID windowId);
        public:
            virtual ~BitmapButton();

            bool HasTransparentBackground() override;
            bool ShouldInheritColours() const override;

            wxSize DoGetBestClientSize() const override;

            void OnPaint(wxPaintEvent& event);
            void OnMouseDown(wxMouseEvent& event);
            void DoUpdateWindowUI(wxUpdateUIEvent& event) override;
        private:
            wxSize bitmapSize() const;
            virtual wxBitmap currentBitmap() const = 0;
            virtual void processClick() = 0;
        };
    }
}

#endif /* defined(TrenchBroom_BitmapButton) */
