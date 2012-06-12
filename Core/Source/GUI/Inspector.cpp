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

#include "Gwen/Controls/Button.h"
#include "Gwen/Controls/ButtonStrip.h"
#include "Gwen/Controls/CheckBox.h"
#include "Gwen/Controls/GroupBox.h"
#include "Gwen/Controls/Label.h"
#include "Gwen/Controls/ListBox.h"
#include "Gwen/Controls/NumericUpDown.h"
#include "Gwen/Controls/Properties.h"
#include "Gwen/Controls/Property/Text.h"
#include "Gwen/Controls/RadioButtonController.h"
#include "Gwen/Controls/ScrollControl.h"
#include "Gwen/Controls/Splitter.h"
#include "Gwen/Controls/TabControl.h"
#include "Gwen/Controls/TextBox.h"
#include "Gwen/Events.h"
#include "Gwen/Platform.h"

#include "Controller/Editor.h"
#include "GUI/EntityBrowserControl.h"
#include "GUI/EntityPropertyTableControl.h"
#include "GUI/SingleTextureControl.h"
#include "GUI/TextureBrowserControl.h"
#include "Model/Assets/Texture.h"
#include "Model/Map/Map.h"
#include "Model/Map/Face.h"
#include "Model/Selection.h"
#include "Model/Undo/UndoManager.h"
#include "Utilities/Console.h"

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
            std::vector<Model::Face*> faces = selection.allFaces();
            if (!faces.empty()) {
                float xOffset, yOffset, xScale, yScale, rotation;
                bool xOffsetMulti, yOffsetMulti, xScaleMulti, yScaleMulti, rotationMulti, textureMulti;
                
                xOffsetMulti = yOffsetMulti = xScaleMulti = yScaleMulti = rotationMulti = textureMulti = false;
                xOffset = faces[0]->xOffset;
                yOffset = faces[0]->yOffset;
                xScale = faces[0]->xScale;
                yScale = faces[0]->yScale;
                rotation = faces[0]->rotation;
                std::string textureName = faces[0]->textureName;
                Model::Assets::Texture* texture = faces[0]->texture;

                for (unsigned int i = 1; i < faces.size(); i++) {
                    xOffsetMulti    |= (xOffset != faces[i]->xOffset);
                    yOffsetMulti    |= (yOffset != faces[i]->yOffset);
                    xScaleMulti     |= (xScale != faces[i]->xScale);
                    yScaleMulti     |= (yScale != faces[i]->yScale);
                    rotationMulti   |= (rotation != faces[i]->rotation);
                    textureMulti    |= (textureName.compare(faces[i]->textureName) != 0);
                }
                
                m_textureLabel->SetPlaceholderString("multiple");
                if (textureMulti) {
                    m_textureView->setTexture(NULL);
                    m_textureLabel->SetText("");
                } else {
                    m_textureView->setTexture(texture);
                    m_textureLabel->SetText(textureName);
                }
                updateNumericControl(m_xOffsetControl, false, xOffsetMulti, xOffset);
                updateNumericControl(m_yOffsetControl, false, yOffsetMulti, yOffset);
                updateNumericControl(m_xScaleControl, false, xScaleMulti, xScale);
                updateNumericControl(m_yScaleControl, false, yScaleMulti, yScale);
                updateNumericControl(m_rotationControl, false, rotationMulti, rotation);
                m_resetFaceButton->SetDisabled(false);
            } else {
                updateNumericControl(m_xOffsetControl, true, false, 0);
                updateNumericControl(m_yOffsetControl, true, false, 0);
                updateNumericControl(m_xScaleControl, true, false, 0);
                updateNumericControl(m_yScaleControl, true, false, 0);
                updateNumericControl(m_rotationControl, true, false, 0);
                m_textureView->setTexture(NULL);
                m_textureLabel->SetPlaceholderString("n/a");
                m_textureLabel->SetText("");
                m_resetFaceButton->SetDisabled(true);
            }
        }
        
        void Inspector::updateTextureWadList() {
            m_textureWadList->Clear();
            
            Model::Assets::TextureManager& textureManager = m_editor.textureManager();
            const std::vector<Model::Assets::TextureCollection*> collections = textureManager.collections();
            for (unsigned int i = 0; i < collections.size(); i++)
                m_textureWadList->AddItem(collections[i]->name());
        }

        void Inspector::propertiesDidChange(const std::vector<Model::Entity*>& entities) {
            updateTextureControls();

            Model::Selection& selection = m_editor.map().selection();
            m_propertiesTable->setEntities(selection.entities());
        }
        
        void Inspector::brushesDidChange(const std::vector<Model::Brush*>& brushes) {
            updateTextureControls();
        }

        void Inspector::facesDidChange(const std::vector<Model::Face*>& faces) {
            updateTextureControls();
        }

        void Inspector::selectionChanged(const Model::SelectionEventData& data) {
            updateTextureControls();
            
            Model::Selection& selection = m_editor.map().selection();
            m_propertiesTable->setEntities(selection.entities());
        }

        void Inspector::textureManagerDidChange(Model::Assets::TextureManager& textureManager) {
            updateTextureControls();
            updateTextureWadList();
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
        
        void Inspector::onResetFaceButtonPressed(Gwen::Controls::Base* control) {
            m_editor.map().resetFaces();
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

        void Inspector::onTextureSelected(Gwen::Controls::Base* control) {
            Model::Assets::Texture* texture = m_textureBrowser->selectedTexture();
            if (texture == NULL)
                return;
            
            m_editor.map().setTexture(texture);
            m_editor.map().selection().addTexture(*texture);
        }

        void Inspector::onTextureWadListRowSelected(Gwen::Controls::Base* control) {
            m_removeTextureWadsButton->SetDisabled(m_textureWadList->GetSelectedRows().empty());
        }

        void Inspector::onAddTextureWadButtonPressed(Gwen::Controls::Base* control) {
            Gwen::Platform::FileOpen("Choose Wad File", "", "wad", this, static_cast<Gwen::Event::Handler::FunctionStr>(&Inspector::onTextureWadChosen));
        }
        
        void Inspector::onTextureWadChosen(const Gwen::String& path) {
            m_editor.loadTextureWad(path);
        }

        void Inspector::onRemoveTextureWadButtonPressed(Gwen::Controls::Base* control) {
            Gwen::Controls::ListBox::Rows rows = m_textureWadList->GetSelectedRows();
            for (int i = rows.size() - 1; i >= 0; i--)
                m_editor.textureManager().removeCollection(i);
        }

        Gwen::Controls::Base* Inspector::createEntityInspector() {
            Gwen::Controls::Base* entityPanel = new Gwen::Controls::Base(m_sectionTabControl);
            entityPanel->Dock(Gwen::Pos::Fill);

            Gwen::Controls::Splitter* splitter = new Gwen::Controls::Splitter(entityPanel, false, 250);
            splitter->Dock(Gwen::Pos::Fill);
            
            Gwen::Controls::GroupBox* propertiesBox = new Gwen::Controls::GroupBox(splitter);
            propertiesBox->SetText("Properties");
            propertiesBox->SetPadding(Gwen::Padding(10, 7, 10, 10));
            propertiesBox->SetMargin(Gwen::Margin(0, 0, 0, 2));
            propertiesBox->SetCacheToTexture();
            splitter->SetPanel(0, propertiesBox);
            
            m_propertiesTable = new EntityPropertyTableControl(propertiesBox, m_editor);
            m_propertiesTable->Dock(Gwen::Pos::Fill);
            
            Gwen::Controls::GroupBox* browserBox = new Gwen::Controls::GroupBox(splitter);
            browserBox->SetText("Browser");
            browserBox->SetPadding(Gwen::Padding(10, 7, 10, 10));
            browserBox->SetMargin(Gwen::Margin(0, 2, 0, 0));
            splitter->SetPanel(1, browserBox);
            
            m_entityBrowser = new EntityBrowserControl(browserBox, m_editor);
            m_entityBrowser->Dock(Gwen::Pos::Fill);
            
            return entityPanel;
        }
        
        Gwen::Controls::Base* Inspector::createFaceInspector() {
            Gwen::Controls::Base* facePanel = new Gwen::Controls::Base(m_sectionTabControl);
            
            // Face properties box
            Gwen::Controls::GroupBox* facePropertiesBox = new Gwen::Controls::GroupBox(facePanel);
            facePropertiesBox->SetText("Properties");
            facePropertiesBox->Dock(Gwen::Pos::Top);
            facePropertiesBox->SetHeight(187);
            facePropertiesBox->SetPadding(Gwen::Padding(10, 7, 10, 10));
            facePropertiesBox->SetCacheToTexture();
            
            // single texture view for current selection
            m_textureView = new TrenchBroom::Gui::SingleTextureControl(facePropertiesBox);
            m_textureView->SetBounds(0, 0, 134, 134);
            m_textureView->SetPadding(Gwen::Padding(3, 3, 3, 3));
            
            m_textureLabel = new Gwen::Controls::Label(facePropertiesBox);
            m_textureLabel->SetBounds(0, 140, 134, 25);
            m_textureLabel->SetAlignment(Gwen::Pos::CenterH);
            
            // face properties (offset, scale, rotation)
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
            
            m_resetFaceButton = new Gwen::Controls::Button(facePropertiesBox);
            m_resetFaceButton->SetText("Reset");
            m_resetFaceButton->SetBounds(267, 104, 100, 20);
            m_resetFaceButton->onPress.Add(this, &Inspector::onResetFaceButtonPressed);

            // texture browser
            Gwen::Controls::GroupBox* textureBrowserBox = new Gwen::Controls::GroupBox(facePanel);
            textureBrowserBox->SetText("Texture Browser");
            textureBrowserBox->Dock(Gwen::Pos::Fill);
            textureBrowserBox->SetMargin(Gwen::Margin(0, 5, 0, 0));
            textureBrowserBox->SetPadding(Gwen::Padding(10, 7, 10, 10));
            textureBrowserBox->SetCacheToTexture();
            
            // texture controls container
            Gwen::Controls::Base* textureBrowserFilterContainer = new Gwen::Controls::Base(textureBrowserBox);
            textureBrowserFilterContainer->SetMargin(Gwen::Margin(0, 0, 0, 5));
            textureBrowserFilterContainer->Dock(Gwen::Pos::Top);
            
            // actual texture browser control
            m_textureBrowser = new TextureBrowserControl(textureBrowserBox, m_editor);
            m_textureBrowser->Dock(Gwen::Pos::Fill);
            m_textureBrowser->onTextureSelected.Add(this, &Inspector::onTextureSelected);
            
            // texture controls
            Gwen::Controls::Base* textureButtonsContainer = new Gwen::Controls::Base(textureBrowserFilterContainer);
            textureButtonsContainer->Dock(Gwen::Pos::Left);
            
            // options for ordering the textures
            Gwen::Controls::ButtonStrip* textureOrderStrip = new Gwen::Controls::ButtonStrip(textureButtonsContainer);
            textureOrderStrip->AddButton("Name");
            textureOrderStrip->AddButton("Usage");
            textureOrderStrip->SetPos(0, 0);
            textureOrderStrip->onSelectionChange.Add(this, &Inspector::onTextureBrowserSortCriterionChanged);
            
            // toggle texture grouping
            Gwen::Controls::Button* textureGroupButton = new Gwen::Controls::Button(textureButtonsContainer);
            textureGroupButton->SetText("Group");
            textureGroupButton->SetIsToggle(true);
            textureGroupButton->SetPos(textureOrderStrip->X() + textureOrderStrip->Width() + 5, 0);
            textureGroupButton->SetWidth(48);
            textureGroupButton->onToggle.Add(this, &Inspector::onTextureBrowserGroupChanged);
            
            // toggle whether only used textures are displayed
            Gwen::Controls::Button* textureUsageButton = new Gwen::Controls::Button(textureButtonsContainer);
            textureUsageButton->SetText("Used");
            textureUsageButton->SetIsToggle(true);
            textureUsageButton->SetPos(textureGroupButton->X() + textureGroupButton->Width() + 5, 0);
            textureUsageButton->SetWidth(48);
            textureUsageButton->onToggle.Add(this, &Inspector::onTextureBrowserFilterUsedChanged);
            
            textureButtonsContainer->SizeToChildren();
            
            // text box for name matching
            Gwen::Controls::TextBox* textureFilterTextBox = new Gwen::Controls::TextBox(textureBrowserFilterContainer);
            textureFilterTextBox->SetPlaceholderString("Filter");
            textureFilterTextBox->SetMargin(Gwen::Margin(5, 0, 0, 0));
            textureFilterTextBox->Dock(Gwen::Pos::Fill);
            textureFilterTextBox->onTextChanged.Add(this, &Inspector::onTextureBrowserFilterTextChanged);
            
            textureBrowserFilterContainer->SizeToChildren();
            
            // texture wad list
            Gwen::Controls::Base* textureWadListContainer = new Gwen::Controls::Base(textureBrowserBox);
            textureWadListContainer->SetMargin(Gwen::Margin(0, 5, 0, 0));
            textureWadListContainer->SetHeight(65);
            textureWadListContainer->Dock(Gwen::Pos::Bottom);
            
            m_textureWadList = new Gwen::Controls::ListBox(textureWadListContainer);
            m_textureWadList->Dock(Gwen::Pos::Fill);
            m_textureWadList->SetAllowMultiSelect(true);
            m_textureWadList->onRowSelected.Add(this, &Inspector::onTextureWadListRowSelected);
            
            Gwen::Controls::Base* textureWadListButtonsContainer = new Gwen::Controls::Base(textureWadListContainer);
            textureWadListButtonsContainer->SetMargin(Gwen::Margin(5, 0, 0, 0));
            textureWadListButtonsContainer->SetWidth(20);
            textureWadListButtonsContainer->Dock(Gwen::Pos::Right);
            
            m_addTextureWadButton = new Gwen::Controls::Button(textureWadListButtonsContainer);
            m_addTextureWadButton->SetText("+");
            m_addTextureWadButton->SetSize(16, 20);
            m_addTextureWadButton->Dock(Gwen::Pos::Top);
            m_addTextureWadButton->onPress.Add(this, &Inspector::onAddTextureWadButtonPressed);
            
            m_removeTextureWadsButton = new Gwen::Controls::Button(textureWadListButtonsContainer);
            m_removeTextureWadsButton->SetText("-");
            m_removeTextureWadsButton->SetSize(16, 20);
            m_removeTextureWadsButton->SetMargin(Gwen::Margin(0, 5, 0, 0));
            m_removeTextureWadsButton->Dock(Gwen::Pos::Top);
            m_removeTextureWadsButton->SetDisabled(true);
            m_removeTextureWadsButton->onPress.Add(this, &Inspector::onRemoveTextureWadButtonPressed);
            
            return facePanel;
        }
        
        Inspector::Inspector(Gwen::Controls::Base* parent, Controller::Editor& editor) : Base(parent), m_editor(editor) {
            m_sectionTabControl = new Gwen::Controls::TabControl(this);
            m_sectionTabControl->Dock(Gwen::Pos::Fill);
            
            m_sectionTabControl->AddPage("Map");
            
            Gwen::Controls::Base* entityInspector = createEntityInspector();
            m_sectionTabControl->AddPage("Entity", entityInspector);
            m_sectionTabControl->AddPage("Brush");

            Gwen::Controls::Base* faceInspector = createFaceInspector();
            m_sectionTabControl->AddPage("Face", faceInspector);
            
            Model::Map& map = m_editor.map();
            Model::Selection& selection = map.selection();
            Model::Assets::TextureManager& textureManager = m_editor.textureManager();

            map.propertiesDidChange                 += new Model::Map::EntityEvent::Listener<Inspector>(this, &Inspector::propertiesDidChange);
            map.brushesDidChange                    += new Model::Map::BrushEvent::Listener<Inspector>(this, &Inspector::brushesDidChange);
            map.facesDidChange                      += new Model::Map::FaceEvent::Listener<Inspector>(this, &Inspector::facesDidChange);
            selection.selectionAdded                += new Model::Selection::SelectionEvent::Listener<Inspector>(this, &Inspector::selectionChanged);
            selection.selectionRemoved              += new Model::Selection::SelectionEvent::Listener<Inspector>(this, &Inspector::selectionChanged);
            textureManager.textureManagerDidChange  += new Model::Assets::TextureManager::TextureManagerEvent::Listener<Inspector>(this, &Inspector::textureManagerDidChange);
            
            updateTextureControls();
            updateTextureWadList();
        }
        
        Inspector::~Inspector() {
            Model::Map& map = m_editor.map();
            Model::Selection& selection = map.selection();
            Model::Assets::TextureManager& textureManager = m_editor.textureManager();

            map.propertiesDidChange                 -= new Model::Map::EntityEvent::Listener<Inspector>(this, &Inspector::propertiesDidChange);
            map.brushesDidChange                    -= new Model::Map::BrushEvent::Listener<Inspector>(this, &Inspector::brushesDidChange);
            map.facesDidChange                      -= new Model::Map::FaceEvent::Listener<Inspector>(this, &Inspector::facesDidChange);
            selection.selectionAdded                -= new Model::Selection::SelectionEvent::Listener<Inspector>(this, &Inspector::selectionChanged);
            selection.selectionRemoved              -= new Model::Selection::SelectionEvent::Listener<Inspector>(this, &Inspector::selectionChanged);
            textureManager.textureManagerDidChange  -= new Model::Assets::TextureManager::TextureManagerEvent::Listener<Inspector>(this, &Inspector::textureManagerDidChange);
        }
    }
}