/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "Clipper.h"

#include "CollectionUtils.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/World.h"
#include "Renderer/BrushRenderer.h"
#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        Clipper::Clipper(MapDocumentWPtr document) :
        m_document(document),
        m_clipSide(ClipSide_Front),
        m_remainingBrushRenderer(new Renderer::BrushRenderer(false)),
        m_clippedBrushRenderer(new Renderer::BrushRenderer(true)) {}

        void Clipper::update() {
            clearRenderers();
            clearBrushes();

            Vec3 point1, point2, point3;
            if (doGetPoints(point1, point2, point3)) {
                clipBrushes(point1, point2, point3);
                updateRenderers();
            }
        }

        void Clipper::clearBrushes() {
            MapUtils::clearAndDelete(m_frontBrushes);
            MapUtils::clearAndDelete(m_backBrushes);
        }
        
        void Clipper::clipBrushes(const Vec3& point1, const Vec3& point2, const Vec3& point3) {
            MapDocumentSPtr document = lock(m_document);
            const Model::BrushList& brushes = document->selectedNodes().brushes();
            const BBox3& worldBounds = document->worldBounds();
            
            if (canClip()) {
                Model::World* world = document->world();
                Model::BrushList::const_iterator bIt, bEnd;
                for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                    Model::Brush* brush = *bIt;
                    Model::Node* parent = brush->parent();
                    
                    Model::BrushFace* frontFace = world->createFace(point1, point2, point3, document->currentTextureName());
                    Model::BrushFace* backFace = world->createFace(point1, point3, point2, document->currentTextureName());
                    setFaceAttributes(brush->faces(), frontFace, backFace);
                    
                    Model::Brush* frontBrush = brush->clone(worldBounds);
                    if (frontBrush->clip(worldBounds, frontFace))
                        m_frontBrushes[parent].push_back(frontBrush);
                    else
                        delete frontBrush;
                    
                    Model::Brush* backBrush = brush->clone(worldBounds);
                    if (backBrush->clip(worldBounds, backFace))
                        m_backBrushes[parent].push_back(backBrush);
                    else
                        delete backBrush;
                }
            } else {
                Model::BrushList::const_iterator bIt, bEnd;
                for (bIt = brushes.begin(), bEnd = brushes.end(); bIt != bEnd; ++bIt) {
                    Model::Brush* brush = *bIt;
                    Model::Node* parent = brush->parent();
                    
                    Model::Brush* frontBrush = brush->clone(worldBounds);
                    m_frontBrushes[parent].push_back(frontBrush);
                }
            }
        }
        
        bool Clipper::canClip() const {
            return doCanClip();
        }

        void Clipper::setFaceAttributes(const Model::BrushFaceList& faces, Model::BrushFace* frontFace, Model::BrushFace* backFace) const {
            assert(!faces.empty());
            
            Model::BrushFaceList::const_iterator faceIt = faces.begin();
            Model::BrushFaceList::const_iterator faceEnd = faces.end();
            const Model::BrushFace* bestFrontFace = *faceIt++;
            const Model::BrushFace* bestBackFace = bestFrontFace;
            
            while (faceIt != faceEnd) {
                const Model::BrushFace* face = *faceIt;
                
                const Vec3 bestFrontDiff = bestFrontFace->boundary().normal - frontFace->boundary().normal;
                const Vec3 frontDiff = face->boundary().normal - frontFace->boundary().normal;
                if (frontDiff.squaredLength() < bestFrontDiff.squaredLength())
                    bestFrontFace = face;
                
                const Vec3f bestBackDiff = bestBackFace->boundary().normal - backFace->boundary().normal;
                const Vec3f backDiff = face->boundary().normal - backFace->boundary().normal;
                if (backDiff.squaredLength() < bestBackDiff.squaredLength())
                    bestBackFace = face;
                ++faceIt;
            }
            
            assert(bestFrontFace != NULL);
            assert(bestBackFace != NULL);
            frontFace->setAttributes(bestFrontFace);
            backFace->setAttributes(bestBackFace);
        }
        
        void Clipper::clearRenderers() {
            m_remainingBrushRenderer->clear();
            m_clippedBrushRenderer->clear();
        }
        
        void Clipper::updateRenderers() {
            if (keepFrontBrushes())
                addBrushesToRenderer(m_frontBrushes, m_remainingBrushRenderer);
            else
                addBrushesToRenderer(m_frontBrushes, m_clippedBrushRenderer);
            
            if (keepBackBrushes())
                addBrushesToRenderer(m_backBrushes, m_remainingBrushRenderer);
            else
                addBrushesToRenderer(m_backBrushes, m_clippedBrushRenderer);
        }
        
        
        class Clipper::AddBrushesToRendererVisitor : public Model::NodeVisitor {
        private:
            Renderer::BrushRenderer* m_renderer;
        public:
            AddBrushesToRendererVisitor(Renderer::BrushRenderer* renderer) : m_renderer(renderer) {}
        private:
            void doVisit(Model::World* world)   {}
            void doVisit(Model::Layer* layer)   {}
            void doVisit(Model::Group* group)   {}
            void doVisit(Model::Entity* entity) {}
            void doVisit(Model::Brush* brush)   { m_renderer->addBrush(brush); }
        };
        
        void Clipper::addBrushesToRenderer(const Model::ParentChildrenMap& map, Renderer::BrushRenderer* renderer) {
            AddBrushesToRendererVisitor visitor(renderer);
            
            Model::ParentChildrenMap::const_iterator it, end;
            for (it = map.begin(), end = map.end(); it != end; ++it) {
                const Model::NodeList& brushes = it->second;
                Model::Node::accept(brushes.begin(), brushes.end(), visitor);
            }
        }

        bool Clipper::keepFrontBrushes() const {
            return m_clipSide != ClipSide_Back;
        }
        
        bool Clipper::keepBackBrushes() const {
            return m_clipSide != ClipSide_Front;
        }
    }
}
