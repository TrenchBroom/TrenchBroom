#/*
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
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef TrenchBroom_SpinControl
#define TrenchBroom_SpinControl

#include <wx/control.h>
#include <wx/event.h>
#include <wx/panel.h>

#include <cassert>

class wxSpinButton;
class wxTextCtrl;

namespace TrenchBroom {
    namespace View {
        class SpinControlEvent : public wxNotifyEvent {
        private:
            bool m_spin;
            double m_value;
        public:
            SpinControlEvent();
            SpinControlEvent(wxEventType commandType, int winId, bool spin, double value);
            SpinControlEvent(const SpinControlEvent& event);
            bool IsSpin() const;
            double GetValue() const;
            virtual wxEvent* Clone() const;
        private:
            DECLARE_DYNAMIC_CLASS(SpinControlEvent)
        };
        
        class SpinControl : public wxPanel {
        protected:
            wxTextCtrl* m_text;
            wxSpinButton* m_spin;
            
            double m_minValue;
            double m_maxValue;
            double m_regularIncrement;
            double m_shiftIncrement;
            double m_ctrlIncrement;
            double m_value;
            unsigned int m_minDigits;
            unsigned int m_maxDigits;
            wxString m_format;
        public:
            SpinControl(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=0, const wxValidator &validator=wxDefaultValidator, const wxString &name=wxControlNameStr);
            
            double GetValue() const;
            void SetValue(double doubleValue);
            void SetValue(const wxString& textValue);
            void SetRange(double min, double max);
            void SetIncrements(double regularIncrement, double shiftIncrement, double ctrlIncrement);
            void SetDigits(unsigned int minDigits, unsigned int maxDigits);
            void SetHint(const wxString& hint);
            bool Enable(bool enable = true);
            void SetFocus();
        private:
            wxSize DoGetBestSize() const;
            
            bool InRange(double value) const;
            double AdjustToRange(double value);
            bool DoSetValue(double value);
            wxString DoFormat(double value) const;
            bool DoSendEvent(bool spin, double value);
            bool SyncFromText();
            
            void OnTextKeyDown(wxKeyEvent& event);
            void OnTextEnter(wxCommandEvent& event);
            void OnTextSetFocus(wxFocusEvent& event);
            void OnTextKillFocus(wxFocusEvent& event);
            void OnSpinButtonUp(wxSpinEvent& event);
            void OnSpinButtonDown(wxSpinEvent& event);
            void OnMouseWheel(wxMouseEvent& event);
            
            void Spin(double multiplier, const wxKeyboardState& keyboardState);
            
            void OnSetFocus(wxFocusEvent& event);
        };
    }
}

typedef void (wxEvtHandler::*SpinControlEventFunction)(TrenchBroom::View::SpinControlEvent &);

wxDECLARE_EVENT(SPIN_CONTROL_EVENT, TrenchBroom::View::SpinControlEvent);
#define SpinControlEventHandler(func) wxEVENT_HANDLER_CAST(SpinControlEventFunction, func)

#endif /* defined(TrenchBroom_SpinControl) */
