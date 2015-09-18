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

#ifndef TrenchBroom_ViewUtils
#define TrenchBroom_ViewUtils

#include "View/ViewTypes.h"
#include "StringUtils.h"

#include <wx/wx.h>

namespace TrenchBroom {
    class Logger;
    
    namespace Assets {
        class EntityModel;
        class EntityModelManager;
        struct ModelSpecification;
    }
    
    namespace View {
        Assets::EntityModel* safeGetModel(Assets::EntityModelManager& manager, const Assets::ModelSpecification& spec, Logger& logger);
        void combineFlags(const size_t numFlags, const int newFlagValue, int& setFlags, int& mixedFlags);
        
        size_t loadDroppedFiles(MapDocumentWPtr document, wxWindow* parent, const wxArrayString& wxPaths);
        
        bool loadTextureCollection(MapDocumentWPtr document, wxWindow* parent, const wxString& wxPath);
        bool containsLoadableTextureCollections(MapDocumentWPtr i_document, const wxArrayString& wxPaths);
        size_t loadTextureCollections(MapDocumentWPtr document, wxWindow* parent, const wxArrayString& wxPaths);
        
        bool loadEntityDefinitionFile(MapDocumentWPtr document, wxWindow* parent, const wxString& wxPath);
        bool containsLoadableEntityDefinitionFile(MapDocumentWPtr document, const wxArrayString& wxPaths);
        size_t loadEntityDefinitionFile(MapDocumentWPtr document, wxWindow* parent, const wxArrayString& wxPaths);
        
        String queryGroupName(wxWindow* parent);
    }
}

#endif /* defined(TrenchBroom_ViewUtils) */
