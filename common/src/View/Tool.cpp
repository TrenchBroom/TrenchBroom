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

#include "Tool.h"

#include "IO/ResourceUtils.h"

#include <wx/bookctrl.h>
#include <wx/panel.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        Tool::Tool(const bool initiallyActive) :
        m_active(initiallyActive),
        m_book(NULL),
        m_pageIndex(0) {}

        Tool::~Tool() {}
        
        bool Tool::active() const {
            return m_active;
        }
        
        bool Tool::activate() {
            assert(!active());
            if (doActivate()) {
                m_active = true;
                toolActivatedNotifier(this);
            }
            return m_active;
        }
        
        bool Tool::deactivate() {
            assert(active());
            if (doDeactivate()) {
                m_active = false;
                toolDeactivatedNotifier(this);
            }
            return !m_active;
        }
        
        void Tool::refreshViews() {
            refreshViewsNotifier(this);
        }

        void Tool::createPage(wxBookCtrlBase* book) {
            assert(m_book == NULL);
            
            m_book = book;
            m_pageIndex = m_book->GetPageCount();
            m_book->AddPage(doCreatePage(m_book), "");
        }
        
        void Tool::showPage() {
            m_book->SetSelection(m_pageIndex);
        }

        wxBitmap Tool::icon() const {
            return IO::loadImageResource(doGetIconName());
        }

        bool Tool::doActivate() {
            return true;
        }
        
        bool Tool::doDeactivate() {
            return true;
        }

        wxWindow* Tool::doCreatePage(wxWindow* parent) {
            return new wxPanel(parent);
        }

        String Tool::doGetIconName() const {
            return "NoTool.png";
        }
    }
}
