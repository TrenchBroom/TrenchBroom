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

#pragma once

#include <QDialog>

#include <memory>
#include <vector>

class QPushButton;

namespace TrenchBroom::Assets
{
class Material;
}

namespace TrenchBroom::Model
{
class BrushFaceHandle;
}

namespace TrenchBroom::View
{

class GLContextManager;
class MapDocument;
class MaterialBrowser;

class ReplaceMaterialDialog : public QDialog
{
  Q_OBJECT
private:
  std::weak_ptr<MapDocument> m_document;

  MaterialBrowser* m_subjectBrowser = nullptr;
  MaterialBrowser* m_replacementBrowser = nullptr;
  QPushButton* m_replaceButton = nullptr;

public:
  ReplaceMaterialDialog(
    std::weak_ptr<MapDocument> document,
    GLContextManager& contextManager,
    QWidget* parent = nullptr);

private:
  void accept() override;
  std::vector<Model::BrushFaceHandle> getApplicableFaces() const;
  void createGui(GLContextManager& contextManager);
private slots:
  void subjectSelected(const Assets::Material* subject);
  void replacementSelected(const Assets::Material* replacement);
  void updateReplaceButton();
};

} // namespace TrenchBroom::View
