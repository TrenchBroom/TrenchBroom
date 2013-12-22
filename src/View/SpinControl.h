#/*
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

#ifndef __TrenchBroom__SpinControl__
#define __TrenchBroom__SpinControl__

#include <wx/control.h>
#include <wx/event.h>
#include <wx/panel.h>

#include <cassert>

class wxSpinButton;
class wxTextCtrl;

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
    unsigned int m_digits;
    wxString m_format;
public:
    SpinControl(wxWindow *parent, wxWindowID id = wxID_ANY, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=0, const wxValidator &validator=wxDefaultValidator, const wxString &name=wxControlNameStr);
    
    double GetValue() const;
    void SetValue(double doubleValue);
    void SetValue(const wxString& textValue);
    void SetRange(double min, double max);
    void SetIncrements(double regularIncrement, double shiftIncrement, double ctrlIncrement);
    void SetDigits(unsigned int digits);
    void SetHint(const wxString& hint);
    bool Enable(bool enable = true);
    void SetFocus();
private:
    wxSize DoGetBestSize() const;

    bool InRange(double value);
    double AdjustToRange(double value);
    bool DoSetValue(double value);
    void DoSendEvent(bool spin, double value);
    bool SyncFromText();
    
    void OnTextEnter(wxCommandEvent& event);
    void OnTextKillFocus(wxFocusEvent& event);
    void OnSpinButton(bool up);
    void OnSpinButtonUp(wxSpinEvent& event);
    void OnSpinButtonDown(wxSpinEvent& event);
    
    void OnSetFocus(wxFocusEvent& event);
};

#define WXDLLIMPEXP_CUSTOM_EVENT

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EXPORTED_EVENT_TYPE(WXDLLIMPEXP_CUSTOM_EVENT, EVT_SPINCONTROL_EVENT, 1)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*spinControlEventFunction)(SpinControlEvent&);

#define EVT_SPINCONTROL_HANDLER(func) \
    (wxObjectEventFunction) \
    (spinControlEventFunction) & func

#define EVT_SPINCONTROL(id,func) \
    DECLARE_EVENT_TABLE_ENTRY(EVT_SPINCONTROL_EVENT, \
        id, \
        wxID_ANY, \
        (wxObjectEventFunction) \
        (spinControlEventFunction) & func, \
        (wxObject*) NULL ),

#endif /* defined(__TrenchBroom__SpinControl__) */
