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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "IssueBrowserView.h"

#include "Model/CollectMatchingIssuesVisitor.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/World.h"
#include "View/MapDocument.h"
#include "View/wxUtils.h"

#include <QHBoxLayout>
#include <QTableView>
#include <QMenu>
#include <QHeaderView>

namespace TrenchBroom {
    namespace View {
        IssueBrowserView::IssueBrowserView(QWidget* parent, MapDocumentWPtr document) :
        QWidget(parent),
        m_document(document),
        m_hiddenGenerators(0),
        m_showHiddenIssues(false),
        m_valid(false) {
            createGui();
            bindEvents();
        }

        void IssueBrowserView::createGui() {
            m_tableModel = new IssueBrowserModel(this);

            m_tableView = new QTableView(nullptr);
            m_tableView->setModel(m_tableModel);
            m_tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
            m_tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);

            auto* layout = new QHBoxLayout();
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(m_tableView);
            setLayout(layout);
        }
        
        int IssueBrowserView::hiddenGenerators() const {
            return m_hiddenGenerators;
        }
        
        void IssueBrowserView::setHiddenGenerators(const int hiddenGenerators) {
            if (hiddenGenerators == m_hiddenGenerators)
                return;
            m_hiddenGenerators = hiddenGenerators;
            invalidate();
        }

        void IssueBrowserView::setShowHiddenIssues(const bool show) {
            m_showHiddenIssues = show;
            invalidate();
        }

        void IssueBrowserView::reload() {
            invalidate();
        }

        void IssueBrowserView::deselectAll() {
            m_tableView->clearSelection();
        }

        // FIXME: column sizes
//        void IssueBrowserView::OnSize(wxSizeEvent& event) {
//            if (IsBeingDeleted()) return;
//
//            const int newWidth = std::max(1, GetClientSize().x - GetColumnWidth(0));
//            SetColumnWidth(1, newWidth);
//            event.Skip();
//        }
//
        void IssueBrowserView::OnItemRightClick(const QPoint& pos) {
            QModelIndex index = m_tableView->indexAt(pos);

            QMenu *menu = new QMenu(this);
            menu->addAction(new QAction("Action 1", this));
            menu->addAction(new QAction("Action 2", this));
            menu->addAction(new QAction("Action 3", this));
            menu->popup(m_tableView->viewport()->mapToGlobal(pos));


            // FIXME:
#if 0
            if (GetSelectedItemCount() == 0 || event.GetIndex() < 0)
                return;
            
            wxMenu popupMenu;
            popupMenu.Append(ShowIssuesCommandId, "Show");
            popupMenu.Append(HideIssuesCommandId, "Hide");
            popupMenu.Bind(wxEVT_MENU, &IssueBrowserView::OnShowIssues, this, ShowIssuesCommandId);
            popupMenu.Bind(wxEVT_MENU, &IssueBrowserView::OnHideIssues, this, HideIssuesCommandId);
            
            const Model::IssueQuickFixList quickFixes = collectQuickFixes(getSelection());
            if (!quickFixes.empty()) {
                wxMenu* quickFixMenu = new wxMenu();
                
                for (size_t i = 0; i < quickFixes.size(); ++i) {
                    Model::IssueQuickFix* quickFix = quickFixes[i];
                    const int quickFixId = FixObjectsBaseId + static_cast<int>(i);
                    quickFixMenu->Append(quickFixId, quickFix->description());
                    
                    wxVariant* data = new wxVariant(reinterpret_cast<void*>(quickFix));

#ifdef _WIN32
                    popupMenu.Bind(wxEVT_MENU, &IssueBrowserView::OnApplyQuickFix, this, quickFixId, quickFixId, data);
#else
                    quickFixMenu->Bind(wxEVT_MENU, &IssueBrowserView::OnApplyQuickFix, this, quickFixId, quickFixId, data);
#endif
                }
                
                popupMenu.AppendSeparator();
                popupMenu.AppendSubMenu(quickFixMenu, "Fix");
            }

            PopupMenu(&popupMenu);
#endif
        }
        
        void IssueBrowserView::OnItemSelectionChanged() {
            updateSelection();
        }

        void IssueBrowserView::OnShowIssues() {
            setIssueVisibility(true);
        }
        
        void IssueBrowserView::OnHideIssues() {
            setIssueVisibility(false);
        }
        
        class IssueBrowserView::IssueVisible {
            int m_hiddenTypes;
            bool m_showHiddenIssues;
        public:
            IssueVisible(const int hiddenTypes, const bool showHiddenIssues) :
            m_hiddenTypes(hiddenTypes),
            m_showHiddenIssues(showHiddenIssues) {}
            
            bool operator()(const Model::Issue* issue) const {
                return m_showHiddenIssues || (!issue->hidden() && (issue->type() & m_hiddenTypes) == 0);
            }
        };
        
        class IssueBrowserView::IssueCmp {
        public:
            bool operator()(const Model::Issue* lhs, const Model::Issue* rhs) const {
                return lhs->seqId() > rhs->seqId();
            }
        };
        
        void IssueBrowserView::updateSelection() {
            MapDocumentSPtr document = lock(m_document);
            const IndexList selection = getSelection();
            
            Model::NodeList nodes;
            for (size_t i = 0; i < selection.size(); ++i) {
                Model::Issue* issue = m_tableModel->issues().at(selection[i]);
                if (!issue->addSelectableNodes(document->editorContext(), nodes)) {
                    nodes.clear();
                    break;
                }
            }
            
            document->deselectAll();
            document->select(nodes);
        }

        void IssueBrowserView::updateIssues() {
            MapDocumentSPtr document = lock(m_document);
            Model::World* world = document->world();
            if (world != nullptr) {
                const Model::IssueGeneratorList& issueGenerators = world->registeredIssueGenerators();
                Model::CollectMatchingIssuesVisitor<IssueVisible> visitor(issueGenerators, IssueVisible(m_hiddenGenerators, m_showHiddenIssues));
                world->acceptAndRecurse(visitor);

                Model::IssueList issues = visitor.issues();
                VectorUtils::sort(issues, IssueCmp());
                m_tableModel->setIssues(std::move(issues));
            }
        }

        void IssueBrowserView::OnApplyQuickFix() {
            // FIXME:

//            const wxVariant* data = static_cast<wxVariant*>(event.GetEventUserData());
//            ensure(data != nullptr, "data is null");
//
//            const Model::IssueQuickFix* quickFix = reinterpret_cast<const Model::IssueQuickFix*>(data->GetVoidPtr());
//            ensure(quickFix != nullptr, "quickFix is null");
//
//            MapDocumentSPtr document = lock(m_document);
//            const Model::IssueList issues = collectIssues(getSelection());
//
//            const Transaction transaction(document, "Apply Quick Fix (" + quickFix->description() + ")");
//            updateSelection();
//            quickFix->apply(document.get(), issues);
        }
        
        Model::IssueList IssueBrowserView::collectIssues(const IndexList& indices) const {
            Model::IssueList result;
            for (size_t index : indices)
                result.push_back(m_tableModel->issues().at(index));
            return result;
        }

        Model::IssueQuickFixList IssueBrowserView::collectQuickFixes(const IndexList& indices) const {
            if (indices.empty())
                return Model::IssueQuickFixList(0);
            
            Model::IssueType issueTypes = ~0;
            for (size_t index : indices) {
                const Model::Issue* issue = m_tableModel->issues().at(index);
                issueTypes &= issue->type();
            }
            
            MapDocumentSPtr document = lock(m_document);
            const Model::World* world = document->world();
            return world->quickFixes(issueTypes);
        }
        
        Model::IssueType IssueBrowserView::issueTypeMask() const {
            Model::IssueType result = ~static_cast<Model::IssueType>(0);
            for (size_t index : getSelection()) {
                Model::Issue* issue = m_tableModel->issues().at(index);
                result &= issue->type();
            }
            return result;
        }

        void IssueBrowserView::setIssueVisibility(const bool show) {
            MapDocumentSPtr document = lock(m_document);
            for (size_t index : getSelection()) {
                Model::Issue* issue = m_tableModel->issues().at(index);
                document->setIssueHidden(issue, !show);
            }

            invalidate();
        }
        
        IssueBrowserView::IndexList IssueBrowserView::getSelection() const {
            // FIXME:
            return {}; //getListCtrlSelection(this);
        }
        
//        wxListItemAttr* IssueBrowserView::OnGetItemAttr(const long item) const {
//            assert(item >= 0 && static_cast<size_t>(item) < m_issues.size());
//
//            static wxListItemAttr attr;
//
//            Model::Issue* issue = m_issues[static_cast<size_t>(item)];
//            if (issue->hidden()) {
//                attr.SetFont(GetFont().Italic());
//                return &attr;
//            }
//
//            return nullptr;
//        }

        
        void IssueBrowserView::bindEvents() {
//            Bind(wxEVT_SIZE, &IssueBrowserView::OnSize, this);
//            Bind(wxEVT_LIST_ITEM_RIGHT_CLICK, &IssueBrowserView::OnItemRightClick, this);
//            Bind(wxEVT_LIST_ITEM_SELECTED, &IssueBrowserView::OnItemSelectionChanged, this);
//            Bind(wxEVT_LIST_ITEM_DESELECTED, &IssueBrowserView::OnItemSelectionChanged, this);
//            Bind(wxEVT_IDLE, &IssueBrowserView::OnIdle, this);

            m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(m_tableView, &QWidget::customContextMenuRequested, this, &IssueBrowserView::OnItemRightClick);

        }

//        void IssueBrowserView::OnIdle() {
//            validate();
//        }

        void IssueBrowserView::invalidate() {
            m_valid = false;

            QMetaObject::invokeMethod(this, "validate", Qt::QueuedConnection);
        }
        
        void IssueBrowserView::validate() {
            if (!m_valid) {
                m_valid = true;
                
                updateIssues();
            }
        }

        // IssueBrowserModel

        IssueBrowserModel::IssueBrowserModel(QObject* parent)
        : QAbstractTableModel(parent),
          m_issues() {}

        void IssueBrowserModel::setIssues(Model::IssueList issues) {
            beginResetModel();
            m_issues = std::move(issues);
            endResetModel();
        }

        const Model::IssueList& IssueBrowserModel::issues() {
            return m_issues;
        }

        int IssueBrowserModel::rowCount(const QModelIndex& parent) const {
            if (parent.isValid()) {
                return 0;
            }
            return static_cast<int>(m_issues.size());
        }

        int IssueBrowserModel::columnCount(const QModelIndex& parent) const {
            if (parent.isValid()) {
                return 0;
            }
            return 2;
        }

        QVariant IssueBrowserModel::data(const QModelIndex& index, int role) const {
            if (role != Qt::DisplayRole
                || !index.isValid()
                || index.row() < 0
                || index.row() >= static_cast<int>(m_issues.size())
                || index.column() < 0
                || index.column() >= 2) {
                return QVariant();
            }

            const Model::Issue* issue = m_issues.at(static_cast<size_t>(index.row()));
            if (index.column() == 0) {
                return QVariant::fromValue<size_t>(issue->lineNumber());
            } else {
                return QVariant(QString::fromStdString(issue->description()));
            }
        }

        QVariant IssueBrowserModel::headerData(int section, Qt::Orientation orientation, int role) const {
            if (role != Qt::DisplayRole) {
                return QVariant();
            }

            if (orientation == Qt::Horizontal) {
                if (section == 0) {
                    return QVariant(tr("Line"));
                } else if (section == 1) {
                    return QVariant(tr("Description"));
                }
            }
            return QVariant();
        }
    }
}
