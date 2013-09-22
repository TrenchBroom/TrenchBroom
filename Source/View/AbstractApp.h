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

#ifndef __TrenchBroom__AbstractApp__
#define __TrenchBroom__AbstractApp__

#include "Utility/ExecutableEvent.h"
#include "Utility/Preferences.h"

#include <wx/wx.h>

#include <cassert>

class wxCommandEvent;
class wxDocManager;
class wxEvtHandler;
class wxExtHelpController;
class wxMenu;
class wxMenuBar;
class wxView;
class DocManager;

namespace TrenchBroom {
    namespace View {
        class KeyboardShortcut;
        class PreferencesFrame;
    }
}

class AbstractApp : public wxApp {
protected:
    class DefaultMenuSelector : public TrenchBroom::Preferences::MultiMenuSelector {
    public:
        const TrenchBroom::Preferences::Menu* select(const TrenchBroom::Preferences::MultiMenu& multiMenu) const {
            return NULL;
        }
    };
    
	DocManager* m_docManager;
    TrenchBroom::View::PreferencesFrame* m_preferencesFrame;
    wxExtHelpController* m_helpController;
    wxLongLong m_lastActivationEvent;

    wxMenu* buildMenu(const TrenchBroom::Preferences::Menu& menu, const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused);
    
    
    virtual wxMenu* CreateFileMenu(const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused);
    virtual wxMenu* CreateEditMenu(const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused);
    virtual wxMenu* CreateViewMenu(const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused);
    virtual wxMenu* CreateHelpMenu(const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused);
public:
    inline void setPreferencesFrame(TrenchBroom::View::PreferencesFrame* preferencesFrame) {
        assert(m_preferencesFrame == NULL || preferencesFrame == NULL);
        m_preferencesFrame = preferencesFrame;
    }
    
    inline TrenchBroom::View::PreferencesFrame* preferencesFrame() const {
        return m_preferencesFrame;
    }
    
    virtual wxMenuBar* CreateMenuBar(const TrenchBroom::Preferences::MultiMenuSelector& selector, wxEvtHandler* eventHandler, bool mapViewFocused);
    void DetachFileHistoryMenu(wxMenuBar* menuBar);

    void UpdateAllViews(wxView* sender = NULL, wxObject* hint = NULL);

	virtual bool OnInit();
    virtual int OnExit();
    void OnUnhandledException();

    virtual void OnOpenAbout(wxCommandEvent& event);
    virtual void OnOpenPreferences(wxCommandEvent& event);
    virtual void OnFileNew(wxCommandEvent& event);
    virtual void OnFileOpen(wxCommandEvent& event);
    virtual void OnFileSave(wxCommandEvent& event);
    virtual void OnFileSaveAs(wxCommandEvent& event);
    virtual void OnFileClose(wxCommandEvent& event);
    virtual void OnHelpShowHelp(wxCommandEvent& event);

    void OnUpdateMenuItem(wxUpdateUIEvent& event);
    
    void OnExecutableEvent(TrenchBroom::ExecutableEvent& event);
    
    int FilterEvent(wxEvent& event);
    
    DECLARE_EVENT_TABLE();
};

#endif /* defined(__TrenchBroom__AbstractApp__) */
