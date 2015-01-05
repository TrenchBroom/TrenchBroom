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

#include "ClipTool.h"

#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace View {
        ClipTool::ClipPlaneStrategy::~ClipPlaneStrategy() {}
        
        Vec3 ClipTool::ClipPlaneStrategy::snapClipPoint(const Grid& grid, const Vec3& point) const {
            return doSnapClipPoint(grid, point);
        }

        bool ClipTool::ClipPlaneStrategy::computeClipPlane(const Vec3& point1, const Vec3& point2, Plane3& clipPlane) const {
            return doComputeClipPlane(point1, point2, clipPlane);
        }
        
        bool ClipTool::ClipPlaneStrategy::computeClipPlane(const Vec3& point1, const Vec3& point2, const Vec3& point3, const Plane3& clipPlane2, Plane3& clipPlane3) const {
            if (!doComputeClipPlane(point1, point2, point3, clipPlane3))
                return false;
            if (clipPlane2.normal.dot(clipPlane3.normal) < 0.0)
                clipPlane3.normal *= -1.0;
            return true;
        }

        ClipTool::ClipTool(MapDocumentWPtr document) :
        Tool(false),
        m_document(document),
        m_numClipPoints(0) {}
        
        void ClipTool::toggleClipSide() {
        }
        
        void ClipTool::performClip() {
        }
        
        void ClipTool::pick(const Ray3& pickRay, Model::PickResult& pickResult) {
        }
        
        Vec3 ClipTool::defaultClipPointPos() const {
            MapDocumentSPtr document = lock(m_document);
            return document->selectionBounds().center();
        }
        
        bool ClipTool::addClipPoint(const Vec3& point, const ClipPlaneStrategy& strategy) {
            MapDocumentSPtr document = lock(m_document);
            const Grid& grid = document->grid();
            const Vec3 snappedPoint = strategy.snapClipPoint(grid, point);
            switch (m_numClipPoints) {
                case 0:
                    m_clipPoints[0] = snappedPoint;
                    ++m_numClipPoints;
                    return true;
                case 1:
                    if (strategy.computeClipPlane(m_clipPoints[0], snappedPoint, m_clipPlanes[0])) {
                        m_clipPoints[1] = snappedPoint;
                        ++m_numClipPoints;
                        return true;
                    }
                    return false;
                case 2:
                    if (strategy.computeClipPlane(m_clipPoints[0], m_clipPoints[1], snappedPoint, m_clipPlanes[0], m_clipPlanes[1])) {
                        m_clipPoints[2] = snappedPoint;
                        ++m_numClipPoints;
                        return true;
                    }
                    return false;
                default:
                    return false;
            }
        }
        
        bool ClipTool::updateClipPoint(const size_t index, const Vec3& newPosition, const ClipPlaneStrategy& strategy) {
            return false;
        }
        
        bool ClipTool::hasClipPoints() const {
            return m_numClipPoints > 0;
        }
        
        void ClipTool::deleteLastClipPoint() {
            if (m_numClipPoints > 0)
                --m_numClipPoints;
        }
        
        bool ClipTool::reset() {
            if (!hasClipPoints())
                return false;
            m_numClipPoints = 0;
            return true;
        }

        bool ClipTool::doActivate() {
            MapDocumentSPtr document = lock(m_document);
            if (!document->selectedNodes().hasOnlyBrushes())
                return false;
            bindObservers();
            reset();
            update();
            return true;
        }
        
        bool ClipTool::doDeactivate() {
            reset();
            unbindObservers();
            return true;
        }
        
        void ClipTool::bindObservers() {
            MapDocumentSPtr document = lock(m_document);
            document->selectionDidChangeNotifier.addObserver(this, &ClipTool::selectionDidChange);
            document->nodesWillChangeNotifier.addObserver(this, &ClipTool::nodesWillChange);
            document->nodesDidChangeNotifier.addObserver(this, &ClipTool::nodesDidChange);
        }
        
        void ClipTool::unbindObservers() {
            if (!expired(m_document)) {
                MapDocumentSPtr document = lock(m_document);
                document->selectionDidChangeNotifier.removeObserver(this, &ClipTool::selectionDidChange);
                document->nodesWillChangeNotifier.removeObserver(this, &ClipTool::nodesWillChange);
                document->nodesDidChangeNotifier.removeObserver(this, &ClipTool::nodesDidChange);
            }
        }
        
        void ClipTool::selectionDidChange(const Selection& selection) {
            update();
        }
        
        void ClipTool::nodesWillChange(const Model::NodeList& nodes) {
            update();
        }
        
        void ClipTool::nodesDidChange(const Model::NodeList& nodes) {
            update();
        }

        void ClipTool::update() {
        }
    }
}
