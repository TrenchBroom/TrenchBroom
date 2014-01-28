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

DEFINE_EVENT_TYPE(EVT_SPINCONTROL_EVENT)

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
m_digits(0),
m_format("%g") {
    m_text = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_RIGHT);
    m_spin = new wxSpinButton(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_VERTICAL);

    m_text->SetSizeHints(wxDefaultCoord, wxDefaultCoord);
    m_text->SetToolTip(GetToolTipText());

    m_spin->SetToolTip(GetToolTipText());
    m_spin->SetSizeHints(wxDefaultCoord, wxDefaultCoord);
    m_spin->SetRange(-32000, 32000);

    DoSetValue(m_value);

    wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(m_text, 1, wxEXPAND);
    sizer->Add(m_spin);
    SetSizerAndFit(sizer);

    SetInitialSize(size);
    Move(pos);

    m_text->Bind(wxEVT_COMMAND_TEXT_ENTER, &SpinControl::OnTextEnter, this);
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

void SpinControl::SetDigits(unsigned int digits) {
    if (digits == m_digits)
        return;
    m_digits = digits;
    m_format.Printf("%6.%%0uf", m_digits);
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

bool SpinControl::InRange(double value) {
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
    
    wxString str(wxString::Format(m_format.c_str(), value));
    if (value == m_value && str == m_text->GetValue())
        return false;
    
    str.ToDouble(&m_value);
    m_text->SetValue(str);
    m_text->SetInsertionPointEnd();
    m_text->DiscardEdits();
    return true;
}

void SpinControl::DoSendEvent(const bool spin, const double value) {
    SpinControlEvent event(EVT_SPINCONTROL_EVENT, GetId(), spin, value);
    event.SetEventObject( this );
    GetEventHandler()->ProcessEvent( event );
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

void SpinControl::OnTextEnter(wxCommandEvent& event) {
    if (SyncFromText())
        DoSendEvent(false, GetValue());
}

void SpinControl::OnTextKillFocus(wxFocusEvent& event) {
    if (SyncFromText())
        DoSendEvent(false, GetValue());
    event.Skip();
}

void SpinControl::OnSpinButton(bool up) {
    static const unsigned int SHIFT = 1;
    static const unsigned int ALT = 2;
    static const unsigned int META = 4;
    static const unsigned int CTRLCMD = 8;
    
    double increment = 0.0f;
    
    wxMouseState mouseState = wxGetMouseState();
    unsigned int keys = 0;
    
    if (mouseState.ShiftDown())
        keys |= SHIFT;
    if (mouseState.AltDown())
        keys |= ALT;
    if (mouseState.MetaDown())
        keys |= META;
    if (mouseState.ControlDown() || mouseState.CmdDown())
        keys |= CTRLCMD;
    
    if (keys == 0)
        increment = m_regularIncrement;
    else if (keys == SHIFT)
        increment = m_shiftIncrement;
    else if (keys == CTRLCMD)
        increment = m_ctrlIncrement;
    
    if (!up)
        increment *= -1.0;
    DoSendEvent(true, increment);
}

void SpinControl::OnSpinButtonUp(wxSpinEvent& event) {
    OnSpinButton(true);
}

void SpinControl::OnSpinButtonDown(wxSpinEvent& event) {
    OnSpinButton(false);
}

void SpinControl::OnSetFocus(wxFocusEvent& event) {
    // no idea why this is necessary, but it works
    SetFocus();
}
