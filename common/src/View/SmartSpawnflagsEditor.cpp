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

#include "SmartSpawnflagsEditor.h"

#include "Assets/EntityDefinition.h"
#include "Assets/AttributeDefinition.h"
#include "Model/AttributableNode.h"
#include "Model/Entity.h"
#include "Model/World.h"
#include "View/FlagsEditor.h"
#include "View/MapDocument.h"
#include "View/ViewUtils.h"

#include <QScrollArea>
#include <QVBoxLayout>

#include <cassert>

namespace TrenchBroom {
    namespace View {
        class SmartSpawnflagsEditor::UpdateSpawnflag : public Model::NodeVisitor {
        private:
            MapDocumentSPtr m_document;
            const Model::AttributeName& m_name;
            size_t m_flagIndex;
            bool m_setFlag;
        public:
            UpdateSpawnflag(MapDocumentSPtr document, const Model::AttributeName& name, const size_t flagIndex, const bool setFlag) :
            m_document(document),
            m_name(name),
            m_flagIndex(flagIndex),
            m_setFlag(setFlag) {}
            
            void doVisit(Model::World* world) override   { m_document->updateSpawnflag(m_name, m_flagIndex, m_setFlag); }
            void doVisit(Model::Layer* layer) override   {}
            void doVisit(Model::Group* group) override   {}
            void doVisit(Model::Entity* entity) override { m_document->updateSpawnflag(m_name, m_flagIndex, m_setFlag); }
            void doVisit(Model::Brush* brush) override   {}
        };
        
        SmartSpawnflagsEditor::SmartSpawnflagsEditor(QObject* parent, View::MapDocumentWPtr document) :
        SmartAttributeEditor(parent, document),
        m_scrolledWindow(nullptr),
        m_flagsEditor(nullptr),
        m_ignoreUpdates(false) {}

        void SmartSpawnflagsEditor::OnFlagChanged(size_t index, int setFlag, int mixedFlag) {
            const Model::AttributableNodeList& toUpdate = attributables();
            if (toUpdate.empty())
                return;

            const bool set = m_flagsEditor->isFlagSet(index);
            
            const TemporarilySetBool ignoreUpdates(m_ignoreUpdates);
            
            const Transaction transaction(document(), "Set Spawnflags");
            UpdateSpawnflag visitor(document(), name(), index, set);
            Model::Node::accept(std::begin(toUpdate), std::end(toUpdate), visitor);
        }
        
        QWidget* SmartSpawnflagsEditor::doCreateVisual(QWidget* parent) {
            assert(m_scrolledWindow == nullptr);
            
            m_scrolledWindow = new QScrollArea(parent);
            //m_scrolledWindow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOX));
            
            m_flagsEditor = new FlagsEditor(nullptr, NumCols);
            connect(m_flagsEditor, &FlagsEditor::flagChanged, this, &SmartSpawnflagsEditor::OnFlagChanged);

            m_scrolledWindow->setWidget(m_flagsEditor);

            return m_scrolledWindow;
        }
        
        void SmartSpawnflagsEditor::doDestroyVisual() {
            ensure(m_scrolledWindow != nullptr, "scrolledWindow is null");
            m_lastScrollPos = m_flagsEditor->pos();
            delete m_scrolledWindow;
            m_scrolledWindow = nullptr;
            m_flagsEditor = nullptr;
        }

        void SmartSpawnflagsEditor::doUpdateVisual(const Model::AttributableNodeList& attributables) {
            assert(!attributables.empty());
            if (m_ignoreUpdates)
                return;
            
            QStringList labels;
            QStringList tooltips;
            getFlags(attributables, labels, tooltips);
            m_flagsEditor->setFlags(labels, tooltips);
            
            int set, mixed;
            getFlagValues(attributables, set, mixed);
            m_flagsEditor->setFlagValue(set, mixed);
            
//            m_scrolledWindow->SetScrollRate(1, m_flagsEditor->lineHeight());
//            m_scrolledWindow->Layout();
//            m_scrolledWindow->FitInside();
//            m_scrolledWindow->Refresh();
            resetScrollPos();
        }
        
        void SmartSpawnflagsEditor::resetScrollPos() {
            // TODO: the y position is not properly set (at least on OS X)
//            int xRate, yRate;
//            m_scrolledWindow->GetScrollPixelsPerUnit(&xRate, &yRate);
//            m_scrolledWindow->Scroll(m_lastScrollPos.x * xRate, m_lastScrollPos.y * yRate);
        }

        void SmartSpawnflagsEditor::getFlags(const Model::AttributableNodeList& attributables, QStringList& labels, QStringList& tooltips) const {
            QStringList defaultLabels;
            
            // Initialize the labels and tooltips.
            for (size_t i = 0; i < NumFlags; ++i) {
                QString defaultLabel;
                defaultLabel = QString::number(1 << i);

                defaultLabels.push_back(defaultLabel);
                labels.push_back(defaultLabel);
                tooltips.push_back("");
            }

            for (size_t i = 0; i < NumFlags; ++i) {
                bool firstPass = true;
                for (const Model::AttributableNode* attributable : attributables) {
                    QString label = defaultLabels[i];
                    QString tooltip = "";

                    const Assets::FlagsAttributeOption* flagDef = Assets::EntityDefinition::safeGetSpawnflagsAttributeOption(attributable->definition(), i);
                    if (flagDef != nullptr) {
                        label = QString::fromStdString(flagDef->shortDescription());
                        tooltip = QString::fromStdString(flagDef->longDescription());
                    }
                    
                    if (firstPass) {
                        labels[i] = label;
                        tooltips[i] = tooltip;
                        firstPass = false;
                    } else {
                        if (labels[i] != label) {
                            labels[i] = defaultLabels[i];
                            tooltips[i] = "";
                        }
                    }
                }
            }
        }

        void SmartSpawnflagsEditor::getFlagValues(const Model::AttributableNodeList& attributables, int& setFlags, int& mixedFlags) const {
            if (attributables.empty()) {
                setFlags = 0;
                mixedFlags = 0;
                return;
            }
            
            Model::AttributableNodeList::const_iterator it = std::begin(attributables);
            Model::AttributableNodeList::const_iterator end = std::end(attributables);
            setFlags = getFlagValue(*it);
            mixedFlags = 0;
            
            while (++it != end)
                combineFlags(NumFlags, getFlagValue(*it), setFlags, mixedFlags);
        }

        int SmartSpawnflagsEditor::getFlagValue(const Model::AttributableNode* attributable) const {
            if (!attributable->hasAttribute(name()))
                return 0;

            const Model::AttributeValue& value = attributable->attribute(name());
            return std::atoi(value.c_str());
        }
    }
}
