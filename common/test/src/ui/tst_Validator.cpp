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
#include "mdl/BrushNode.h"
#include "mdl/EmptyPropertyKeyValidator.h"
#include "mdl/EmptyPropertyValueValidator.h"
#include "mdl/EntityNode.h"
#include "mdl/GroupNode.h"
#include "mdl/Issue.h"
#include "mdl/IssueQuickFix.h"
#include "mdl/LayerNode.h"
#include "mdl/PatchNode.h"
#include "mdl/WorldNode.h"

#include "kdl/overload.h"
#include "kdl/vector_utils.h"

#include "Catch2.h"

namespace tb::ui
{

TEST_CASE_METHOD(MapDocumentTest, "ValidatorTest.emptyProperty")
{
  mdl::EntityNode* entityNode =
    document->createPointEntity(m_pointEntityDef, vm::vec3d{0, 0, 0});

  document->deselectAll();
  document->selectNodes({entityNode});
  document->setProperty("", "");
  REQUIRE(entityNode->entity().hasProperty(""));

  auto validators = std::vector<const mdl::Validator*>{
    new mdl::EmptyPropertyKeyValidator(), new mdl::EmptyPropertyValueValidator()};

  class AcceptAllIssues
  {
  public:
    bool operator()(const mdl::Issue*) const { return true; }
  };

  auto issues = std::vector<const mdl::Issue*>{};
  document->world()->accept(kdl::overload(
    [&](auto&& thisLambda, mdl::WorldNode* w) {
      issues = kdl::vec_concat(std::move(issues), w->issues(validators));
      w->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::LayerNode* l) {
      issues = kdl::vec_concat(std::move(issues), l->issues(validators));
      l->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::GroupNode* g) {
      issues = kdl::vec_concat(std::move(issues), g->issues(validators));
      g->visitChildren(thisLambda);
    },
    [&](auto&& thisLambda, mdl::EntityNode* e) {
      issues = kdl::vec_concat(std::move(issues), e->issues(validators));
      e->visitChildren(thisLambda);
    },
    [&](mdl::BrushNode* b) {
      issues = kdl::vec_concat(std::move(issues), b->issues(validators));
    },
    [&](mdl::PatchNode* p) {
      issues = kdl::vec_concat(std::move(issues), p->issues(validators));
    }));

  REQUIRE(2 == issues.size());

  const mdl::Issue* issue0 = issues.at(0);
  const mdl::Issue* issue1 = issues.at(1);

  // Should be one EmptyPropertyNameIssue and one EmptyPropertyValueIssue
  CHECK((
    (issue0->type() == validators[0]->type() && issue1->type() == validators[1]->type())
    || (issue0->type() == validators[1]->type() && issue1->type() == validators[0]->type())));

  auto fixes = document->world()->quickFixes(issue0->type());
  REQUIRE(1 == fixes.size());

  const auto* quickFix = fixes.at(0);
  quickFix->apply(*document, std::vector<const mdl::Issue*>{issue0});

  // The fix should have deleted the property
  CHECK(!entityNode->entity().hasProperty(""));

  kdl::vec_clear_and_delete(validators);
}

} // namespace tb::ui
