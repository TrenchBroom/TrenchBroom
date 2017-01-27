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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "RecentDocumentSelectedCommand.h"

wxDEFINE_EVENT(RECENT_DOCUMENT_SELECTED_EVENT, TrenchBroom::View::RecentDocumentSelectedCommand);

namespace TrenchBroom {
    namespace View {
        wxIMPLEMENT_DYNAMIC_CLASS(RecentDocumentSelectedCommand, wxNotifyEvent)
        RecentDocumentSelectedCommand::RecentDocumentSelectedCommand() :
        wxNotifyEvent(RECENT_DOCUMENT_SELECTED_EVENT, wxID_ANY),
        m_documentPath("") {}
        
        const IO::Path& RecentDocumentSelectedCommand::documentPath() const {
            return m_documentPath;
        }
        
        void RecentDocumentSelectedCommand::setDocumentPath(const IO::Path& documentPath) {
            m_documentPath = documentPath;
        }
        
        wxEvent* RecentDocumentSelectedCommand::Clone() const {
            return new RecentDocumentSelectedCommand(*this);
        }
        
    }
}
