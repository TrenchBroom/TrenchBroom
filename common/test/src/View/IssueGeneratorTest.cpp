/*
 Copyright (C) 2021 Kristian Duske
 Copyright (C) 2021 Eric Wasylishen

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

#include "MapDocumentTest.h"

#include "Model/BrushNode.h"
#include "Model/EmptyPropertyKeyIssueGenerator.h"
#include "Model/EmptyPropertyValueIssueGenerator.h"
#include "Model/EntityNode.h"
#include "Model/GroupNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/LayerNode.h"
#include "Model/PatchNode.h"
#include "Model/WorldNode.h"

#include <kdl/overload.h>
#include <kdl/vector_utils.h>

#include "Catch2.h"

namespace TrenchBroom {
namespace View {
TEST_CASE_METHOD(MapDocumentTest, "IssueGeneratorTest.emptyProperty") {
  Model::EntityNode* entityNode = document->createPointEntity(m_pointEntityDef, vm::vec3::zero());

  document->deselectAll();
  document->selectNode(entityNode);
  document->setProperty("", "");
  REQUIRE(entityNode->entity().hasProperty(""));

  auto issueGenerators = std::vector<Model::IssueGenerator*>{
    new Model::EmptyPropertyKeyIssueGenerator(), new Model::EmptyPropertyValueIssueGenerator()};

  class AcceptAllIssues {
  public:
    bool operator()(const Model::Issue*) const { return true; }
  };

  auto issues = std::vector<Model::Issue*>{};
  document->world()->accept(kdl::overload(
    [&](auto&& thisLambda, Model::WorldNode* w) {
      issues = kdl::vec_concat(std::move(issues), w->issues(issueGenerators));
      w->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::LayerNode* l) {
      issues = kdl::vec_concat(std::move(issues), l->issues(issueGenerators));
      l->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::GroupNode* g) {
      issues = kdl::vec_concat(std::move(issues), g->issues(issueGenerators));
      g->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, Model::EntityNode* e) {
      issues = kdl::vec_concat(std::move(issues), e->issues(issueGenerators));
      e->visitChildren(thisLambda);
    },
    [&](Model::BrushNode* b) {
      issues = kdl::vec_concat(std::move(issues), b->issues(issueGenerators));
    },
    [&](Model::PatchNode* p) {
      issues = kdl::vec_concat(std::move(issues), p->issues(issueGenerators));
    }));

  REQUIRE(2 == issues.size());

  Model::Issue* issue0 = issues.at(0);
  Model::Issue* issue1 = issues.at(1);

  // Should be one EmptyPropertyNameIssue and one EmptyPropertyValueIssue
  CHECK(
    ((issue0->type() == issueGenerators[0]->type() &&
      issue1->type() == issueGenerators[1]->type()) ||
     (issue0->type() == issueGenerators[1]->type() &&
      issue1->type() == issueGenerators[0]->type())));

  std::vector<Model::IssueQuickFix*> fixes = document->world()->quickFixes(issue0->type());
  REQUIRE(1 == fixes.size());

  Model::IssueQuickFix* quickFix = fixes.at(0);
  quickFix->apply(document.get(), std::vector<Model::Issue*>{issue0});

  // The fix should have deleted the property
  CHECK(!entityNode->entity().hasProperty(""));

  kdl::vec_clear_and_delete(issueGenerators);
}
} // namespace View
} // namespace TrenchBroom
