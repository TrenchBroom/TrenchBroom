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

#include "EntityLinkRenderer.h"

#include "Macros.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Model/BrushNode.h"
#include "Model/EditorContext.h"
#include "Model/EntityNode.h"
#include "Model/EntityNodeBase.h"
#include "Model/GroupNode.h"
#include "Model/LayerNode.h"
#include "Model/WorldNode.h"
#include "Renderer/ActiveShader.h"
#include "Renderer/Camera.h"
#include "Renderer/PrimType.h"
#include "Renderer/RenderBatch.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/Shaders.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>
#include <kdl/overload.h>

#include <vecmath/vec.h>

#include <cassert>
#include <unordered_set>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        EntityLinkRenderer::EntityLinkRenderer(std::weak_ptr<View::MapDocument> document) :
        m_document(document),
        m_defaultColor(0.5f, 1.0f, 0.5f, 1.0f),
        m_selectedColor(1.0f, 0.0f, 0.0f, 1.0f),
        m_valid(false) {}

        void EntityLinkRenderer::setDefaultColor(const Color& color) {
            if (color == m_defaultColor)
                return;
            m_defaultColor = color;
            invalidate();
        }

        void EntityLinkRenderer::setSelectedColor(const Color& color) {
            if (color == m_selectedColor)
                return;
            m_selectedColor = color;
            invalidate();
        }

        void EntityLinkRenderer::render(RenderContext&, RenderBatch& renderBatch) {
            renderBatch.add(this);
        }

        void EntityLinkRenderer::invalidate() {
            m_valid = false;
        }

        void EntityLinkRenderer::doPrepareVertices(VboManager& vboManager) {
            if (!m_valid) {
                validate();

                // Upload the VBO's
                m_entityLinks.prepare(vboManager);
                m_entityLinkArrows.prepare(vboManager);
            }
        }

        void EntityLinkRenderer::doRender(RenderContext& renderContext) {
            assert(m_valid);
            renderLines(renderContext);
            renderArrows(renderContext);
        }

        void EntityLinkRenderer::renderLines(RenderContext& renderContext) {
            ActiveShader shader(renderContext.shaderManager(), Shaders::EntityLinkShader);
            shader.set("CameraPosition", renderContext.camera().position());
            shader.set("IsOrtho", renderContext.camera().orthographicProjection());
            shader.set("MaxDistance", 6000.0f);

            glAssert(glDisable(GL_DEPTH_TEST));
            shader.set("Alpha", 0.4f);
            m_entityLinks.render(PrimType::Lines);

            glAssert(glEnable(GL_DEPTH_TEST));
            shader.set("Alpha", 1.0f);
            m_entityLinks.render(PrimType::Lines);
        }

        void EntityLinkRenderer::renderArrows(RenderContext& renderContext) {
            ActiveShader shader(renderContext.shaderManager(), Shaders::EntityLinkArrowShader);
            shader.set("CameraPosition", renderContext.camera().position());
            shader.set("IsOrtho", renderContext.camera().orthographicProjection());
            shader.set("MaxDistance", 6000.0f);
            shader.set("Zoom", renderContext.camera().zoom());

            glAssert(glDisable(GL_DEPTH_TEST));
            shader.set("Alpha", 0.4f);
            m_entityLinkArrows.render(PrimType::Lines);

            glAssert(glEnable(GL_DEPTH_TEST));
            shader.set("Alpha", 1.0f);
            m_entityLinkArrows.render(PrimType::Lines);
        }

        void EntityLinkRenderer::validate() {
            std::vector<Vertex> links;
            getLinks(links);

            // build the arrows before destroying `links`
            std::vector<ArrowVertex> arrows;
            getArrows(arrows, links);

            m_entityLinks = VertexArray::move(std::move(links));
            m_entityLinkArrows = VertexArray::move(std::move(arrows));

            m_valid = true;
        }

        void EntityLinkRenderer::getArrows(std::vector<ArrowVertex>& arrows, const std::vector<Vertex>& links) {
            assert((links.size() % 2) == 0);
            for (size_t i = 0; i < links.size(); i += 2) {
                const auto& startVertex = links[i];
                const auto& endVertex = links[i + 1];

                const auto lineVec = (getVertexComponent<0>(endVertex) - getVertexComponent<0>(startVertex));
                const auto lineLength = length(lineVec);
                const auto lineDir = lineVec / lineLength;
                const auto color = getVertexComponent<1>(startVertex);

                if (lineLength < 512) {
                    const auto arrowPosition = getVertexComponent<0>(startVertex) + (lineVec * 0.6f);
                    addArrow(arrows, color, arrowPosition, lineDir);
                } else if (lineLength < 1024) {
                    const auto arrowPosition1 = getVertexComponent<0>(startVertex) + (lineVec * 0.2f);
                    const auto arrowPosition2 = getVertexComponent<0>(startVertex) + (lineVec * 0.6f);

                    addArrow(arrows, color, arrowPosition1, lineDir);
                    addArrow(arrows, color, arrowPosition2, lineDir);
                } else {
                    const auto arrowPosition1 = getVertexComponent<0>(startVertex) + (lineVec * 0.1f);
                    const auto arrowPosition2 = getVertexComponent<0>(startVertex) + (lineVec * 0.4f);
                    const auto arrowPosition3 = getVertexComponent<0>(startVertex) + (lineVec * 0.7f);

                    addArrow(arrows, color, arrowPosition1, lineDir);
                    addArrow(arrows, color, arrowPosition2, lineDir);
                    addArrow(arrows, color, arrowPosition3, lineDir);
                }
            }
        }

        void EntityLinkRenderer::addArrow(std::vector<ArrowVertex>& arrows, const vm::vec4f& color, const vm::vec3f& arrowPosition, const vm::vec3f& lineDir) {
            arrows.emplace_back(vm::vec3f{0, 3, 0}, color, arrowPosition, lineDir);
            arrows.emplace_back(vm::vec3f{9, 0, 0}, color, arrowPosition, lineDir);

            arrows.emplace_back(vm::vec3f{9, 0, 0}, color, arrowPosition, lineDir);
            arrows.emplace_back(vm::vec3f{0,-3, 0}, color, arrowPosition, lineDir);
        }

        namespace {
            class CollectLinksVisitor {
            protected:
                const Model::EditorContext& m_editorContext;
                const Color m_defaultColor;
                const Color m_selectedColor;
                std::vector<EntityLinkRenderer::Vertex>& m_links;
            protected:
                CollectLinksVisitor(const Model::EditorContext& editorContext, const Color& defaultColor, const Color& selectedColor, std::vector<EntityLinkRenderer::Vertex>& links) :
                m_editorContext(editorContext),
                m_defaultColor(defaultColor),
                m_selectedColor(selectedColor),
                m_links(links) {}
            public:
                virtual ~CollectLinksVisitor() = default;
                virtual void visit(Model::EntityNodeBase* node) = 0;
            protected:
                void addLink(const Model::EntityNodeBase* source, const Model::EntityNodeBase* target) {
                    const auto anySelected = source->selected() || source->descendantSelected() || target->selected() || target->descendantSelected();
                    const auto& sourceColor = anySelected ? m_selectedColor : m_defaultColor;
                    const auto targetColor = anySelected ? m_selectedColor : m_defaultColor;

                    m_links.emplace_back(vm::vec3f(source->linkSourceAnchor()), sourceColor);
                    m_links.emplace_back(vm::vec3f(target->linkTargetAnchor()), targetColor);
                }
            };

            class CollectAllLinksVisitor : public CollectLinksVisitor {
            public:
                CollectAllLinksVisitor(const Model::EditorContext& editorContext, const Color& defaultColor, const Color& selectedColor, std::vector<EntityLinkRenderer::Vertex>& links) :
                CollectLinksVisitor(editorContext, defaultColor, selectedColor, links) {}

                void visit(Model::EntityNodeBase* node) override {
                    if (m_editorContext.visible(node)) {
                        addTargets(node, node->linkTargets());
                        addTargets(node, node->killTargets());
                    }
                }
            private:
                void addTargets(Model::EntityNodeBase* source, const std::vector<Model::EntityNodeBase*>& targets) {
                    for (const Model::EntityNodeBase* target : targets) {
                        if (m_editorContext.visible(target))
                            addLink(source, target);
                    }
                }
            };

            class CollectTransitiveSelectedLinksVisitor : public CollectLinksVisitor {
            private:
                std::unordered_set<Model::Node*> m_visited;
            public:
                CollectTransitiveSelectedLinksVisitor(const Model::EditorContext& editorContext, const Color& defaultColor, const Color& selectedColor, std::vector<EntityLinkRenderer::Vertex>& links) :
                CollectLinksVisitor(editorContext, defaultColor, selectedColor, links) {}

                void visit(Model::EntityNodeBase* node) override {
                    if (m_editorContext.visible(node)) {
                        const bool visited = !m_visited.insert(node).second;
                        if (!visited) {
                            addSources(node->linkSources(), node);
                            addSources(node->killSources(), node);
                            addTargets(node, node->linkTargets());
                            addTargets(node, node->killTargets());
                        }
                    }
                }
            private:
                void addSources(const std::vector<Model::EntityNodeBase*>& sources, Model::EntityNodeBase* target) {
                    for (Model::EntityNodeBase* source : sources) {
                        if (m_editorContext.visible(source)) {
                            addLink(source, target);
                            visit(source);
                        }
                    }
                }

                void addTargets(Model::EntityNodeBase* source, const std::vector<Model::EntityNodeBase*>& targets) {
                    for (Model::EntityNodeBase* target : targets) {
                        if (m_editorContext.visible(target)) {
                            addLink(source, target);
                            visit(target);
                        }
                    }
                }
            };

            struct CollectDirectSelectedLinksVisitor : public CollectLinksVisitor {
            public:
                CollectDirectSelectedLinksVisitor(const Model::EditorContext& editorContext, const Color& defaultColor, const Color& selectedColor, std::vector<EntityLinkRenderer::Vertex>& links) :
                CollectLinksVisitor(editorContext, defaultColor, selectedColor, links) {}
                
                void visit(Model::EntityNodeBase* node) override {
                    if (node->selected() || node->descendantSelected()) {
                        addSources(node->linkSources(), node);
                        addSources(node->killSources(), node);
                        addTargets(node, node->linkTargets());
                        addTargets(node, node->killTargets());
                    }
                }
            private:
                void addSources(const std::vector<Model::EntityNodeBase*>& sources, Model::EntityNodeBase* target) {
                    for (const Model::EntityNodeBase* source : sources) {
                        if (!source->selected() && !source->descendantSelected() && m_editorContext.visible(source))
                            addLink(source, target);
                    }
                }

                void addTargets(Model::EntityNodeBase* source, const std::vector<Model::EntityNodeBase*>& targets) {
                    for (const Model::EntityNodeBase* target : targets) {
                        if (m_editorContext.visible(target))
                            addLink(source, target);
                    }
                }
            };
        }

        void EntityLinkRenderer::getLinks(std::vector<Vertex>& links) const {
            const QString entityLinkMode = pref(Preferences::EntityLinkMode);

            if (entityLinkMode == Preferences::entityLinkModeAll()) {
                getAllLinks(links);
            } else if (entityLinkMode == Preferences::entityLinkModeTransitive()) {
                getTransitiveSelectedLinks(links);
            } else if (entityLinkMode == Preferences::entityLinkModeDirect()) {
                getDirectSelectedLinks(links);
            }
        }

        void EntityLinkRenderer::getAllLinks(std::vector<Vertex>& links) const {
            auto document = kdl::mem_lock(m_document);
            const Model::EditorContext& editorContext = document->editorContext();

            CollectAllLinksVisitor collectLinks(editorContext, m_defaultColor, m_selectedColor, links);

            if (document->world() != nullptr) {
                document->world()->accept(kdl::overload(
                    [](auto&& thisLambda, Model::WorldNode* world) {
                        world->visitChildren(thisLambda);
                    },
                    [](auto&& thisLambda, Model::LayerNode* layer) {
                        layer->visitChildren(thisLambda);
                    },
                    [](auto&& thisLambda, Model::GroupNode* group) {
                        group->visitChildren(thisLambda);
                    },
                    [&](Model::EntityNode* entity) {
                        collectLinks.visit(entity);
                    },
                    [](Model::BrushNode*) {}
                ));
            }
        }

        static void collectSelectedLinks(const Model::NodeCollection& selectedNodes, CollectLinksVisitor& collectLinks) {
            for (auto* node : selectedNodes) {
                node->accept(kdl::overload(
                    [](Model::WorldNode*) {},
                    [](Model::LayerNode*) {},
                    [](Model::GroupNode*) {},
                    [&](Model::EntityNode* entity) {
                        collectLinks.visit(entity);
                    },
                    [](auto&& thisLambda, Model::BrushNode* brush) {
                        brush->parent()->accept(thisLambda);
                    }
                ));
            }
        }

        void EntityLinkRenderer::getTransitiveSelectedLinks(std::vector<Vertex>& links) const {
            auto document = kdl::mem_lock(m_document);
            const Model::EditorContext& editorContext = document->editorContext();

            CollectTransitiveSelectedLinksVisitor collectLinks(editorContext, m_defaultColor, m_selectedColor, links);
            collectSelectedLinks(document->selectedNodes(), collectLinks);
        }

        void EntityLinkRenderer::getDirectSelectedLinks(std::vector<Vertex>& links) const {
            auto document = kdl::mem_lock(m_document);
            const Model::EditorContext& editorContext = document->editorContext();

            CollectDirectSelectedLinksVisitor collectLinks(editorContext, m_defaultColor, m_selectedColor, links);
            collectSelectedLinks(document->selectedNodes(), collectLinks);
        }
    }
}
