/*
 Copyright (C) 2010 Kristian Duske

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

#pragma once

#include <QWidget>

#include "NotifierConnection.h"

#include <memory>
#include <string>
#include <vector>

class QTextEdit;
class QSplitter;

namespace tb
{
namespace mdl
{
struct EntityDefinition;
struct PropertyDefinition;
} // namespace mdl

namespace mdl
{
class Node;
}

namespace ui
{
class EntityPropertyGrid;
class MapDocument;
class Selection;
class SmartPropertyEditorManager;

/**
 * Panel containing the EntityPropertyGrid (the key/value editor table),
 * smart editor, and documentation text view.
 */
class EntityPropertyEditor : public QWidget
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;
  QSplitter* m_splitter = nullptr;
  EntityPropertyGrid* m_propertyGrid = nullptr;
  SmartPropertyEditorManager* m_smartEditorManager = nullptr;
  QTextEdit* m_documentationText = nullptr;
  const mdl::EntityDefinition* m_currentDefinition = nullptr;

  NotifierConnection m_notifierConnection;

public:
  explicit EntityPropertyEditor(
    std::weak_ptr<MapDocument> document, QWidget* parent = nullptr);
  ~EntityPropertyEditor() override;

private:
  void OnCurrentRowChanged();

  void connectObservers();

  void selectionDidChange(const Selection& selection);
  void nodesDidChange(const std::vector<mdl::Node*>& nodes);

  void updateIfSelectedEntityDefinitionChanged();
  void updateDocumentationAndSmartEditor();

  /**
   * Returns a description of the options for ChoicePropertyOption and
   * FlagsPropertyDefinition, other subclasses return an empty string.
   */
  static QString optionDescriptions(const mdl::PropertyDefinition& definition);

  void updateDocumentation(const std::string& propertyKey);
  void createGui(std::weak_ptr<MapDocument> document);

  void updateMinimumSize();
};
} // namespace ui
} // namespace tb
