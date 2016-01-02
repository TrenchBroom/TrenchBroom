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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "SpinControl.h"

#include <wx/sizer.h>
#include <wx/spinbutt.h>
#include <wx/textctrl.h>

#include <limits>

wxDEFINE_EVENT(SPIN_CONTROL_EVENT, TrenchBroom::View::SpinControlEvent);

namespace TrenchBroom {
    namespace View {
        IMPLEMENT_DYNAMIC_CLASS(SpinControlEvent, wxNotifyEvent)
        SpinControlEvent::SpinControlEvent() :
        wxNotifyEvent(wxEVT_NULL, wxID_ANY),
        m_spin(true),
        m_value(0.0) {}
        
        SpinControlEvent::SpinControlEvent(wxEventType commandType, int winId, bool spin, double value) :
        wxNotifyEvent(commandType, winId),
        m_spin(spin),
        m_value(value) {}
        
        SpinControlEvent::SpinControlEvent(const SpinControlEvent& event) :
        wxNotifyEvent(event),
        m_spin(event.IsSpin()),
        m_value(event.GetValue()) {}
        
        bool SpinControlEvent::IsSpin() const {
            return m_spin;
        }
        
        double SpinControlEvent::GetValue() const {
            return m_value;
        }
        
        wxEvent* SpinControlEvent::Clone() const {
            return new SpinControlEvent(*this);
        }
        
        SpinControl::SpinControl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) :
        wxPanel(parent, id, pos, size, (style & ~wxBORDER_MASK) | wxBORDER_NONE, name),
        m_text(NULL),
        m_spin(NULL),
        m_minValue(std::numeric_limits<double>::min()),
        m_maxValue(std::numeric_limits<double>::max()),
        m_regularIncrement(0.0),
        m_shiftIncrement(0.0),
        m_ctrlIncrement(0.0),
        m_value(0.0),
        m_minDigits(0),
        m_maxDigits(0) { // m_digits must be different from 0 because it's being set to 0 below in the constructor
            m_text = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT);
            m_spin = new wxSpinButton(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL);
            
            m_text->SetSizeHints(wxDefaultCoord, wxDefaultCoord);
            m_text->SetToolTip(GetToolTipText());
            
            m_spin->SetToolTip(GetToolTipText());
            m_spin->SetSizeHints(wxDefaultCoord, wxDefaultCoord);
            m_spin->SetRange(-32000, 32000);
            
            DoSetValue(m_value);
            SetDigits(0, 0);
            
            wxSizer* textSizer = new wxBoxSizer(wxVERTICAL);
            textSizer->Add(m_text, 0, wxEXPAND | wxALIGN_CENTRE_VERTICAL);
            
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            sizer->Add(textSizer, 1, wxEXPAND
#ifdef _WIN32
                       | wxTOP | wxBOTTOM, 1
#endif
            );
            sizer->Add(m_spin);
            SetSizer(sizer);
            
            SetInitialSize(size);
            Move(pos);
            
            Bind(wxEVT_MOUSEWHEEL, &SpinControl::OnMouseWheel, this);
            m_text->Bind(wxEVT_MOUSEWHEEL, &SpinControl::OnMouseWheel, this);
            m_spin->Bind(wxEVT_MOUSEWHEEL, &SpinControl::OnMouseWheel, this);
            
            m_text->Bind(wxEVT_KEY_DOWN, &SpinControl::OnTextKeyDown, this);
            m_text->Bind(wxEVT_COMMAND_TEXT_ENTER, &SpinControl::OnTextEnter, this);
            m_text->Bind(wxEVT_SET_FOCUS, &SpinControl::OnTextSetFocus, this);
            m_text->Bind(wxEVT_KILL_FOCUS, &SpinControl::OnTextKillFocus, this);
            m_spin->Bind(wxEVT_SPIN_UP, &SpinControl::OnSpinButtonUp, this);
            m_spin->Bind(wxEVT_SPIN_DOWN, &SpinControl::OnSpinButtonDown, this);
            
            Bind(wxEVT_SET_FOCUS, &SpinControl::OnSetFocus, this);
        }
        
        double SpinControl::GetValue() const {
            return m_value;
        }
        
        void SpinControl::SetValue(double doubleValue) {
            if (InRange(doubleValue))
                DoSetValue(doubleValue);
        }
        
        void SpinControl::SetValue(const wxString& textValue) {
            double doubleValue;
            if (textValue.ToDouble(&doubleValue) && InRange(doubleValue)) {
                DoSetValue(doubleValue);
            } else {
                m_text->SetValue(textValue);
                m_text->SetSelection(0, -1);
                m_text->SetInsertionPointEnd();
            }
        }
        
        void SpinControl::SetRange(double min, double max) {
            assert(min < max);
            
            m_minValue = min;
            m_maxValue = max;
            
            DoSetValue(AdjustToRange(m_value));
        }
        
        void SpinControl::SetIncrements(double regularIncrement, double shiftIncrement, double ctrlIncrement) {
            m_regularIncrement = regularIncrement;
            m_shiftIncrement = shiftIncrement;
            m_ctrlIncrement = ctrlIncrement;
        }
        
        void SpinControl::SetDigits(unsigned int minDigits, unsigned int maxDigits) {
            assert(minDigits <= maxDigits);
            m_minDigits = minDigits;
            m_maxDigits = maxDigits;
            
            m_format.Clear();
            m_format << "%." << m_maxDigits << "f";
            DoSetValue(m_value);
        }
        
        void SpinControl::SetHint(const wxString& hint) {
#if defined __APPLE__
            m_text->SetHint(hint);
#endif
        }
        
        bool SpinControl::Enable(bool enable) {
            if (wxPanel::Enable(enable)) {
                m_text->Enable(enable);
                m_spin->Enable(enable);
                return true;
            }
            return false;
        }
        
        void SpinControl::SetFocus() {
            m_text->SetFocus();
        }
        
        wxSize SpinControl::DoGetBestSize() const {
            wxSize spinSize = m_spin->GetBestSize();
            wxSize textSize = m_text->GetBestSize();
            
            return wxSize(spinSize.x + textSize.x + 0, textSize.y);
        }
        
        bool SpinControl::InRange(double value) const {
            return value >= m_minValue && value <= m_maxValue;
        }
        
        double SpinControl::AdjustToRange(double value) {
            if (m_value < m_minValue)
                return m_minValue;
            else if (m_value > m_maxValue)
                return m_maxValue;
            return value;
        }
        
        bool SpinControl::DoSetValue(double value) {
            if (!InRange(value))
                return false;
            
            const wxString str = DoFormat(value);
            if (value == m_value && str == m_text->GetValue())
                return false;
            
            str.ToDouble(&m_value);
            m_text->SetValue(str);
            m_text->SetInsertionPointEnd();
            m_text->DiscardEdits();
            return true;
        }
        
        wxString SpinControl::DoFormat(const double value) const {
            wxString str(wxString::Format(m_format.c_str(), value));

            if (m_minDigits < m_maxDigits) {
                while (str.Length() > m_minDigits && str.Last() == '0')
                    str.RemoveLast();
                if (str.Last() == '.') {
                    assert(m_minDigits == 0);
                    str.RemoveLast();
                }
            }
            
            return str;
        }

        bool SpinControl::DoSendEvent(const bool spin, const double value) {
            SpinControlEvent event(SPIN_CONTROL_EVENT, GetId(), spin, value);
            event.SetEventObject( this );
            GetEventHandler()->ProcessEvent( event );
            return event.IsAllowed();
        }
        
        bool SpinControl::SyncFromText() {
            if (!m_text->IsModified())
                return false;
            
            double textValue;
            if (m_text->GetValue().ToDouble(&textValue))
                textValue = AdjustToRange(textValue);
            else
                textValue = m_value;
            return DoSetValue(textValue);
        }
        
        void SpinControl::OnTextKeyDown(wxKeyEvent& event) {
            if (IsBeingDeleted()) return;

            switch (event.GetKeyCode()) {
                case WXK_UP:
                    Spin(+1.0, event);
                    break;
                case WXK_DOWN:
                    Spin(-1.0, event);
                    break;
                default:
                    event.Skip();
                    break;
            }
        }
        
        void SpinControl::OnTextEnter(wxCommandEvent& event) {
            if (IsBeingDeleted()) return;

            if (SyncFromText())
                DoSendEvent(false, GetValue());
        }
        
        void SpinControl::OnTextSetFocus(wxFocusEvent& event) {
            if (IsBeingDeleted()) return;
            m_text->SelectAll();
            event.Skip();
        }

        void SpinControl::OnTextKillFocus(wxFocusEvent& event) {
            if (IsBeingDeleted()) return;

            if (SyncFromText())
                DoSendEvent(false, GetValue());
            event.Skip();
        }
        
        void SpinControl::OnSpinButtonUp(wxSpinEvent& event) {
            if (IsBeingDeleted()) return;

            Spin(+1.0, wxGetMouseState());
        }
        
        void SpinControl::OnSpinButtonDown(wxSpinEvent& event) {
            if (IsBeingDeleted()) return;

            Spin(-1.0, wxGetMouseState());
        }
        
        void SpinControl::OnMouseWheel(wxMouseEvent& event) {
            if (IsBeingDeleted()) return;

            double multiplier = event.GetWheelRotation() > 0 ? 1.0 : -1.0;
#if defined __APPLE__
            if (event.ShiftDown())
                multiplier *= -1.0;
#endif
            Spin(multiplier, event);
        }
        
        void SpinControl::Spin(const double multiplier, const wxKeyboardState& keyboardState) {
            double increment = 0.0f;
            switch (keyboardState.GetModifiers()) {
                case wxMOD_CMD:
                    increment = m_ctrlIncrement;
                    break;
                case wxMOD_SHIFT:
                    increment = m_shiftIncrement;
                    break;
                default:
                    increment = m_regularIncrement;
                    break;
            }
            
            increment *= multiplier;
            const double newValue = GetValue() + increment;
            if (DoSendEvent(true, increment))
                DoSetValue(newValue);
        }
        
        void SpinControl::OnSetFocus(wxFocusEvent& event) {
            if (IsBeingDeleted()) return;

            // no idea why this is necessary, but it works
            SetFocus();
        }
    }
}
