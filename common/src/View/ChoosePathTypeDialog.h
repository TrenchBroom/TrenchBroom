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

#ifndef TrenchBroom_ChoosePathTypeDialog
#define TrenchBroom_ChoosePathTypeDialog

#include "IO/Path.h"

#include <wx/dialog.h>

class wxRadioButton;
class wxWindow;

namespace TrenchBroom {
    namespace View {
        class ChoosePathTypeDialog : public wxDialog {
        private:
            IO::Path m_absPath;
            IO::Path m_docRelativePath;
            IO::Path m_gameRelativePath;
            IO::Path m_appRelativePath;

            wxRadioButton* m_absRadio;
            wxRadioButton* m_docRelativeRadio;
            wxRadioButton* m_appRelativeRadio;
            wxRadioButton* m_gameRelativeRadio;
        public:
            ChoosePathTypeDialog();
            ChoosePathTypeDialog(wxWindow* parent, const IO::Path& absPath, const IO::Path& docPath, const IO::Path& gamePath);
            bool Create();
            
            const IO::Path& path() const;
            
            DECLARE_DYNAMIC_CLASS(ChoosePathTypeDialog)
        private:
            static IO::Path makeRelativePath(const IO::Path& absPath, const IO::Path& newRootPath);
        };
    }
}

#endif /* defined(TrenchBroom_ChoosePathTypeDialog) */
