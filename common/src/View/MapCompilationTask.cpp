/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "MapCompilationTask.h"

#include "View/MapCompilationContext.h"

#include <wx/filefn.h>

namespace TrenchBroom {
    namespace View {
        MapCompilationTask::MapCompilationTask() {}
    
        MapCompilationTask::~MapCompilationTask() {}

        bool MapCompilationTask::execute(MapCompilationContext& context) const {
            return doExecute(context);
        }
    
        MapCompilationCopyFiles::MapCompilationCopyFiles(const String& sourceFileSpec, const String& targetFileSpec) :
        m_sourceFileSpec(sourceFileSpec),
        m_targetFileSpec(targetFileSpec) {}
    
        bool MapCompilationCopyFiles::doExecute(MapCompilationContext& context) const {
            const IO::Path sourceFilePath = getSourceFilePath(context);
            const IO::Path targetFilePath = getTargetFilePath(context);
            
            
        }

        IO::Path MapCompilationCopyFiles::getSourceFilePath(const MapCompilationContext& context) const {
            return IO::Path(context.translateVariables(m_sourceFileSpec));
        }
        
        IO::Path MapCompilationCopyFiles::getTargetFilePath(const MapCompilationContext& context) const {
            return IO::Path(context.translateVariables(m_targetFileSpec));
        }
        
    }
}
