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

#include "Inspector.h"
#include "Controller/Editor.h"
#include "Model/Map/Map.h"
#include "Model/Map/Face.h"
#include "Model/Selection.h"
#include "Gwen/Controls/Button.h"
#include "Gwen/Controls/ButtonStrip.h"
#include "Gwen/Controls/CheckBox.h"
#include "Gwen/Controls/GroupBox.h"
#include "Gwen/Controls/Label.h"
#include "Gwen/Controls/NumericUpDown.h"
#include "Gwen/Controls/RadioButtonController.h"
#include "Gwen/Controls/ScrollControl.h"
#include "Gwen/Controls/TabControl.h"
#include "Gwen/Controls/TextBox.h"
#include "GUI/SingleTextureControl.h"
#include "GUI/TextureBrowserControl.h"
#include <sstream>

namespace TrenchBroom {
    namespace Gui {
        
        void Inspector::updateNumericControl(Gwen::Controls::NumericUpDown* control, bool disabled, bool multi, float value) {
            if (disabled) {
                control->SetDisabled(true);
                control->SetPlaceholderString("n/a");
                control->SetHasValue(false);
            } else {
                control->SetDisabled(false);
                if (multi) {
                    control->SetHasValue(false);
                    control->SetPlaceholderString("multiple");
                } else {
                    control->SetHasValue(true);
                    control->SetValue(value, false);
                }
            }
        }

        void Inspector::updateTextureControls() {
            Model::Selection& selection = m_editor.map().selection();
            const vector<Model::Face*>& faces = selection.faces();
            if (selection.mode() == Model::TB_SM_FACES) {
                float xOffset, yOffset, xScale, yScale, rotation;
                bool xOffsetMulti, yOffsetMulti, xScaleMulti, yScaleMulti, rotationMulti;
                
                xOffsetMulti = yOffsetMulti = xScaleMulti = yScaleMulti = rotationMulti = false;
                xOffset = static_cast<float>(faces[0]->xOffset());
                yOffset = static_cast<float>(faces[0]->yOffset());
                xScale = faces[0]->xScale();
                yScale = faces[0]->yScale();
                rotation = faces[0]->rotation();
                Model::Assets::Texture* texture = faces[0]->texture();

                for (unsigned int i = 1; i < faces.size(); i++) {
                    if (texture != faces[i]->texture())
                        texture = NULL;
                    xOffsetMulti |= xOffset == static_cast<float>(faces[i]->xOffset());
                    yOffsetMulti |= yOffset == static_cast<float>(faces[i]->yOffset());
                    xScaleMulti |= xScale == faces[i]->xScale();
                    yScaleMulti |= yScale == faces[i]->yScale();
                    rotationMulti |= rotation == faces[i]->rotation();
                }
                
                m_textureView->setTexture(texture);
                m_textureLabel->SetPlaceholderString("multiple");
                m_textureLabel->SetText(texture == NULL ? "" : texture->name);
                updateNumericControl(m_xOffsetControl, false, xOffsetMulti, xOffset);
                updateNumericControl(m_yOffsetControl, false, yOffsetMulti, yOffset);
                updateNumericControl(m_xScaleControl, false, xScaleMulti, xScale);
                updateNumericControl(m_yScaleControl, false, yScaleMulti, yScale);
                updateNumericControl(m_rotationControl, false, rotationMulti, rotation);
            } else {
                updateNumericControl(m_xOffsetControl, true, false, 0);
                updateNumericControl(m_yOffsetControl, true, false, 0);
                updateNumericControl(m_xScaleControl, true, false, 0);
                updateNumericControl(m_yScaleControl, true, false, 0);
                updateNumericControl(m_rotationControl, true, false, 0);
                m_textureView->setTexture(NULL);
                m_textureLabel->SetPlaceholderString("n/a");
                m_textureLabel->SetText("");
            }
        }
        
        void Inspector::selectionChanged(const Model::SelectionEventData& data) {
            updateTextureControls();
        }

        void Inspector::onXOffsetChanged(Gwen::Controls::Base* control) {
            m_editor.map().setXOffset(static_cast<int>(m_xOffsetControl->GetValue()));
        }

        void Inspector::onYOffsetChanged(Gwen::Controls::Base* control) {
            m_editor.map().setYOffset(static_cast<int>(m_yOffsetControl->GetValue()));
        }
        
        void Inspector::onXScaleChanged(Gwen::Controls::Base* control) {
            m_editor.map().setXScale(m_xScaleControl->GetValue());
        }
        
        void Inspector::onYScaleChanged(Gwen::Controls::Base* control) {
            m_editor.map().setYScale(m_yScaleControl->GetValue());
        }
        
        void Inspector::onRotationChanged(Gwen::Controls::Base* control) {
            m_editor.map().setRotation(m_rotationControl->GetValue());
        }
        
        void Inspector::onTextureBrowserSortCriterionChanged(Gwen::Controls::Base* control) {
            Gwen::Controls::ButtonStrip* buttonStrip = static_cast<Gwen::Controls::ButtonStrip*>(control);
            if (buttonStrip->GetSelectedButtonIndex() == 1)
                m_textureBrowser->setSortCriterion(Model::Assets::TB_TS_USAGE);
            else
                m_textureBrowser->setSortCriterion(Model::Assets::TB_TS_NAME);
        }
        
        void Inspector::onTextureBrowserGroupChanged(Gwen::Controls::Base* control) {
            Gwen::Controls::Button* button = static_cast<Gwen::Controls::Button*>(control);
            m_textureBrowser->setGroup(button->GetToggleState());
        }

        void Inspector::onTextureBrowserFilterUsedChanged(Gwen::Controls::Base* control) {
            Gwen::Controls::Button* button = static_cast<Gwen::Controls::Button*>(control);
            m_textureBrowser->setHideUnused(button->GetToggleState());
        }

        void Inspector::onTextureBrowserFilterTextChanged(Gwen::Controls::Base* control) {
            Gwen::Controls::TextBox* textBox = static_cast<Gwen::Controls::TextBox*>(control);
            m_textureBrowser->setFilterText(Gwen::Utility::UnicodeToString(textBox->GetText()));
        }

        Inspector::Inspector(Gwen::Controls::Base* parent, Controller::Editor& editor) : Base(parent), m_editor(editor) {
            SetMargin(Gwen::Margin(5, 5, 5, 5));
            m_sectionTabControl = new Gwen::Controls::TabControl(this);
            m_sectionTabControl->Dock(Gwen::Pos::Fill);
            
            m_sectionTabControl->AddPage("Map");
            m_sectionTabControl->AddPage("Entity");
            m_sectionTabControl->AddPage("Brush");

            Gwen::Controls::Base* facePanel = new Gwen::Controls::Base(m_sectionTabControl);
            Gwen::Controls::GroupBox* facePropertiesBox = new Gwen::Controls::GroupBox(facePanel);
            facePropertiesBox->SetText("Face Properties");
            facePropertiesBox->Dock(Gwen::Pos::Top);
            facePropertiesBox->SetHeight(183);
            facePropertiesBox->SetPadding(Gwen::Padding(10, 7, 10, 10));
            
            m_textureView = new TrenchBroom::Gui::SingleTextureControl(facePropertiesBox);
            m_textureView->SetBounds(0, 0, 134, 134);
            m_textureView->SetPadding(Gwen::Padding(3, 3, 3, 3));
            
            m_textureLabel = new Gwen::Controls::Label(facePropertiesBox);
            m_textureLabel->SetBounds(0, 140, 134, 20);
            m_textureLabel->SetAlignment(Gwen::Pos::CenterH);
            
            Gwen::Controls::Label* xLabel = new Gwen::Controls::Label(facePropertiesBox);
            xLabel->SetBounds(143, 23, 12, 20);
            xLabel->SetText("X");
            xLabel->SetAlignment(Gwen::Pos::Right);
            
            Gwen::Controls::Label* yLabel = new Gwen::Controls::Label(facePropertiesBox);
            yLabel->SetBounds(143, 51, 12, 20);
            yLabel->SetText("Y");
            yLabel->SetAlignment(Gwen::Pos::Right);
            
            Gwen::Controls::Label* offsetLabel = new Gwen::Controls::Label(facePropertiesBox);
            offsetLabel->SetBounds(159, 0, 100, 20);
            offsetLabel->SetText("Offset");
            offsetLabel->SetAlignment(Gwen::Pos::CenterH);
            
            m_xOffsetControl = new Gwen::Controls::NumericUpDown(facePropertiesBox);
            m_xOffsetControl->SetBounds(159, 20, 100, 20);
            m_xOffsetControl->onChanged.Add(this, &Inspector::onXOffsetChanged);
            
            m_yOffsetControl = new Gwen::Controls::NumericUpDown(facePropertiesBox);
            m_yOffsetControl->SetBounds(159, 48, 100, 20);
            m_yOffsetControl->onChanged.Add(this, &Inspector::onYOffsetChanged);

            Gwen::Controls::Label* scaleLabel = new Gwen::Controls::Label(facePropertiesBox);
            scaleLabel->SetBounds(267, 0, 100, 20);
            scaleLabel->SetText("Scale");
            scaleLabel->SetAlignment(Gwen::Pos::CenterH);
            
            m_xScaleControl = new Gwen::Controls::NumericUpDown(facePropertiesBox);
            m_xScaleControl->SetBounds(267, 20, 100, 20);
            m_xScaleControl->SetIncrement(0.1f);
            m_xScaleControl->onChanged.Add(this, &Inspector::onXScaleChanged);
            
            m_yScaleControl = new Gwen::Controls::NumericUpDown(facePropertiesBox);
            m_yScaleControl->SetBounds(267, 48, 100, 20);
            m_yScaleControl->SetIncrement(0.1f);
            m_yScaleControl->onChanged.Add(this, &Inspector::onYScaleChanged);
            
            Gwen::Controls::Label* rotationLabel = new Gwen::Controls::Label(facePropertiesBox);
            rotationLabel->SetBounds(159, 79, 100, 20);
            rotationLabel->SetText("Rotation");
            rotationLabel->SetAlignment(Gwen::Pos::Right);

            m_rotationControl = new Gwen::Controls::NumericUpDown(facePropertiesBox);
            m_rotationControl->SetBounds(267, 76, 100, 20);
            m_rotationControl->onChanged.Add(this, &Inspector::onRotationChanged);
            
            Gwen::Controls::GroupBox* textureBrowserBox = new Gwen::Controls::GroupBox(facePanel);
            textureBrowserBox->SetText("Texture Browser");
            textureBrowserBox->Dock(Gwen::Pos::Fill);
            textureBrowserBox->SetMargin(Gwen::Margin(0, 5, 0, 0));
            textureBrowserBox->SetPadding(Gwen::Padding(10, 7, 10, 10));

            Gwen::Controls::Base* textureBrowserFilterContainer = new Gwen::Controls::Base(textureBrowserBox);
            textureBrowserFilterContainer->SetMargin(Gwen::Margin(0, 0, 0, 5));
            textureBrowserFilterContainer->Dock(Gwen::Pos::Top);

            m_textureBrowser = new TextureBrowserControl(textureBrowserBox, m_editor);
            m_textureBrowser->Dock(Gwen::Pos::Fill);

            Gwen::Controls::Base* textureButtonsContainer = new Gwen::Controls::Base(textureBrowserFilterContainer);
            textureButtonsContainer->Dock(Gwen::Pos::Left);
            
            Gwen::Controls::ButtonStrip* textureOrderStrip = new Gwen::Controls::ButtonStrip(textureButtonsContainer);
            textureOrderStrip->AddButton("Name");
            textureOrderStrip->AddButton("Usage");
            textureOrderStrip->SetPos(0, 0);
            textureOrderStrip->onSelectionChange.Add(this, &Inspector::onTextureBrowserSortCriterionChanged);
            
            Gwen::Controls::Button* textureGroupButton = new Gwen::Controls::Button(textureButtonsContainer);
            textureGroupButton->SetText("Group");
            textureGroupButton->SetIsToggle(true);
            textureGroupButton->SetPos(textureOrderStrip->X() + textureOrderStrip->Width() + 5, 0);
            textureGroupButton->SetWidth(48);
            textureGroupButton->onToggle.Add(this, &Inspector::onTextureBrowserGroupChanged);
                                                                                    
            Gwen::Controls::Button* textureUsageButton = new Gwen::Controls::Button(textureButtonsContainer);
            textureUsageButton->SetText("Used");
            textureUsageButton->SetIsToggle(true);
            textureUsageButton->SetPos(textureGroupButton->X() + textureGroupButton->Width() + 5, 0);
            textureUsageButton->SetWidth(48);
            textureUsageButton->onToggle.Add(this, &Inspector::onTextureBrowserFilterUsedChanged);
            
            textureButtonsContainer->SizeToChildren();
            
            Gwen::Controls::TextBox* textureFilterTextBox = new Gwen::Controls::TextBox(textureBrowserFilterContainer);
            textureFilterTextBox->SetPlaceholderString("Filter");
            textureFilterTextBox->SetMargin(Gwen::Margin(5, 0, 0, 0));
            textureFilterTextBox->Dock(Gwen::Pos::Fill);
            textureFilterTextBox->onTextChanged.Add(this, &Inspector::onTextureBrowserFilterTextChanged);
            
            textureBrowserFilterContainer->SizeToChildren();
            
            m_sectionTabControl->AddPage("Face", facePanel);

            Model::Selection& selection = m_editor.map().selection();
            selection.selectionAdded    += new Model::Selection::SelectionEvent::Listener<Inspector>(this, &Inspector::selectionChanged);
            selection.selectionRemoved  += new Model::Selection::SelectionEvent::Listener<Inspector>(this, &Inspector::selectionChanged);
            
            updateTextureControls();
        }
        
        Inspector::~Inspector() {
            Model::Selection& selection = m_editor.map().selection();
            selection.selectionAdded    -= new Model::Selection::SelectionEvent::Listener<Inspector>(this, &Inspector::selectionChanged);
            selection.selectionRemoved  -= new Model::Selection::SelectionEvent::Listener<Inspector>(this, &Inspector::selectionChanged);
        }
    }
}