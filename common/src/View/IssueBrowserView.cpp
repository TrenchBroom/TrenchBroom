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

#include "IssueBrowserView.h"

#include "Model/Issue.h"
#include "Model/IssueManager.h"
#include "View/MapDocument.h"

#include <wx/settings.h>

namespace TrenchBroom {
    namespace View {
        IssueBrowserView::IssueBrowserView(wxWindow* parent, MapDocumentWPtr document, ControllerWPtr controller) :
        wxListCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_VRULES | wxBORDER_NONE),
        m_document(document),
        m_controller(controller) {
            AppendColumn("Line");
            AppendColumn("Description");
            
            Model::IssueManager& issueManager = lock(m_document)->issueManager();
            issueCountDidChange(issueManager.issueCount());
            
            bindObservers();
            bindEvents();
        }

        IssueBrowserView::~IssueBrowserView() {
            unbindObservers();
        }

        void IssueBrowserView::OnSize(wxSizeEvent& event) {
            const int newWidth = std::max(1, GetClientSize().x - GetColumnWidth(0));
            SetColumnWidth(1, newWidth);
            event.Skip();
        }

        wxString IssueBrowserView::OnGetItemText(const long item, const long column) const {
            Model::IssueManager& issueManager = lock(m_document)->issueManager();
            assert(item >= 0 && static_cast<size_t>(item) < issueManager.issueCount());
            assert(column >= 0 && column < 2);

            Model::Issue* issue = issueManager.issues()[static_cast<size_t>(item)];
            if (column == 0) {
                wxString result;
                result << issue->filePosition();
                return result;
            } else {
                return issue->description();
            }
        }

        void IssueBrowserView::bindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                Model::IssueManager& issueManager = document->issueManager();
                issueManager.issueCountDidChangeNotifier.addObserver(this, &IssueBrowserView::issueCountDidChange);
            }
        }
        
        void IssueBrowserView::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                Model::IssueManager& issueManager = document->issueManager();
                issueManager.issueCountDidChangeNotifier.removeObserver(this, &IssueBrowserView::issueCountDidChange);
            }
        }
        
        void IssueBrowserView::issueCountDidChange(const size_t count) {
            SetItemCount(static_cast<long>(count));
        }

        void IssueBrowserView::bindEvents() {
            Bind(wxEVT_SIZE, &IssueBrowserView::OnSize, this);
        }
    }
}
