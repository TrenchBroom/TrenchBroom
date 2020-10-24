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

#include "Ensure.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>
#include <kdl/overload.h>
#include <kdl/vector_utils.h>
#include <kdl/vector_set.h>

#include <vector>

#include <QHBoxLayout>
#include <QTableView>
#include <QMenu>
#include <QHeaderView>
#include <QItemSelectionModel>

namespace TrenchBroom {
    namespace View {
        IssueBrowserView::IssueBrowserView(std::weak_ptr<MapDocument> document, QWidget* parent) :
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
            m_tableView->verticalHeader()->setVisible(false);
            m_tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
            m_tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
            m_tableView->horizontalHeader()->setSectionsClickable(false);
            m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

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

        /**
         * Updates the MapDocument selection to match the table view
         */
        void IssueBrowserView::updateSelection() {
            auto document = kdl::mem_lock(m_document);

            std::vector<Model::Node*> nodes;
            for (Model::Issue* issue : collectIssues(getSelection())) {
                if (!issue->addSelectableNodes(document->editorContext(), nodes)) {
                    nodes.clear();
                    break;
                }
            }

            document->deselectAll();
            document->select(nodes);
        }

        void IssueBrowserView::updateIssues() {
            auto document = kdl::mem_lock(m_document);
            if (document->world() != nullptr) {
                const auto& issueGenerators = document->world()->registeredIssueGenerators();
                
                auto issues = std::vector<Model::Issue*>{};
                const auto collectIssues = [&](auto* node) {
                    for (auto* issue : node->issues(issueGenerators)) {
                        if (m_showHiddenIssues || (!issue->hidden() && (issue->type() & m_hiddenGenerators) == 0)) {
                            issues.push_back(issue);
                        }
                    }
                };

                document->world()->acceptLambda(kdl::overload(
                    [&](auto&& thisLambda, Model::WorldNode* world)   { collectIssues(world); world->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, Model::LayerNode* layer)   { collectIssues(layer); layer->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, Model::GroupNode* group)   { collectIssues(group); group->visitChildren(thisLambda); },
                    [&](auto&& thisLambda, Model::EntityNode* entity) { collectIssues(entity); entity->visitChildren(thisLambda); },
                    [&](Model::BrushNode* brush)                      { collectIssues(brush); }
                ));

                kdl::vec_sort(issues, [](const auto* lhs, const auto* rhs) { return lhs->seqId() > rhs->seqId(); });
                m_tableModel->setIssues(std::move(issues));
            }
        }

        void IssueBrowserView::applyQuickFix(const Model::IssueQuickFix* quickFix) {
            ensure(quickFix != nullptr, "quickFix is null");

            auto document = kdl::mem_lock(m_document);
            const std::vector<Model::Issue*> issues = collectIssues(getSelection());

            const Transaction transaction(document, "Apply Quick Fix (" + quickFix->description() + ")");
            updateSelection();
            quickFix->apply(document.get(), issues);
        }

        std::vector<Model::Issue*> IssueBrowserView::collectIssues(const QList<QModelIndex>& indices) const {
            // Use a vector_set to filter out duplicates.
            // The QModelIndex list returned by getSelection() contains duplicates
            // (not sure why, current row and selected row?)
            kdl::vector_set<Model::Issue*> result;
            result.reserve(static_cast<size_t>(indices.size()));
            for (QModelIndex index : indices) {
                if (index.isValid()) {
                    const auto row = static_cast<size_t>(index.row());
                    result.insert(m_tableModel->issues().at(row));
                }
            }
            return result.release_data();
        }

        std::vector<Model::IssueQuickFix*> IssueBrowserView::collectQuickFixes(const QList<QModelIndex>& indices) const {
            if (indices.empty()) {
                return {};
            }

            Model::IssueType issueTypes = ~static_cast<Model::IssueType>(0);
            for (QModelIndex index : indices) {
                if (!index.isValid()) {
                    continue;
                }
                const Model::Issue* issue = m_tableModel->issues().at(static_cast<size_t>(index.row()));
                issueTypes &= issue->type();
            }

            auto document = kdl::mem_lock(m_document);
            const Model::WorldNode* world = document->world();
            return world->quickFixes(issueTypes);
        }

        Model::IssueType IssueBrowserView::issueTypeMask() const {
            Model::IssueType result = ~static_cast<Model::IssueType>(0);
            for (Model::Issue* issue : collectIssues(getSelection())) {
                result &= issue->type();
            }
            return result;
        }

        void IssueBrowserView::setIssueVisibility(const bool show) {
            auto document = kdl::mem_lock(m_document);
            for (Model::Issue* issue : collectIssues(getSelection())) {
                document->setIssueHidden(issue, !show);
            }

            invalidate();
        }

        QList<QModelIndex> IssueBrowserView::getSelection() const {
            return m_tableView->selectionModel()->selectedIndexes();
        }

        void IssueBrowserView::bindEvents() {
            m_tableView->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(m_tableView, &QWidget::customContextMenuRequested, this, &IssueBrowserView::itemRightClicked);

            connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
                &IssueBrowserView::itemSelectionChanged);
        }

        void IssueBrowserView::itemRightClicked(const QPoint& pos) {
            const QList<QModelIndex> selectedIndexes = m_tableView->selectionModel()->selectedIndexes();
            if (selectedIndexes.empty()) {
                return;
            }

            auto* popupMenu = new QMenu(this);
            popupMenu->addAction(tr("Show"), this, &IssueBrowserView::showIssues);
            popupMenu->addAction(tr("Hide"), this, &IssueBrowserView::hideIssues);

            const std::vector<Model::IssueQuickFix*> quickFixes = collectQuickFixes(selectedIndexes);
            if (!quickFixes.empty()) {
                auto* quickFixMenu = new QMenu();
                quickFixMenu->setTitle(tr("Fix"));

                for (Model::IssueQuickFix* quickFix : quickFixes) {
                    quickFixMenu->addAction(QString::fromStdString(quickFix->description()), this, [=]() {
                        this->applyQuickFix(quickFix);
                    });
                }

                popupMenu->addSeparator();
                popupMenu->addMenu(quickFixMenu);
            }

            // `pos` is in m_tableView->viewport() coordinates as per: http://doc.qt.io/qt-5/qwidget.html#customContextMenuRequested
            popupMenu->popup(m_tableView->viewport()->mapToGlobal(pos));
        }

        void IssueBrowserView::itemSelectionChanged() {
            updateSelection();
        }

        void IssueBrowserView::showIssues() {
            setIssueVisibility(true);
        }

        void IssueBrowserView::hideIssues() {
            setIssueVisibility(false);
        }

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

        void IssueBrowserModel::setIssues(std::vector<Model::Issue*> issues) {
            beginResetModel();
            m_issues = std::move(issues);
            endResetModel();
        }

        const std::vector<Model::Issue*>& IssueBrowserModel::issues() {
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
            if (!index.isValid()
                || index.row() < 0
                || index.row() >= static_cast<int>(m_issues.size())
                || index.column() < 0
                || index.column() >= 2) {
                return QVariant();
            }

            const Model::Issue* issue = m_issues.at(static_cast<size_t>(index.row()));

            if (role == Qt::DisplayRole) {
                if (index.column() == 0) {
                    if (issue->lineNumber() > 0) {
                        return QVariant::fromValue<size_t>(issue->lineNumber());
                    }
                } else {
                    return QVariant(QString::fromStdString(issue->description()));
                }
            } else if (role == Qt::FontRole) {
                if (issue->hidden()) {
                    // hidden issues are italic
                    QFont italicFont;
                    italicFont.setItalic(true);
                    return QVariant(italicFont);
                }
                return QVariant();
            }

            return QVariant();
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
