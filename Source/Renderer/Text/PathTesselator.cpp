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

#include "PathTesselator.h"

#include "Renderer/Text/Path.h"

#include <cstdio>

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
#if defined _WIN32
            typedef GLvoid (__stdcall *GluTessCallbackType)();
#else
            typedef GLvoid (*GluTessCallbackType)();
#endif

            namespace PathTesselatorCallback {
                typedef std::vector<Vec2f*> PointList;
                static PointList tempPoints;

                TESS_CALLBACK gluTessBeginData(GLenum type, PathMesh* mesh) {
                    mesh->begin(type);
                }

                TESS_CALLBACK gluTessVertexData(Vec2f* vertex, PathMesh* mesh) {
                    mesh->append(*vertex);
                }

                TESS_CALLBACK gluTessCombineData(GLdouble coords[3], void *vertexData[4], GLfloat weight[4], void **outData, PathMesh* mesh) {
                    Vec2f* vertex = new Vec2f();
                    vertex->x = static_cast<float>(coords[0]);
                    vertex->y = static_cast<float>(coords[1]);
                    *outData = vertex;
                    tempPoints.push_back(vertex);
                }

                TESS_CALLBACK gluTessEndData(PathMesh* mesh) {
                    mesh->end();
                }

				TESS_CALLBACK gluTessError(GLenum errorCode) {
					const GLubyte *estring;

					estring = gluErrorString(errorCode);
					fprintf (stderr, "Tessellation Error: %s\n", estring);
				}
			}

            PathTesselator::PathTesselator() :
            m_gluTess(NULL) {}

            PathTesselator::~PathTesselator() {
                if (m_gluTess != NULL) {
                    gluDeleteTess(m_gluTess);
                    m_gluTess = NULL;
                }
            }

            PathMeshPtr PathTesselator::tesselate(const Path* path) {
                if (m_gluTess == NULL) {
                    m_gluTess = gluNewTess();
                    gluTessProperty(m_gluTess, GLU_TESS_BOUNDARY_ONLY, GL_FALSE);
                    gluTessProperty(m_gluTess, GLU_TESS_TOLERANCE, 0);

					gluTessCallback(m_gluTess, GLU_TESS_BEGIN,			NULL);
					gluTessCallback(m_gluTess, GLU_TESS_BEGIN_DATA,		reinterpret_cast<GluTessCallbackType>(PathTesselatorCallback::gluTessBeginData));
					gluTessCallback(m_gluTess, GLU_TESS_VERTEX,			NULL);
                    gluTessCallback(m_gluTess, GLU_TESS_VERTEX_DATA,		reinterpret_cast<GluTessCallbackType>(PathTesselatorCallback::gluTessVertexData));
					gluTessCallback(m_gluTess, GLU_TESS_EDGE_FLAG,		NULL);
					gluTessCallback(m_gluTess, GLU_TESS_EDGE_FLAG_DATA,	NULL);
					gluTessCallback(m_gluTess, GLU_TESS_COMBINE,		NULL);
                    gluTessCallback(m_gluTess, GLU_TESS_COMBINE_DATA,	reinterpret_cast<GluTessCallbackType>(PathTesselatorCallback::gluTessCombineData));
					gluTessCallback(m_gluTess, GLU_TESS_END,			NULL);
                    gluTessCallback(m_gluTess, GLU_TESS_END_DATA,		reinterpret_cast<GluTessCallbackType>(PathTesselatorCallback::gluTessEndData));
					gluTessCallback(m_gluTess, GLU_TESS_ERROR,			reinterpret_cast<GluTessCallbackType>(PathTesselatorCallback::gluTessError));
					gluTessCallback(m_gluTess, GLU_TESS_ERROR_DATA,		NULL);

                    gluTessNormal(m_gluTess, 0, 0, -1);
                }

                PathMesh* mesh = new PathMesh();
                GLdouble coords[3];
                coords[2] = 0.0;

                const PathPolygons& polygons = path->polygons();
                for (unsigned int i = 0; i < polygons.size(); i++) {
                    const PathPolygon& polygon = *polygons[i];

                    if (polygon.winding() == PathPolygon::NonZero)
                        gluTessProperty(m_gluTess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);
                    else
                        gluTessProperty(m_gluTess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);

                    gluTessBeginPolygon(m_gluTess, mesh);
                    const PathContours& contours = polygon.contours();
                    for (unsigned int j = 0; j < contours.size(); j++) {
                        const PathContour& contour = *contours[j];

                        gluTessBeginContour(m_gluTess);
                        const PathPoints& points = contour.points();
                        PathPoints::const_iterator it, end;
                        for (it = points.begin(), end = points.end(); it != end; ++it) {
                            const Vec2f& point = *it;
                            coords[0] = point.x;
                            coords[1] = point.y;
                            gluTessVertex(m_gluTess, coords, const_cast<Vec2f*>(&point));
                        }
                        gluTessEndContour(m_gluTess);
                    }
                    gluTessEndPolygon(m_gluTess);
                }

                // clean up
                while (!PathTesselatorCallback::tempPoints.empty()) {
                    delete PathTesselatorCallback::tempPoints.back();
                    PathTesselatorCallback::tempPoints.pop_back();
                }

                return PathMeshPtr(mesh);
            }
        }
    }
}
