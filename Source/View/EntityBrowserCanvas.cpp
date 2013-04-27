/*
 Copyright (C) 2010-2012 Kristian Duske

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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "EntityBrowserCanvas.h"

#include "IO/FileManager.h"
#include "Model/MapDocument.h"
#include "Renderer/ApplyMatrix.h"
#include "Renderer/EntityModelRenderer.h"
#include "Renderer/EntityModelRendererManager.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/SharedResources.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Shader/ShaderProgram.h"
#include "Renderer/Text/FontManager.h"
#include "Renderer/Text/TexturedFont.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"

#include <map>

namespace TrenchBroom {
    namespace View {
        void EntityBrowserCanvas::addEntityToLayout(Layout& layout, Model::PointEntityDefinition* definition, const Renderer::Text::FontDescriptor& font) {
            if ((!m_hideUnused || definition->usageCount() > 0) && (m_filterText.empty() || Utility::containsString(definition->name(), m_filterText, false))) {
                Renderer::Text::FontManager& fontManager =  m_documentViewHolder.document().sharedResources().fontManager();
                const float maxCellWidth = layout.maxCellWidth();
                const Renderer::Text::FontDescriptor actualFont = fontManager.selectFontSize(font, definition->name(), maxCellWidth, 5);
                const Vec2f actualSize = fontManager.font(actualFont)->measure(definition->name());

                Renderer::EntityModelRendererManager& modelRendererManager = m_documentViewHolder.document().sharedResources().modelRendererManager();
                const StringList& searchPaths = m_documentViewHolder.document().searchPaths();
                Renderer::EntityModelRenderer* modelRenderer = modelRendererManager.modelRenderer(*definition, searchPaths);

                BBoxf rotatedBounds;
                if (modelRenderer != NULL) {
                    const Vec3f& center = modelRenderer->center();
                    const Mat4f transformation = translationMatrix(center) * rotationMatrix(m_rotation) * translationMatrix(-center);
                    rotatedBounds = modelRenderer->boundsAfterTransformation(transformation);
                } else {
                    rotatedBounds = definition->bounds();
                    const Vec3f center = rotatedBounds.center();
                    rotatedBounds.rotate(m_rotation, center);
                }

                Vec3f size = rotatedBounds.size();
                layout.addItem(EntityCellData(definition, modelRenderer, actualFont, rotatedBounds), size.y(), size.z(), actualSize.x(), font.size() + 2.0f);
            }
        }

        void EntityBrowserCanvas::renderEntityBounds(Renderer::Transformation& transformation, Renderer::ShaderProgram& boundsProgram, const Model::PointEntityDefinition& definition, const BBoxf& rotatedBounds, const Vec3f& offset, float scaling) {
            const BBoxf& bounds = definition.bounds();
            const Vec3f rotationOffset = Vec3f(0.0f, -rotatedBounds.min.y(), -rotatedBounds.min.z());
            const Mat4f itemMatrix = (translationMatrix(offset) *
                                      scalingMatrix(scaling) *
                                      translationMatrix(rotationOffset) *
                                      translationMatrix(bounds.center()) *
                                      rotationMatrix(m_rotation) *
                                      translationMatrix(-bounds.center()));
           
            Renderer::ApplyModelMatrix applyItemMatrix(transformation, itemMatrix);
            boundsProgram.setUniformVariable("Color", definition.color());

            // TODO: use a vertex buffer here!
            Vec3f::List vertices;
            bounds.vertices(vertices);

            glBegin(GL_LINES);
            for (unsigned int i = 0; i < vertices.size(); i++)
                Renderer::glVertexV3f(vertices[i]);
            glEnd();
        }

        void EntityBrowserCanvas::renderEntityModel(Renderer::Transformation& transformation, Renderer::ShaderProgram& entityModelProgram, Renderer::EntityModelRenderer& renderer, const BBoxf& rotatedBounds, const Vec3f& offset, float scaling) {
            const Vec3f& rotationCenter = renderer.center();
            const Vec3f rotationOffset = Vec3f(0.0f, -rotatedBounds.min.y(), -rotatedBounds.min.z());
            const Mat4f itemMatrix = (translationMatrix(offset) *
                                      scalingMatrix(scaling) *
                                      translationMatrix(rotationOffset) *
                                      translationMatrix(rotationCenter) *
                                      rotationMatrix(m_rotation) *
                                      translationMatrix(-rotationCenter));

            Renderer::ApplyModelMatrix applyItemMatrix(transformation, itemMatrix);
            renderer.render(entityModelProgram);
        }

        void EntityBrowserCanvas::doInitLayout(Layout& layout) {
            layout.setOuterMargin(5.0f);
            layout.setGroupMargin(5.0f);
            layout.setRowMargin(5.0f);
            layout.setCellMargin(5.0f);
            layout.setCellWidth(93.0f, 93.0f);
            layout.setCellHeight(64.0f, 128.0f);
            layout.setMaxUpScale(1.5f);
        }

        void EntityBrowserCanvas::doReloadLayout(Layout& layout) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Model::EntityDefinitionManager& definitionManager = m_documentViewHolder.document().definitionManager();

            String fontName = prefs.getString(Preferences::RendererFontName);
            int fontSize = prefs.getInt(Preferences::EntityBrowserFontSize);
            assert(fontSize >= 0);

            Renderer::Text::FontDescriptor font(fontName, static_cast<unsigned int>(fontSize));
            IO::FileManager fileManager;

            if (m_group) {
                Model::EntityDefinitionManager::EntityDefinitionGroups groups = definitionManager.groups(Model::EntityDefinition::PointEntity, m_sortOrder);
                Model::EntityDefinitionManager::EntityDefinitionGroups::const_iterator groupIt, groupEnd;

                for (groupIt = groups.begin(), groupEnd = groups.end(); groupIt != groupEnd; ++groupIt) {
                    const String& groupName = groupIt->first;
                    const Model::EntityDefinitionList& definitions = groupIt->second;

                    layout.addGroup(groupName, fontSize + 2.0f);

                    Model::EntityDefinitionList::const_iterator defIt, defEnd;
                    for (defIt = definitions.begin(), defEnd = definitions.end(); defIt != defEnd; ++defIt) {
                        Model::PointEntityDefinition* definition = static_cast<Model::PointEntityDefinition*>(*defIt);
                        addEntityToLayout(layout, definition, font);
                    }
                }
            } else {
                const Model::EntityDefinitionList& definitions = definitionManager.definitions(Model::EntityDefinition::PointEntity, m_sortOrder);
                for (unsigned int i = 0; i < definitions.size(); i++)
                    addEntityToLayout(layout, static_cast<Model::PointEntityDefinition*>(definitions[i]), font);
            }
        }

        void EntityBrowserCanvas::doClear() {
        }

        void EntityBrowserCanvas::doRender(Layout& layout, float y, float height) {
            if (m_vbo == NULL)
                m_vbo = new Renderer::Vbo(GL_ARRAY_BUFFER, 0xFFFF);

            Renderer::ShaderManager& shaderManager = m_documentViewHolder.document().sharedResources().shaderManager();
            Renderer::Text::FontManager& fontManager = m_documentViewHolder.document().sharedResources().fontManager();

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Renderer::Text::FontDescriptor defaultDescriptor(prefs.getString(Preferences::RendererFontName),
                                                             static_cast<unsigned int>(prefs.getInt(Preferences::TextureBrowserFontSize)));

            Renderer::EntityModelRendererManager& modelRendererManager = m_documentViewHolder.document().sharedResources().modelRendererManager();

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_DEPTH_TEST);

            const float viewLeft      = static_cast<float>(GetClientRect().GetLeft());
            const float viewTop       = static_cast<float>(GetClientRect().GetBottom());
            const float viewRight     = static_cast<float>(GetClientRect().GetRight());
            const float viewBottom    = static_cast<float>(GetClientRect().GetTop());

            const Mat4f projection = orthoMatrix(-1024.0f, 1024.0f, viewLeft, viewTop, viewRight, viewBottom);
            Renderer::Transformation transformation(projection, viewMatrix(Vec3f::NegX, Vec3f::PosZ) * translationMatrix(Vec3f(256.0f, 0.0f, 0.0f)));

            size_t visibleGroupCount = 0;
            size_t visibleItemCount = 0;

            typedef std::map<Renderer::Text::FontDescriptor, Vec2f::List> StringMap;
            StringMap stringVertices;

            for (unsigned int i = 0; i < layout.size(); i++) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    visibleGroupCount++;

                    const String& title = group.item();
                    if (!title.empty()) {
                        const LayoutBounds titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
                        const Vec2f offset(titleBounds.left() + 2.0f, height - (titleBounds.top() - y) - titleBounds.height());

                        Renderer::Text::TexturedFont* font = fontManager.font(defaultDescriptor);
                        Vec2f::List titleVertices = font->quads(title, false, offset);
                        Vec2f::List& vertices = stringVertices[defaultDescriptor];
                        vertices.insert(vertices.end(), titleVertices.begin(), titleVertices.end());
                    }

                    for (unsigned int j = 0; j < group.size(); j++) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                visibleItemCount++;

                                const Layout::Group::Row::Cell& cell = row[k];
                                const LayoutBounds titleBounds = cell.titleBounds();
                                const Vec2f offset(titleBounds.left(), height - (titleBounds.top() - y) - titleBounds.height());

                                Renderer::Text::TexturedFont* font = fontManager.font(cell.item().fontDescriptor);
                                Vec2f::List titleVertices = font->quads(cell.item().entityDefinition->name(), false, offset);
                                Vec2f::List& vertices = stringVertices[cell.item().fontDescriptor];
                                vertices.insert(vertices.end(), titleVertices.begin(), titleVertices.end());
                            }
                        }
                    }
                }
            }

            { // render bounds
                Renderer::ActivateShader shader(shaderManager, Renderer::Shaders::EdgeShader);
                for (unsigned int i = 0; i < layout.size(); i++) {
                    const Layout::Group& group = layout[i];
                    if (group.intersectsY(y, height)) {
                        for (unsigned int j = 0; j < group.size(); j++) {
                            const Layout::Group::Row& row = group[j];
                            if (row.intersectsY(y, height)) {
                                for (unsigned int k = 0; k < row.size(); k++) {
                                    const Layout::Group::Row::Cell& cell = row[k];
                                    Model::PointEntityDefinition* definition = cell.item().entityDefinition;
                                    Renderer::EntityModelRenderer* modelRenderer = cell.item().modelRenderer;
                                    if (modelRenderer == NULL)
                                        renderEntityBounds(transformation,
                                                           shader.currentShader(),
                                                           *definition,
                                                           cell.item().bounds, Vec3f(0.0f, cell.itemBounds().left(), height - (cell.itemBounds().bottom() - y)),
                                                           cell.scale());
                                }
                            }
                        }
                    }
                }
            }

            { // render models
                Renderer::ActivateShader shader(shaderManager, Renderer::Shaders::EntityModelShader);
                shader.currentShader().setUniformVariable("ApplyTinting", false);
                shader.currentShader().setUniformVariable("Brightness", prefs.getFloat(Preferences::RendererBrightness));
                shader.currentShader().setUniformVariable("GrayScale", false);

                modelRendererManager.activate();
                for (unsigned int i = 0; i < layout.size(); i++) {
                    const Layout::Group& group = layout[i];
                    if (group.intersectsY(y, height)) {
                        for (unsigned int j = 0; j < group.size(); j++) {
                            const Layout::Group::Row& row = group[j];
                            if (row.intersectsY(y, height)) {
                                for (unsigned int k = 0; k < row.size(); k++) {
                                    const Layout::Group::Row::Cell& cell = row[k];
                                    Renderer::EntityModelRenderer* modelRenderer = cell.item().modelRenderer;
                                    if (modelRenderer != NULL)
                                        renderEntityModel(transformation,
                                                          shader.currentShader(),
                                                          *modelRenderer,
                                                          cell.item().bounds,
                                                          Vec3f(0.0f, cell.itemBounds().left(), height - (cell.itemBounds().bottom() - y)),
                                                          cell.scale());
                                }
                            }
                        }
                    }
                }
                modelRendererManager.deactivate();
            }

            glDisable(GL_DEPTH_TEST);
            transformation = Renderer::Transformation(projection, viewMatrix(Vec3f::NegZ, Vec3f::PosY) * translationMatrix(Vec3f(0.0f, 0.0f, -1.0f)));

            if (visibleGroupCount > 0) { // render group title background
                unsigned int vertexCount = static_cast<unsigned int>(4 * visibleGroupCount);
                Renderer::VertexArray vertexArray(*m_vbo, GL_QUADS, vertexCount,
                                                  Renderer::Attribute::position2f());

                Renderer::SetVboState mapVbo(*m_vbo, Renderer::Vbo::VboMapped);
                for (unsigned int i = 0; i < layout.size(); i++) {
                    const Layout::Group& group = layout[i];
                    if (group.intersectsY(y, height)) {
                        LayoutBounds titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
                        vertexArray.addAttribute(Vec2f(titleBounds.left(), height - (titleBounds.top() - y)));
                        vertexArray.addAttribute(Vec2f(titleBounds.left(), height - (titleBounds.bottom() - y)));
                        vertexArray.addAttribute(Vec2f(titleBounds.right(), height - (titleBounds.bottom() - y)));
                        vertexArray.addAttribute(Vec2f(titleBounds.right(), height - (titleBounds.top() - y)));
                    }
                }

                Renderer::SetVboState activateVbo(*m_vbo, Renderer::Vbo::VboActive);
                Renderer::ActivateShader shader(shaderManager, Renderer::Shaders::BrowserGroupShader);
                shader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::BrowserGroupBackgroundColor));
                vertexArray.render();
            }

            if (!stringVertices.empty()) { // render strings
                StringMap::iterator it, end;
                for (it = stringVertices.begin(), end = stringVertices.end(); it != end; ++it) {
                    const Renderer::Text::FontDescriptor& descriptor = it->first;
                    Renderer::Text::TexturedFont* font = fontManager.font(descriptor);
                    const Vec2f::List& vertices = it->second;

                    unsigned int vertexCount = static_cast<unsigned int>(vertices.size() / 2);
                    Renderer::VertexArray vertexArray(*m_vbo, GL_QUADS, vertexCount,
                                                      Renderer::Attribute::position2f(),
                                                      Renderer::Attribute::texCoord02f(), 0);

                    Renderer::SetVboState mapVbo(*m_vbo, Renderer::Vbo::VboMapped);
                    vertexArray.addAttributes(vertices);

                    Renderer::SetVboState activateVbo(*m_vbo, Renderer::Vbo::VboActive);
                    Renderer::ActivateShader shader(shaderManager, Renderer::Shaders::TextShader);
                    shader.currentShader().setUniformVariable("Color", prefs.getColor(Preferences::BrowserTextColor));
                    shader.currentShader().setUniformVariable("Texture", 0);

                    font->activate();
                    vertexArray.render();
                    font->deactivate();
                }
            }
        }

        bool EntityBrowserCanvas::dndEnabled() {
            return true;
        }

        wxImage* EntityBrowserCanvas::dndImage(const Layout::Group::Row::Cell& cell) {
            if (!SetCurrent(*glContext()))
                return NULL;

            const LayoutBounds& bounds = cell.itemBounds();

            unsigned int width = static_cast<unsigned int>(bounds.width());
            unsigned int height = static_cast<unsigned int>(bounds.height());

            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            Renderer::EntityModelRendererManager& modelRendererManager = m_documentViewHolder.document().sharedResources().modelRendererManager();

            Renderer::ShaderManager& shaderManager = m_documentViewHolder.document().sharedResources().shaderManager();
            Renderer::ShaderProgram& boundsProgram = shaderManager.shaderProgram(Renderer::Shaders::EdgeShader);
            Renderer::ShaderProgram& entityModelProgram = shaderManager.shaderProgram(Renderer::Shaders::EntityModelShader);

            m_offscreenRenderer.setDimensions(width, height);
            m_offscreenRenderer.preRender(); // Scampie's Vista machine crashes here

            const float viewLeft      = 0.0f;
            const float viewTop       = 0.0f;
            const float viewRight     = bounds.width();
            const float viewBottom    = bounds.height();

            const Mat4f projection = orthoMatrix(-1024.0f, 1024.0f, viewLeft, viewTop, viewRight, viewBottom);
            const Mat4f view = viewMatrix(Vec3f::NegX, Vec3f::PosZ) * translationMatrix(Vec3f(256.0f, 0.0f, 0.0f));
            Renderer::Transformation transformation(projection, view);

            glViewport(0, 0, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);

            Renderer::EntityModelRenderer* modelRenderer = cell.item().modelRenderer;
            if (modelRenderer == NULL) {
                boundsProgram.activate();
                Model::PointEntityDefinition* definition = cell.item().entityDefinition;
                renderEntityBounds(transformation, boundsProgram, *definition, cell.item().bounds, Vec3f::Null, cell.scale());
                boundsProgram.deactivate();
            } else {
                modelRendererManager.activate();
                entityModelProgram.activate();
                entityModelProgram.setUniformVariable("ApplyTinting", false);
                entityModelProgram.setUniformVariable("Brightness", prefs.getFloat(Preferences::RendererBrightness));
                renderEntityModel(transformation, entityModelProgram, *modelRenderer, cell.item().bounds, Vec3f::Null, cell.scale());
                entityModelProgram.deactivate();
                modelRendererManager.deactivate();
            }

            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glPixelStorei(GL_PACK_ROW_LENGTH, 0);
            glPixelStorei(GL_PACK_SKIP_ROWS, 0);
            glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

            wxImage* image = m_offscreenRenderer.getImage();
            m_offscreenRenderer.postRender();

            return image;
        }

        wxDataObject* EntityBrowserCanvas::dndData(const Layout::Group::Row::Cell& cell) {
            return new wxTextDataObject("entity:" + cell.item().entityDefinition->name());
        }

        EntityBrowserCanvas::EntityBrowserCanvas(wxWindow* parent, wxWindowID windowId, wxScrollBar* scrollBar, DocumentViewHolder& documentViewHolder) :
        CellLayoutGLCanvas(parent, windowId, documentViewHolder.document().sharedResources().attribs(), documentViewHolder.document().sharedResources().sharedContext(), scrollBar),
        m_documentViewHolder(documentViewHolder),
        m_offscreenRenderer(m_documentViewHolder.document().sharedResources().multisample(), m_documentViewHolder.document().sharedResources().samples()),
        m_vbo(NULL),
        m_group(false),
        m_hideUnused(false),
        m_sortOrder(Model::EntityDefinitionManager::Name) {
            const Quatf hRotation = Quatf(Math<float>::radians(-30.0f), Vec3f::PosZ);
            const Quatf vRotation = Quatf(Math<float>::radians(20.0f), Vec3f::PosY);
            m_rotation = vRotation * hRotation;
        }

        EntityBrowserCanvas::~EntityBrowserCanvas() {
            clear();
            delete m_vbo;
            m_vbo = NULL;
        }
    }
}
