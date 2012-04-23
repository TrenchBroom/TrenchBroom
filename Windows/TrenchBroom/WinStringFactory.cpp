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

#include "WinStringFactory.h"
#include "GL/GLee.h"

namespace TrenchBroom {
	namespace Renderer {
		typedef void (__stdcall *GluTessCallbackType)();

		namespace StringFactoryCallback {
            void CALLBACK gluTessBeginData(GLenum type, StringData* data) {
                data->begin(type);
            }
            
            void CALLBACK gluTessVertexData(StringData::Point* vertex, StringData* data) {
                StringData::Point point (vertex->x, vertex->y);
                data->append(point);
            }
            
            void CALLBACK gluTessCombineData(GLdouble coords[3], void *vertexData[4], GLfloat weight[4], void **outData, StringData* data) {
				StringData::Point* vertex = new StringData::Point();
                vertex->x = coords[0];
                vertex->y = coords[1];
                *outData = vertex;
				fprintf(stdout, "%li", outData);
            }
            
            void CALLBACK gluTessEndData(StringData* data) {
                data->end();
            }

			void CALLBACK gluTessError(GLenum errorCode) {
				const GLubyte *estring;

				estring = gluErrorString(errorCode);
				fprintf (stderr, "Tessellation Error: %s\n", estring);
				exit (0);
			}
        }

		WinStringFactory::WinStringFactory(HDC mainDC) : m_gluTess(NULL) {
			m_dc = CreateCompatibleDC(mainDC);
		}


		WinStringFactory::~WinStringFactory() {
			if (m_gluTess != NULL)
				gluDeleteTess(m_gluTess);
			DeleteDC(m_dc);
		}

		StringData* WinStringFactory::createStringData(const FontDescriptor& descriptor, const string& str) {
			if (m_gluTess == NULL) {
                m_gluTess = gluNewTess();
                gluTessProperty(m_gluTess, GLU_TESS_BOUNDARY_ONLY, GL_FALSE);
                gluTessProperty(m_gluTess, GLU_TESS_TOLERANCE, 0);
                
				gluTessCallback(m_gluTess, GLU_TESS_BEGIN,			NULL);
                gluTessCallback(m_gluTess, GLU_TESS_BEGIN_DATA,		reinterpret_cast<GluTessCallbackType>(StringFactoryCallback::gluTessBeginData));
				gluTessCallback(m_gluTess, GLU_TESS_VERTEX,			NULL);
                gluTessCallback(m_gluTess, GLU_TESS_VERTEX_DATA,	reinterpret_cast<GluTessCallbackType>(StringFactoryCallback::gluTessVertexData));
				gluTessCallback(m_gluTess, GLU_TESS_EDGE_FLAG,		NULL);
				gluTessCallback(m_gluTess, GLU_TESS_EDGE_FLAG_DATA, NULL);
				gluTessCallback(m_gluTess, GLU_TESS_COMBINE,		NULL);
                gluTessCallback(m_gluTess, GLU_TESS_COMBINE_DATA,	reinterpret_cast<GluTessCallbackType>(StringFactoryCallback::gluTessCombineData));
				gluTessCallback(m_gluTess, GLU_TESS_END,			NULL);
                gluTessCallback(m_gluTess, GLU_TESS_END_DATA,		reinterpret_cast<GluTessCallbackType>(StringFactoryCallback::gluTessEndData));
				gluTessCallback(m_gluTess, GLU_TESS_ERROR,			reinterpret_cast<GluTessCallbackType>(StringFactoryCallback::gluTessError));
				gluTessCallback(m_gluTess, GLU_TESS_ERROR_DATA,		NULL);
                gluTessNormal(m_gluTess, 0, 0, -1);
            }

			TCHAR* fontName = new TCHAR[descriptor.name.length()];
			for (int i = 0; i < descriptor.name.length(); i++)
				fontName[i] = descriptor.name[i];

			TCHAR* wstr = new TCHAR[str.length()];
			for (int i = 0; i < str.length(); i++)
				wstr[i] = str[i];

			float scale = 1.0f;

			HFONT font = CreateFont(scale * descriptor.size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE, fontName);
			SelectObject(m_dc, font);
			SIZE size;
			GetTextExtentPoint32(m_dc, wstr, str.length(), &size);

			BeginPath(m_dc);
			TextOut(m_dc, 0, 0, wstr, str.length());
			EndPath(m_dc);
			FlattenPath(m_dc);

			int winding = GetPolyFillMode(m_dc);
			if (winding == ALTERNATE)
				gluTessProperty(m_gluTess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
			else
				gluTessProperty(m_gluTess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);

			const int numPoints = GetPath(m_dc, NULL, NULL, 0);
			LPPOINT pathPoints = new POINT[numPoints];
			LPBYTE pathTypes = new BYTE[numPoints];

			StringData::Point* stringPoints = new StringData::Point[numPoints - 4];

			GetPath(m_dc, pathPoints, pathTypes, numPoints);

			StringData* stringData = new StringData(size.cx / scale, size.cy / scale);
			double coords[2];

			int contourSize = 0;
			gluTessBeginPolygon(m_gluTess, stringData);
			for (int i = 0; i < numPoints - 4; i++) {
				stringPoints[i].x = pathPoints[i + 4].x / scale;
				stringPoints[i].y = stringData->height - (pathPoints[i + 4].y / scale);
				if (pathTypes[i + 4] == PT_MOVETO) {
					if (contourSize > 0) {
						gluTessEndContour(m_gluTess);
						contourSize = 0;
					}

					gluTessBeginContour(m_gluTess);
					coords[0] = stringPoints[i].x;
                    coords[1] = stringPoints[i].y;
                    gluTessVertex(m_gluTess, coords, &stringPoints[i]);
					contourSize++;
				} else {
					if ((pathTypes[i + 4] & PT_LINETO) != 0) {
						coords[0] = stringPoints[i].x;
						coords[1] = stringPoints[i].y;
		                gluTessVertex(m_gluTess, coords, &stringPoints[i]);
						contourSize++;
					}
					if ((pathTypes[i + 4] & PT_CLOSEFIGURE) != 0) {
						gluTessEndContour(m_gluTess);
						contourSize = 0;
					}
				}
			}
			if (contourSize > 0)
				gluTessEndContour(m_gluTess);
			gluTessEndPolygon(m_gluTess);

			DeleteObject(font);
			delete [] fontName;
			delete [] wstr;
			delete [] pathPoints;
			delete [] pathTypes;
			delete [] stringPoints;

			return stringData;
		}
	}
}
