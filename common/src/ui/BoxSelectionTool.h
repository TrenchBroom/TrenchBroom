//Added by Lws

#pragma once

#include "ui/HandleDragTracker.h"
#include "vm/bbox.h"

#include <memory>

namespace tb::render
{
class RenderContext;
class RenderBatch;
}

namespace tb::mdl
{
class Node;
}

namespace tb::ui
{
class DrawShapeTool;
class MapDocument;
class InputState;
struct DragState;

// Box selection renderer - used to display the box selection area
class SelectionBoxRenderer
{
private:
  vm::bbox3d m_selectionBounds;
  bool m_valid;
  
public:
  SelectionBoxRenderer();
  
  void setSelectionBounds(const vm::bbox3d& bounds);
  void clear();
  
  void render(render::RenderContext& renderContext, render::RenderBatch& renderBatch) const;
};

// Box selection tool delegate
class BoxSelectionDragDelegate : public HandleDragTrackerDelegate
{
private:
  DrawShapeTool& m_tool;
  std::weak_ptr<MapDocument> m_document;
  vm::bbox3d m_selectionBounds;
  SelectionBoxRenderer m_renderer;

public:
  BoxSelectionDragDelegate(DrawShapeTool& tool, std::weak_ptr<MapDocument> document);

  HandlePositionProposer start(
    const InputState& inputState,
    const vm::vec3d& initialHandlePosition,
    const vm::vec3d& handleOffset) override;

  DragStatus update(
    const InputState& inputState,
    const DragState& dragState,
    const vm::vec3d& proposedHandlePosition) override;

  void end(const InputState& inputState, const DragState& dragState) override;
  void cancel(const DragState& dragState) override;

  void render(
    const InputState& inputState,
    const DragState& dragState,
    render::RenderContext& renderContext,
    render::RenderBatch& renderBatch) const override;
};

} // namespace tb::ui 