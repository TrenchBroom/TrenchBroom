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

#ifndef TrenchBroom_Inspector_h
#define TrenchBroom_Inspector_h

#include "Gwen/Controls/Base.h"

namespace Gwen {
    namespace Controls {
        class TabControl;
        class NumericUpDown;
        class Label;
    }
    namespace Skin {
        class Base;
    }
}

namespace TrenchBroom {
    namespace Model {
        class SelectionEventData;
    }
    
    namespace Controller {
        class Editor;
    }
    
    namespace Gui {
        class SingleTextureControl;
        class TextureBrowserControl;
        
        class Inspector : public Gwen::Controls::Base {
        private:
            Gwen::Controls::TabControl* m_sectionTabControl;
            
            SingleTextureControl* m_textureView;
            Gwen::Controls::Label* m_textureLabel;
            Gwen::Controls::NumericUpDown* m_xOffsetControl;
            Gwen::Controls::NumericUpDown* m_yOffsetControl;
            Gwen::Controls::NumericUpDown* m_xScaleControl;
            Gwen::Controls::NumericUpDown* m_yScaleControl;
            Gwen::Controls::NumericUpDown* m_rotationControl;
            
            TextureBrowserControl* m_textureBrowser;
            
            Controller::Editor& m_editor;
        protected:
            void updateNumericControl(Gwen::Controls::NumericUpDown* control, bool disabled, bool multi, float value);
            void updateTextureControls();
            void selectionChanged(const Model::SelectionEventData& data);
            void onXOffsetChanged(Gwen::Controls::Base* control);
            void onYOffsetChanged(Gwen::Controls::Base* control);
            void onXScaleChanged(Gwen::Controls::Base* control);
            void onYScaleChanged(Gwen::Controls::Base* control);
            void onRotationChanged(Gwen::Controls::Base* control);
            void onTextureBrowserSortCriterionChanged(Gwen::Controls::Base* control);
            void onTextureBrowserGroupChanged(Gwen::Controls::Base* control);
            void onTextureBrowserFilterUsedChanged(Gwen::Controls::Base* control);
        public:
            Inspector(Gwen::Controls::Base* parent, Controller::Editor& editor);
            virtual ~Inspector();
        };
    }
}

#endif
