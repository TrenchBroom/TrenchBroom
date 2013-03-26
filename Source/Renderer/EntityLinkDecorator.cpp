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

#include "EntityLinkDecorator.h"

#include "Model/Entity.h"
#include "Model/Filter.h"
#include "Model/Map.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Utility/VecMath.h"
#include "Utility/Console.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        EntityLinkDecorator::EntityLinkDecorator(const Model::Map& map, const Color& color) :
        EntityDecorator(map),
        m_color(color),
        m_vertexArray(NULL),
        m_valid(false),
        m_doRebuild(true) {}

        // helper function
        void EntityLinkDecorator::addArrowVerts(Vec4f::List &vList, const Vec3f &pointA, const Vec3f &pointB) {
            if ((pointB - pointA).lengthSquared() < 0.0001f)
                return;

            vList.push_back(Vec4f(pointA, 0.0f));
            vList.push_back(Vec4f(pointB, 1.0f));

#if 0
            // calculate arrowhead
            const Vec3f
                lineVec = pointB - pointA,
                lineDir = lineVec.normalized();
            Vec3f
                lineRight;

            if( fabs( lineDir.dot( Vec3f::PosY )) < 0.9f ) {
                lineRight = -lineDir.crossed( Vec3f::PosY );
            } else {
                lineRight = -lineDir.crossed( Vec3f::PosZ );
            }

            lineRight.normalize();

            const Vec3f
                lineUp = lineDir.crossed( lineRight );

            const Vec3f
                arrowBase		= pointB - lineDir * 16.0f,
                arrowCorner1	= arrowBase + lineUp * 4.0,
                arrowCorner2	= arrowBase + lineRight * 4.0,
                arrowCorner3	= arrowBase - lineUp * 4.0,
                arrowCorner4	= arrowBase - lineRight * 4.0;

            float
                lineLength = lineVec.length(),
                arrowCornerDist = ( lineLength - 16.0f ) / lineLength;

            vList.push_back( Vec4f( pointB, 1.0f ) );
            vList.push_back( Vec4f( arrowCorner1, arrowCornerDist ));
            vList.push_back( Vec4f( pointB, 1.0f ) );
            vList.push_back( Vec4f( arrowCorner2, arrowCornerDist ));
            vList.push_back( Vec4f( pointB, 1.0f ) );
            vList.push_back( Vec4f( arrowCorner3, arrowCornerDist ));
            vList.push_back( Vec4f( pointB, 1.0f ) );
            vList.push_back( Vec4f( arrowCorner4, arrowCornerDist ));

            vList.push_back( Vec4f( arrowCorner1, arrowCornerDist ));
            vList.push_back( Vec4f( arrowCorner2, arrowCornerDist ));
            vList.push_back( Vec4f( arrowCorner2, arrowCornerDist ));
            vList.push_back( Vec4f( arrowCorner3, arrowCornerDist ));
            vList.push_back( Vec4f( arrowCorner3, arrowCornerDist ));
            vList.push_back( Vec4f( arrowCorner4, arrowCornerDist ));
            vList.push_back( Vec4f( arrowCorner4, arrowCornerDist ));
            vList.push_back( Vec4f( arrowCorner1, arrowCornerDist ));
#endif
        }

        // only find local links
        void EntityLinkDecorator::gatherLinksLocal(Vec4f::List& vList, RenderContext& context, Model::Entity& curEnt) {
            if (!context.filter().entityVisible(curEnt))
                return;

            Model::EntityList::const_iterator it, end;
            const Model::EntityList& targetList = curEnt.linkTargets();

            for (it = targetList.begin(), end = targetList.end(); it != end; ++it) {
                const Model::Entity& targetEnt = **it;

                if (!context.filter().entityVisible(targetEnt))
                    continue;

                addArrowVerts(vList, curEnt.center(), targetEnt.center());
            }

            const Model::EntityList& sourceList = curEnt.linkSources();

            for (it = sourceList.begin(), end = sourceList.end(); it != end; ++it) {
                const Model::Entity& sourceEnt = **it;

                if (!context.filter().entityVisible(sourceEnt))
                    continue;

                addArrowVerts(vList, sourceEnt.center(), curEnt.center());
            }
        }

        // find links in a "context"
        void EntityLinkDecorator::gatherLinks(Vec4f::List& vListLocal, Vec4f::List& vListContext, RenderContext& context, Model::Entity& curEnt, Model::EntitySet& visitedEntities) {
            Model::EntitySet::iterator vIt = visitedEntities.lower_bound(&curEnt);

            if (*vIt == &curEnt)
                return;

            visitedEntities.insert(vIt, &curEnt);

            if (!context.filter().entityVisible(curEnt))
                return;

            Model::EntityList::const_iterator it, end;
            const Model::EntityList& targetList = curEnt.linkTargets();

            for (it = targetList.begin(), end = targetList.end(); it != end; ++it) {
                Model::Entity& targetEnt = **it;

                if (!context.filter().entityVisible(targetEnt))
                    continue;

                if (curEnt.selected() || targetEnt.selected())
                    addArrowVerts(vListLocal, curEnt.center(), targetEnt.center());
                else
                    addArrowVerts(vListContext, curEnt.center(), targetEnt.center());

                gatherLinks(vListLocal, vListContext, context, targetEnt, visitedEntities);
            }

            const Model::EntityList& sourceList = curEnt.linkSources();

            for (it = sourceList.begin(), end = sourceList.end(); it != end; ++it) {
                Model::Entity& sourceEnt = **it;

                if (!context.filter().entityVisible(sourceEnt))
                    continue;

                gatherLinks(vListLocal, vListContext, context, sourceEnt, visitedEntities);
            }
        }

        void EntityLinkDecorator::gatherLinksUnrelated(Vec4f::List& vList, RenderContext& context, Model::Entity& curEnt) {
            if (!context.filter().entityVisible(curEnt))
                return;

            const Model::EntityList& targetList = curEnt.linkTargets();
            const Model::EntityList& entities = map().entities();
            Model::EntityList::const_iterator it, end;

            for (it = targetList.begin(), end = targetList.end(); it != end; ++it) {
                const Model::Entity& targetEnt = **it;

                if (!context.filter().entityVisible(targetEnt))
                    continue;

                addArrowVerts(vList, curEnt.center(), targetEnt.center());
            }
        }

        void EntityLinkDecorator::render(Vbo& vbo, RenderContext& context) {
            if (context.viewOptions().linkDisplayMode() == View::ViewOptions::LinkDisplayNone)
                return;

            const Model::EntityList& entities = map().entities();
            if (entities.empty())
                return;

            SetVboState activateVbo(vbo, Vbo::VboActive);
            
            if (m_doRebuild) {
                delete m_vertexArray;
                m_vertexArray = NULL;
                m_doRebuild = false;

                // for keeping track of which entities have already been visited by the link-gathering algorithm - should this be part of each entity instead ?
                Model::EntitySet visitedEntities;

                Vec4f::List vertsLocal; // links directly connected to a selected entity
                Vec4f::List vertsContext; // links not directly connected, but in the same context
                Vec4f::List vertsUnrelated; // links not related to the current selection

                Model::EntityList::const_iterator it, end;

                // first pass, outputs local or local+context links
                for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                    Model::Entity& entity = **it;

                    if (entity.selected()) {
                        if (context.viewOptions().linkDisplayMode() == View::ViewOptions::LinkDisplayLocal) {
                            gatherLinksLocal(vertsLocal, context, entity);
                        } else {
                            gatherLinks(vertsLocal, vertsContext, context, entity, visitedEntities);
                        }
                    }
                }

                // second pass, only used in "display all" mode, outputs target links of entities the first pass didn't visit
                if (context.viewOptions().linkDisplayMode() == View::ViewOptions::LinkDisplayAll) {
                    Model::EntitySet::const_iterator vIt;
                    for (it = entities.begin(), end = entities.end(); it != end; ++it) {
                        Model::Entity& entity = **it;
                        vIt = visitedEntities.lower_bound( &entity );

                        if( *vIt != &entity )
                            gatherLinksUnrelated(vertsUnrelated, context, entity);
                    }
                }

                unsigned int vertexCount = static_cast<unsigned int>(vertsLocal.size() + vertsContext.size() + vertsUnrelated.size());
                if (vertexCount == 0)
                    return;
                
                //FIXME : doesn't need to be a color4f at the moment
                m_vertexArray = new VertexArray(vbo, GL_LINES, vertexCount, Attribute::position3f(), Attribute::color4f(), 0);
                
                SetVboState mapVbo(vbo, Vbo::VboMapped);

                // draw order : unrelated -> context -> local
                // vertData.y sets the type of link for the shader ( 0 = unrelated, 0.5 = context, 1.0 = local )
                Vec4f vertData(1.0f);

                vertData.y = 0.0f;
                for (int i = 0; i != vertsUnrelated.size(); ++i ) {
                    vertData.x = vertsUnrelated[i].w;
                    m_vertexArray->addAttribute(Vec3f(vertsUnrelated[i].x, vertsUnrelated[i].y, vertsUnrelated[i].z));
                    m_vertexArray->addAttribute(vertData);
                }
                vertData.y = 0.5f;
                for (int i = 0; i != vertsContext.size(); ++i) {
                    vertData.x = vertsContext[i].w;
                    m_vertexArray->addAttribute(Vec3f(vertsContext[i].x, vertsContext[i].y, vertsContext[i].z));
                    m_vertexArray->addAttribute(vertData);
                }
                vertData.y = 1.0f;
                for (int i = 0; i != vertsLocal.size(); ++i) {
                    vertData.x = vertsLocal[i].w;
                    m_vertexArray->addAttribute(Vec3f(vertsLocal[i].x, vertsLocal[i].y, vertsLocal[i].z));
                    m_vertexArray->addAttribute(vertData);
                }

                m_valid = true;
            }

            if (!m_valid || m_vertexArray == NULL)
                return;

            ActivateShader shader(context.shaderManager(), Shaders::EntityLinkShader );
            shader.currentShader().setUniformVariable("CameraPosition", context.camera().position());
            //shader.currentShader().setUniformVariable("Color", m_color); // unused at the moment, make color view-prefs for the different types of links ?
            shader.currentShader().setUniformVariable("Occluded", 1.0f);

            // render the "occluded" portion without depth-test
            glLineWidth(2.0f);
            glDepthMask(GL_FALSE);
            glDisable(GL_DEPTH_TEST);

            m_vertexArray->render();

            shader.currentShader().setUniformVariable("Occluded", 0.0f);
            glEnable(GL_DEPTH_TEST);

            m_vertexArray->render();

            glDepthMask(GL_TRUE);
            glLineWidth(1.0f);
        }
    }
}