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

#include "IssueBrowser.h"

#include "Macros.h"
#include "Model/Issue.h"
#include "Model/IssueGenerator.h"
#include "Model/IssueManager.h"
#include "View/ControllerFacade.h"
#include "View/FlagChangedCommand.h"
#include "View/FlagsPopupEditor.h"
#include "View/MapDocument.h"
#include "View/ViewConstants.h"

#include <wx/checkbox.h>
#include <wx/dataview.h>
#include <wx/menu.h>
#include <wx/settings.h>
#include <wx/simplebook.h>
#include <wx/sizer.h>
#include <wx/variant.h>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        class IssueBrowserDataModel : public wxDataViewModel {
        private:
            MapDocumentWPtr m_document;
            bool m_showHiddenIssues;
            Model::IssueType m_hiddenGenerators;
        public:
            IssueBrowserDataModel(MapDocumentWPtr document) :
            m_document(document),
            m_showHiddenIssues(false) {
                bindObservers();
            }
            
            ~IssueBrowserDataModel() {
                unbindObservers();
            }
            
            unsigned int GetColumnCount() const {
                return 2;
            }
            
            wxString GetColumnType(unsigned int col) const {
                assert(col < GetColumnCount());
                return "string";
            }
            
            bool IsContainer(const wxDataViewItem& item) const {
                if (!item.IsOk())
                    return true;
                return false;
            }
            
            unsigned int GetChildren(const wxDataViewItem& item, wxDataViewItemArray& children) const {
                if (!item.IsOk()) {
                    if (!expired(m_document)) {
                        MapDocumentSPtr document = lock(m_document);
                        const Model::IssueManager& issueManager = document->issueManager();
                        Model::Issue* issue = issueManager.issues();
                        while (issue != NULL) {
                            if (showIssue(issue))
                                children.Add(wxDataViewItem(reinterpret_cast<void*>(issue)));
                            issue = issue->next();
                        }
                    }
                }
                
                return children.size();
            }
            
            wxDataViewItem GetParent(const wxDataViewItem& item) const {
                return wxDataViewItem(NULL);
            }
            
            void GetValue(wxVariant& result, const wxDataViewItem& item, unsigned int col) const {
                assert(col < GetColumnCount());
                assert(item.IsOk());

                const void* data = item.GetID();
                assert(data != NULL);
                const Model::Issue* issue = reinterpret_cast<const Model::Issue*>(data);
                
                if (col == 0) {
                    if (issue->filePosition() == 0) {
                        result = wxVariant("");
                    } else {
                        result = wxVariant(wxString() << issue->filePosition());
                    }
                } else {
                    result = wxVariant(wxString(issue->description()));
                }
            }
            
            bool SetValue(const wxVariant& value, const wxDataViewItem& item, unsigned int col) {
                assert(col < GetColumnCount());
                return false;
            }
            
            void setShowHiddenIssues(const bool showHiddenIssues) {
                if (showHiddenIssues == m_showHiddenIssues)
                    return;
                m_showHiddenIssues = showHiddenIssues;
                reload();
            }
            
            void setHiddenGenerators(const Model::IssueType hiddenGenerators) {
                if (hiddenGenerators == m_hiddenGenerators)
                    return;
                m_hiddenGenerators = hiddenGenerators;
                reload();
            }
            
            void refreshLineNumbers() {
                if (!expired(m_document)) {
                    MapDocumentSPtr document = lock(m_document);
                    const Model::IssueManager& issueManager = document->issueManager();
                    Model::Issue* issue = issueManager.issues();
                    while (issue != NULL) {
                        if (showIssue(issue))
                            ValueChanged(wxDataViewItem(reinterpret_cast<void*>(issue)), 0);
                        issue = issue->next();
                    }
                }
            }
        private:
            void bindObservers() {
                MapDocumentSPtr document = lock(m_document);
                Model::IssueManager& issueManager = document->issueManager();
                issueManager.issueWasAddedNotifier.addObserver(this, &IssueBrowserDataModel::issueWasAdded);
                issueManager.issueWillBeRemovedNotifier.addObserver(this, &IssueBrowserDataModel::issueWillBeRemoved);
                issueManager.issueIgnoreChangedNotifier.addObserver(this, &IssueBrowserDataModel::issueIgnoreChanged);
                issueManager.issuesClearedNotifier.addObserver(this, &IssueBrowserDataModel::issuesCleared);
            }
            
            void unbindObservers() {
                if (!expired(m_document)) {
                    MapDocumentSPtr document = lock(m_document);
                    Model::IssueManager& issueManager = document->issueManager();
                    issueManager.issueWasAddedNotifier.removeObserver(this, &IssueBrowserDataModel::issueWasAdded);
                    issueManager.issueWillBeRemovedNotifier.removeObserver(this, &IssueBrowserDataModel::issueWillBeRemoved);
                    issueManager.issueIgnoreChangedNotifier.removeObserver(this, &IssueBrowserDataModel::issueIgnoreChanged);
                    issueManager.issuesClearedNotifier.removeObserver(this, &IssueBrowserDataModel::issuesCleared);
                }
            }
            
            void issueWasAdded(Model::Issue* issue) {
                assert(issue != NULL);
                addIssue(issue);
            }
            
            void issueWillBeRemoved(Model::Issue* issue) {
                assert(issue != NULL);
                removeIssue(issue);
            }
            
            void issueIgnoreChanged(Model::Issue* issue) {
                assert(issue != NULL);
                if (issue->isHidden())
                    removeIssue(issue);
                else
                    addIssue(issue);
            }
            
            void issuesCleared() {
                Cleared();
            }
            
            void reload() {
                Cleared();
                
                if (!expired(m_document)) {
                    MapDocumentSPtr document = lock(m_document);
                    const Model::IssueManager& issueManager = document->issueManager();
                    
                    wxDataViewItemArray items;
                    Model::Issue* issue = issueManager.issues();
                    while (issue != NULL) {
                        if (showIssue(issue))
                            items.Add(wxDataViewItem(reinterpret_cast<void*>(issue)));
                        issue = issue->next();
                    }
                    
                    ItemsAdded(wxDataViewItem(NULL), items);
                }
            }

            void addIssue(Model::Issue* issue) {
                if (showIssue(issue)) {
                    const bool success = ItemAdded(wxDataViewItem(NULL), wxDataViewItem(reinterpret_cast<void*>(issue)));
                    _unused(success);
                    assert(success);
                }
            }
            
            void removeIssue(Model::Issue* issue) {
                if (showIssue(issue)) {
                    const bool success = ItemDeleted(wxDataViewItem(NULL), wxDataViewItem(reinterpret_cast<void*>(issue)));
                    _unused(success);
                    assert(success);
                }
            }

            bool showIssue(Model::Issue* issue) const {
                return !issue->hasType(m_hiddenGenerators) && (m_showHiddenIssues || !issue->isHidden());
            }
        };
        
        IssueBrowser::IssueBrowser(wxWindow* parent, wxSimplebook* extraBook, MapDocumentWPtr document, ControllerWPtr controller) :
        wxPanel(parent),
        m_document(document),
        m_controller(controller),
        m_model(NULL),
        m_tree(NULL),
        m_showHiddenIssuesCheckBox(NULL),
        m_filterEditor(NULL) {
            m_model = new IssueBrowserDataModel(m_document);
            m_tree = new wxDataViewCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxDV_HORIZ_RULES | wxDV_MULTIPLE | wxBORDER_SIMPLE);
            m_tree->AssociateModel(m_model);
            m_tree->AppendTextColumn("Line", 0)->SetWidth(80);
            m_tree->AppendTextColumn("Description", 1)->SetWidth(200);
            m_tree->Expand(wxDataViewItem(NULL));
            
            m_tree->Bind(wxEVT_SIZE, &IssueBrowser::OnTreeViewSize, this);
            m_tree->Bind(wxEVT_DATAVIEW_ITEM_CONTEXT_MENU, &IssueBrowser::OnTreeViewContextMenu, this);

            wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
            sizer->Add(m_tree, 1, wxEXPAND);
            SetSizerAndFit(sizer);

            wxPanel* extraPanel = new wxPanel(extraBook);
            m_showHiddenIssuesCheckBox = new wxCheckBox(extraPanel, wxID_ANY, "Show hidden issues");
            m_showHiddenIssuesCheckBox->Bind(wxEVT_CHECKBOX, &IssueBrowser::OnShowHiddenIssuesChanged, this);
            
            m_filterEditor = new FlagsPopupEditor(extraPanel, 1, "Filter", false);
            m_filterEditor->Bind(EVT_FLAG_CHANGED_EVENT,
                                EVT_FLAG_CHANGED_HANDLER(IssueBrowser::OnFilterChanged),
                                this);
            
            wxBoxSizer* extraPanelSizer = new wxBoxSizer(wxHORIZONTAL);
            extraPanelSizer->AddStretchSpacer();
            extraPanelSizer->Add(m_showHiddenIssuesCheckBox, 0, wxALIGN_CENTER_VERTICAL);
            extraPanelSizer->AddSpacer(LayoutConstants::ControlHorizontalMargin);
            extraPanelSizer->Add(m_filterEditor, 0, wxALIGN_RIGHT);
            extraPanel->SetSizer(extraPanelSizer);
            extraBook->AddPage(extraPanel, "");
            
            bindObservers();
        }
        
        IssueBrowser::~IssueBrowser() {
            unbindObservers();
        }

        Model::QuickFix::List collectQuickFixes(const wxDataViewItemArray& selections) {
            assert(!selections.empty());
            const Model::Issue* issue = reinterpret_cast<const Model::Issue*>(selections[0].GetID());
            const Model::IssueType type = issue->type();
            Model::QuickFix::List result = issue->quickFixes();
            
            for (size_t i = 1; i < selections.size(); ++i) {
                issue = reinterpret_cast<const Model::Issue*>(selections[i].GetID());
                if (issue->type() != type)
                    return Model::QuickFix::List(0);
            }
            return result;
        }
        
        void IssueBrowser::OnShowHiddenIssuesChanged(wxCommandEvent& event) {
            m_model->setShowHiddenIssues(m_showHiddenIssuesCheckBox->IsChecked());
        }

        void IssueBrowser::OnFilterChanged(FlagChangedCommand& command) {
            m_model->setHiddenGenerators(~command.flagSetValue());
        }

        void IssueBrowser::OnTreeViewContextMenu(wxDataViewEvent& event) {
            const wxDataViewItem& item = event.GetItem();
            if (!item.IsOk())
                return;
            if (!m_tree->IsSelected(item))
                return;
            
            wxDataViewItemArray selections;
            m_tree->GetSelections(selections);
            assert(!selections.empty());

            wxMenu popupMenu;
            popupMenu.Append(SelectObjectsCommandId, "Select");
            popupMenu.Append(ShowIssuesCommandId, "Show");
            popupMenu.Append(HideIssuesCommandId, "Hide");
            popupMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, &IssueBrowser::OnSelectIssues, this, SelectObjectsCommandId);
            popupMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, &IssueBrowser::OnShowIssues, this, ShowIssuesCommandId);
            popupMenu.Bind(wxEVT_COMMAND_MENU_SELECTED, &IssueBrowser::OnHideIssues, this, HideIssuesCommandId);

            const Model::QuickFix::List quickFixes = collectQuickFixes(selections);
            if (!quickFixes.empty()) {
                wxMenu* quickFixMenu = new wxMenu();
                for (size_t i = 0; i < quickFixes.size(); ++i) {
                    const Model::QuickFix* quickFix = quickFixes[i];
                    quickFixMenu->Append(FixObjectsBaseId + i, quickFix->description());
                }
                popupMenu.AppendSubMenu(quickFixMenu, "Fix");
                quickFixMenu->Bind(wxEVT_COMMAND_MENU_SELECTED, &IssueBrowser::OnApplyQuickFix, this, FixObjectsBaseId, FixObjectsBaseId + quickFixes.size());
            }
            
            PopupMenu(&popupMenu);
        }
        
        void IssueBrowser::OnSelectIssues(wxCommandEvent& event) {
            View::ControllerSPtr controller = lock(m_controller);
            controller->beginUndoableGroup("Select fixable objects");

            wxDataViewItemArray selections;
            m_tree->GetSelections(selections);
            selectIssueObjects(selections, controller);
            
            controller->closeGroup();
        }
        
        void IssueBrowser::OnShowIssues(wxCommandEvent& event) {
            setIssueVisibility(true);
        }
        
        void IssueBrowser::OnHideIssues(wxCommandEvent& event) {
            setIssueVisibility(false);
        }

        void IssueBrowser::OnApplyQuickFix(wxCommandEvent& event) {
            wxDataViewItemArray selections;
            m_tree->GetSelections(selections);
            assert(!selections.empty());
            
            Model::QuickFix::List quickFixes = collectQuickFixes(selections);
            const size_t index = static_cast<size_t>(event.GetId()) - FixObjectsBaseId;
            assert(index < quickFixes.size());
            const Model::QuickFix* quickFix = quickFixes[index];
            
            View::ControllerSPtr controller = lock(m_controller);
            controller->beginUndoableGroup("");

            selectIssueObjects(selections, controller);
            
            for (size_t i = 0; i < selections.size(); ++i) {
                const wxDataViewItem& item = selections[i];
                void* data = item.GetID();
                assert(data != NULL);
                Model::Issue* issue = reinterpret_cast<Model::Issue*>(data);
                issue->applyQuickFix(quickFix, controller);
            }
            
            controller->closeGroup();
            
            m_tree->UnselectAll();
            m_tree->SetSelections(selections);
        }
        
        void IssueBrowser::OnTreeViewSize(wxSizeEvent& event) {
            const int scrollbarWidth = wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
            const int newWidth = std::max(1, m_tree->GetClientSize().x - m_tree->GetColumn(0)->GetWidth() - scrollbarWidth);
            m_tree->GetColumn(1)->SetWidth(newWidth);
            event.Skip();
        }

        void IssueBrowser::selectIssueObjects(const wxDataViewItemArray& selections, View::ControllerSPtr controller) {
            controller->deselectAll();
            
            for (size_t i = 0; i < selections.size(); ++i) {
                const wxDataViewItem& item = selections[i];
                void* data = item.GetID();
                assert(data != NULL);
                Model::Issue* issue = reinterpret_cast<Model::Issue*>(data);
                issue->select(controller);
            }
        }

        void IssueBrowser::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->documentWasSavedNotifier.addObserver(this, &IssueBrowser::documentWasSaved);
            document->documentWasNewedNotifier.addObserver(this, &IssueBrowser::documentWasNewedOrLoaded);
            document->documentWasLoadedNotifier.addObserver(this, &IssueBrowser::documentWasNewedOrLoaded);
        }
        
        void IssueBrowser::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->documentWasSavedNotifier.removeObserver(this, &IssueBrowser::documentWasSaved);
                document->documentWasNewedNotifier.removeObserver(this, &IssueBrowser::documentWasNewedOrLoaded);
                document->documentWasLoadedNotifier.removeObserver(this, &IssueBrowser::documentWasNewedOrLoaded);
            }
        }
        
        void IssueBrowser::documentWasNewedOrLoaded() {
            updateFilterFlags();
        }

        void IssueBrowser::documentWasSaved() {
            m_model->refreshLineNumbers();
        }

        void IssueBrowser::updateFilterFlags() {
            MapDocumentSPtr document = lock(m_document);
            Model::IssueManager& issueManager = document->issueManager();
            
            wxArrayInt flags;
            wxArrayString labels;
            
            const Model::IssueManager::GeneratorList& generators = issueManager.registeredGenerators();
            Model::IssueManager::GeneratorList::const_iterator it, end;
            for (it = generators.begin(), end = generators.end(); it != end; ++it) {
                const Model::IssueGenerator* generator = *it;
                const Model::IssueType flag = generator->type();
                const String& description = generator->description();
                
                flags.push_back(flag);
                labels.push_back(description);
            }
            
            m_filterEditor->setFlags(flags, labels);
            m_model->setHiddenGenerators(issueManager.defaultHiddenGenerators());
            m_filterEditor->setFlagValue(~issueManager.defaultHiddenGenerators());
        }

        void IssueBrowser::setIssueVisibility(const bool show) {
            wxDataViewItemArray selections;
            m_tree->GetSelections(selections);
            
            MapDocumentSPtr document = lock(m_document);
            Model::IssueManager& issueManager = document->issueManager();
            
            for (size_t i = 0; i < selections.size(); ++i) {
                const wxDataViewItem& item = selections[i];
                void* data = item.GetID();
                assert(data != NULL);
                Model::Issue* issue = reinterpret_cast<Model::Issue*>(data);
                issueManager.setIssueHidden(issue, !show);
            }
            
            document->incModificationCount();
            m_tree->UnselectAll();
        }
    }
}
