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

#include "DocManager.h"

IMPLEMENT_DYNAMIC_CLASS(DocManager, wxDocManager)
wxDocument* DocManager::CreateDocument(const wxString& pathOrig, long flags) {
    wxDocument* document = GetCurrentDocument();
    if (!m_useSDI || document == NULL)
        return wxDocManager::CreateDocument(pathOrig, flags);
    
    document->OnSaveModified();
    
    if (flags == wxDOC_NEW) {
        document->OnNewDocument();
        document->UpdateAllViews();
    } else {
        wxString path = pathOrig;   // may be modified below
        
        wxDocTemplate* docTemplate = document->GetDocumentTemplate();
        wxDocTemplate *temp = NULL;
        if (!path.empty())
            temp = SelectDocumentType(&docTemplate, 1);
        else
            temp = SelectDocumentPath(&docTemplate, 1, path, flags);
        if (temp) {
            document->SetFilename(path);
            document->SetDocumentName(temp->GetDocumentName());
            document->SetDocumentTemplate(temp);
            document->OnOpenDocument(path);
            
            if (!temp->FileMatchesTemplate(path))
                AddFileToHistory(path);
        }
    }
    
    document->Modify(false);
    
    return document;
}
