//Added by Lws

#include "BoxSelectionTool.h"

#include "render/Camera.h"
#include "ui/DrawShapeTool.h"
#include "ui/Grid.h"
#include "ui/HandleDragTracker.h"
#include "ui/InputState.h"
#include "ui/MapDocument.h"
#include "ui/Transaction.h"
#include "mdl/ModelUtils.h"
#include "mdl/BrushNode.h"
#include "mdl/WorldNode.h"
#include "mdl/LayerNode.h"
#include "mdl/GroupNode.h"
#include "mdl/EntityNode.h"
#include "mdl/PatchNode.h"
#include "render/RenderService.h"
#include "render/RenderContext.h"
#include "render/RenderBatch.h"
#include "Color.h"

#include "kdl/memory_utils.h"
#include "kdl/overload.h"

#include "vm/intersection.h"

namespace tb::ui
{

SelectionBoxRenderer::SelectionBoxRenderer() 
  : m_valid(false) 
{
}
  
void SelectionBoxRenderer::setSelectionBounds(const vm::bbox3d& bounds) {
  m_selectionBounds = bounds;
  m_valid = true;
}

void SelectionBoxRenderer::clear() {
  m_valid = false;
}

void SelectionBoxRenderer::render(render::RenderContext& renderContext, render::RenderBatch& renderBatch) const {
  if (!m_valid) {
    return;
  }
  
  auto renderService = render::RenderService{renderContext, renderBatch};
  
  // Use blue color to show the selection box
  const auto color = Color(0.2f, 0.4f, 1.0f, 0.6f);
  renderService.setForegroundColor(color);
  renderService.setLineWidth(2.0f); // Set the line width
  
  // Get the camera direction to adapt to different views
  const auto& camera = renderContext.camera();
  const auto direction = camera.direction();
  
  // Draw the boundary lines of the box
  const auto min = m_selectionBounds.min;
  const auto max = m_selectionBounds.max;
  
  // Determine the main projection plane, based on the camera direction
  vm::axis::type majorAxis = vm::find_abs_max_component(direction);
  
  if (majorAxis == vm::axis::z) {
    // Top view or bottom view - XY plane
    renderService.renderLine(
      vm::vec3f(min.x(), min.y(), min.z()),
      vm::vec3f(max.x(), min.y(), min.z()));
    renderService.renderLine(
      vm::vec3f(max.x(), min.y(), min.z()),
      vm::vec3f(max.x(), max.y(), min.z()));
    renderService.renderLine(
      vm::vec3f(max.x(), max.y(), min.z()),
      vm::vec3f(min.x(), max.y(), min.z()));
    renderService.renderLine(
      vm::vec3f(min.x(), max.y(), min.z()),
      vm::vec3f(min.x(), min.y(), min.z()));
  } else if (majorAxis == vm::axis::y) {
    // Front view or back view - XZ plane
    renderService.renderLine(
      vm::vec3f(min.x(), min.y(), min.z()),
      vm::vec3f(max.x(), min.y(), min.z()));
    renderService.renderLine(
      vm::vec3f(max.x(), min.y(), min.z()),
      vm::vec3f(max.x(), min.y(), max.z()));
    renderService.renderLine(
      vm::vec3f(max.x(), min.y(), max.z()),
      vm::vec3f(min.x(), min.y(), max.z()));
    renderService.renderLine(
      vm::vec3f(min.x(), min.y(), max.z()),
      vm::vec3f(min.x(), min.y(), min.z()));
  } else {
    // Side view - YZ plane
    renderService.renderLine(
      vm::vec3f(min.x(), min.y(), min.z()),
      vm::vec3f(min.x(), max.y(), min.z()));
    renderService.renderLine(
      vm::vec3f(min.x(), max.y(), min.z()),
      vm::vec3f(min.x(), max.y(), max.z()));
    renderService.renderLine(
      vm::vec3f(min.x(), max.y(), max.z()),
      vm::vec3f(min.x(), min.y(), max.z()));
    renderService.renderLine(
      vm::vec3f(min.x(), min.y(), max.z()),
      vm::vec3f(min.x(), min.y(), min.z()));
  }
}

BoxSelectionDragDelegate::BoxSelectionDragDelegate(
  DrawShapeTool& tool, std::weak_ptr<MapDocument> document)
  : m_tool{tool}
  , m_document{std::move(document)}
{
}

HandlePositionProposer BoxSelectionDragDelegate::start(
  const InputState& inputState,
  const vm::vec3d& initialHandlePosition,
  const vm::vec3d& handleOffset) 
{
  // Record the starting position of the selection box
  m_selectionBounds = vm::bbox3d{initialHandlePosition, initialHandlePosition};
  
  // Update the renderer
  m_renderer.setSelectionBounds(m_selectionBounds);
  
  // No longer use the tool's update method to avoid brush creation errors
  // Only refresh the view
  m_tool.refreshViews();

  const auto& camera = inputState.camera();
  const auto plane = vm::plane3d{
    initialHandlePosition,
    vm::vec3d{vm::get_abs_max_component_axis(camera.direction())}};

  return makeHandlePositionProposer(
    makePlaneHandlePicker(plane, handleOffset), makeIdentityHandleSnapper());
}

DragStatus BoxSelectionDragDelegate::update(
  const InputState&,
  const DragState& dragState,
  const vm::vec3d& proposedHandlePosition) 
{
  // Ensure a valid selection box range
  // We need to ensure that min and max are correctly set, and the box does not have zero or negative size
  auto min = vm::vec3d{
    std::min(dragState.initialHandlePosition.x(), proposedHandlePosition.x()),
    std::min(dragState.initialHandlePosition.y(), proposedHandlePosition.y()),
    std::min(dragState.initialHandlePosition.z(), proposedHandlePosition.z())
  };
  
  auto max = vm::vec3d{
    std::max(dragState.initialHandlePosition.x(), proposedHandlePosition.x()),
    std::max(dragState.initialHandlePosition.y(), proposedHandlePosition.y()),
    std::max(dragState.initialHandlePosition.z(), proposedHandlePosition.z())
  };
  
  // Ensure the box has at least a minimum size to prevent empty brush errors
  const double minSize = 0.1; // Minimum size (adjust as needed)
  
  // Correctly modify the components of vec3d - create a new vector instead of trying to modify the existing vector
  if (max.x() - min.x() < minSize) {
    max = vm::vec3d(min.x() + minSize, max.y(), max.z());
  }
  if (max.y() - min.y() < minSize) {
    max = vm::vec3d(max.x(), min.y() + minSize, max.z());
  }
  if (max.z() - min.z() < minSize) {
    max = vm::vec3d(max.x(), max.y(), min.z() + minSize);
  }
  
  // Update the selection box size
  m_selectionBounds = vm::bbox3d{min, max};
  
  // Update the renderer
  m_renderer.setSelectionBounds(m_selectionBounds);
  
  // No longer create brushes through the tool, only refresh the view
  m_tool.refreshViews();
  return DragStatus::Continue;
}

void BoxSelectionDragDelegate::end(
  const InputState& inputState, 
  const DragState&) 
{ 
  // When the box selection ends, perform the actual selection operation
  auto document = kdl::mem_lock(m_document);
  if (document)
  {
    try {
      // Verify that the selection box is valid
      if (m_selectionBounds.is_empty()) {
        return;
      }
        
      // Get the main direction of the camera to determine the projection plane
      const auto& camera = inputState.camera();
      const auto direction = camera.direction();
      vm::axis::type majorAxis = vm::find_abs_max_component(direction);
        
      // Get all nodes in the current world
      auto allNodes = std::vector<mdl::Node*>{};
      document->world()->accept(kdl::overload(
        [&](auto&& thisLambda, mdl::WorldNode* world) { 
          world->visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, mdl::LayerNode* layer) {
          layer->visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, mdl::GroupNode* group) {
          allNodes.push_back(group);
          group->visitChildren(thisLambda);
        },
        [&](auto&& thisLambda, mdl::EntityNode* entity) {
          allNodes.push_back(entity);
          entity->visitChildren(thisLambda);
        },
        [&](mdl::BrushNode* brush) {
          allNodes.push_back(brush);
        },
        [&](mdl::PatchNode* patch) {
          allNodes.push_back(patch);
        }));
        
      // Filter out nodes that are contained in the selection box - based on view projection
      auto selectedNodes = std::vector<mdl::Node*>{};
      for (auto* node : allNodes) {
        const auto& nodeBounds = node->logicalBounds();
        const auto nodeCenter = nodeBounds.center();
        
        bool nodeSelected = false;
        
        // Determine if the node is within the selection range based on the current view projection
        if (majorAxis == vm::axis::z) {
          // Top view - projected onto the XY plane
          nodeSelected = nodeCenter.x() >= m_selectionBounds.min.x() &&
                        nodeCenter.x() <= m_selectionBounds.max.x() &&
                        nodeCenter.y() >= m_selectionBounds.min.y() &&
                        nodeCenter.y() <= m_selectionBounds.max.y();
        } else if (majorAxis == vm::axis::y) {
          // Front view - projected onto the XZ plane
          nodeSelected = nodeCenter.x() >= m_selectionBounds.min.x() &&
                        nodeCenter.x() <= m_selectionBounds.max.x() &&
                        nodeCenter.z() >= m_selectionBounds.min.z() &&
                        nodeCenter.z() <= m_selectionBounds.max.z();
        } else {
          // Side view - projected onto the YZ plane
          nodeSelected = nodeCenter.y() >= m_selectionBounds.min.y() &&
                        nodeCenter.y() <= m_selectionBounds.max.y() &&
                        nodeCenter.z() >= m_selectionBounds.min.z() &&
                        nodeCenter.z() <= m_selectionBounds.max.z();
        }
        
        if (nodeSelected) {
          selectedNodes.push_back(node);
        }
      }
      
      // Filter out selectable nodes
      auto selectableNodes = mdl::collectSelectableNodes(
        selectedNodes, document->editorContext());
        
      // If there are selectable nodes, create a selection transaction
      if (!selectableNodes.empty()) {
        auto transaction = Transaction{*document, "Box Select"};
        document->deselectAll();
        document->selectNodes(selectableNodes);
        transaction.commit();
      }
      
      // Clear the renderer
      m_renderer.clear();
    } catch (const std::exception&) {
    } catch (...) {
    }
  }
}

void BoxSelectionDragDelegate::cancel(const DragState&) 
{ 
  // Cancel the selection operation
  m_renderer.clear();
}

void BoxSelectionDragDelegate::render(
  const InputState&,
  const DragState&,
  render::RenderContext& renderContext,
  render::RenderBatch& renderBatch) const
{
  // Use the custom renderer to draw the selection box
  m_renderer.render(renderContext, renderBatch);
}

} // namespace tb::ui 