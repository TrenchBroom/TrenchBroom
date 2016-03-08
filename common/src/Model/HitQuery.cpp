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

#include "HitQuery.h"

#include "Model/EditorContext.h"
#include "Model/HitAdapter.h"
#include "Model/HitFilter.h"

namespace TrenchBroom {
    namespace Model {
        HitQuery::HitQuery(const Hit::List& hits, const EditorContext& editorContext) :
        m_hits(hits),
        m_editorContext(&editorContext),
        m_include(HitFilter::always()),
        m_exclude(HitFilter::never()) {}
        
        HitQuery::HitQuery(const Hit::List& hits) :
        m_hits(hits),
        m_editorContext(NULL),
        m_include(HitFilter::always()),
        m_exclude(HitFilter::never()) {}
        
        HitQuery::~HitQuery() {
            delete m_include;
            delete m_exclude;
        }
        
        HitQuery& HitQuery::pickable() {
            if (m_editorContext != NULL)
                m_include = new HitFilterChain(new ContextHitFilter(*m_editorContext), m_include);
            return *this;
        }

        HitQuery& HitQuery::type(const Hit::HitType type) {
            m_include = new HitFilterChain(new TypedHitFilter(type), m_include);
            return *this;
        }
        
        HitQuery& HitQuery::occluded(const Hit::HitType type) {
            delete m_exclude;
            m_exclude = new TypedHitFilter(type);
            return *this;
        }
        
        HitQuery& HitQuery::selected() {
            m_include = new HitFilterChain(new Model::SelectionHitFilter(), m_include);
            return *this;
        }
        
        HitQuery& HitQuery::minDistance(const FloatType minDistance) {
            m_include = new HitFilterChain(new Model::MinDistanceHitFilter(minDistance), m_include);
            return *this;
        }
        
        const Hit& HitQuery::first() const {
            if (!m_hits.empty()) {
                Hit::List::const_iterator it = m_hits.begin();
                Hit::List::const_iterator end = m_hits.end();
                Hit::List::const_iterator bestMatch = m_hits.end();
                FloatType bestMatchError = std::numeric_limits<FloatType>::max();
                FloatType bestOccluderError = std::numeric_limits<FloatType>::max();
                
                bool containsOccluder = false;
                while (it != end && !containsOccluder) {
                    if (!visible(*it)) { // Don't consider hidden objects during picking at all.
                        ++it;
                        continue;
                    }
                    
                    const FloatType distance = it->distance();
                    do {
                        const Hit& hit = *it;
                        if (m_include->matches(hit)) {
                            if (hit.error() < bestMatchError) {
                                bestMatch = it;
                                bestMatchError = hit.error();
                            }
                        } else if (!m_exclude->matches(hit)) {
                            bestOccluderError = std::min(bestOccluderError, hit.error());
                            containsOccluder = true;
                        }
                        ++it;
                    } while (it != end && Math::eq(it->distance(), distance));
                }
                
                if (bestMatch != m_hits.end() && bestMatchError <= bestOccluderError)
                    return *bestMatch;
            }
            return Hit::NoHit;
        }
        
        Hit::List HitQuery::all() const {
            Hit::List result;
            Hit::List::const_iterator it, end;
            for (it = m_hits.begin(), end = m_hits.end(); it != end; ++it) {
                const Hit& hit = *it;
                if (m_include->matches(hit))
                    result.push_back(hit);
            }
            return result;
        }

        bool HitQuery::visible(const Hit& hit) const {
            if (m_editorContext == NULL)
                return true;
            Node* node = hitToNode(hit);
            if (node == NULL)
                return true;
            return m_editorContext->visible(hitToNode(hit));
        }
    }
}
