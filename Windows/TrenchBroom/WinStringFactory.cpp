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
            void gluTessBeginData(GLenum type, StringData* data) {
                data->begin(type);
            }
            
            void gluTessVertexData(LPPOINT vertex, StringData* data) {
                StringData::Point point (vertex->x, vertex->y);
                data->append(point);
            }
            
            void gluTessCombineData(GLdouble coords[3], void *vertexData[4], GLfloat weight[4], void **outData, StringData* data) {
				LPPOINT vertex = new POINT();
                vertex->x = coords[0];
                vertex->y = coords[1];
                *outData = vertex;
            }
            
            void gluTessEndData(StringData* data) {
                data->end();
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
                
                gluTessCallback(m_gluTess, GLU_TESS_BEGIN_DATA,   reinterpret_cast<GluTessCallbackType>(StringFactoryCallback::gluTessBeginData));
                gluTessCallback(m_gluTess, GLU_TESS_VERTEX_DATA,  reinterpret_cast<GluTessCallbackType>(StringFactoryCallback::gluTessVertexData));
                gluTessCallback(m_gluTess, GLU_TESS_COMBINE_DATA, reinterpret_cast<GluTessCallbackType>(StringFactoryCallback::gluTessCombineData));
                gluTessCallback(m_gluTess, GLU_TESS_END_DATA,     reinterpret_cast<GluTessCallbackType>(StringFactoryCallback::gluTessEndData));
                gluTessNormal(m_gluTess, 0, 0, -1);
            }

			wchar_t* fontName = new wchar_t[descriptor.name.length()];
			for (int i = 0; i < descriptor.name.length(); i++)
				fontName[i] = descriptor.name[i];

			wchar_t* wstr = new wchar_t[str.length()];
			for (int i = 0; i < str.length(); i++)
				wstr[i] = str[i];

			HFONT font = CreateFont(descriptor.size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY, FF_DONTCARE, fontName);
			SelectObject(m_dc, font);
			SIZE size;
			GetTextExtentPoint32(m_dc, wstr, str.length(), &size);

			BeginPath(m_dc);
			TextOut(m_dc, 0, 0, wstr, str.length());
			EndPath(m_dc);
			FlattenPath(m_dc);

			const int numPoints = GetPath(m_dc, NULL, NULL, 0);
			LPPOINT points = new POINT[numPoints];
			LPBYTE types = new BYTE[numPoints];

			GetPath(m_dc, points, types, numPoints);

			StringData* stringData = new StringData(size.cx, size.cy);
			double coords[2];

			gluTessBeginPolygon(m_gluTess, stringData);
			for (int i = 0; i < numPoints; i++) {
				switch (types[i]) {
				case PT_MOVETO:
					gluTessBeginContour(m_gluTess);
					coords[0] = points[i].x;
                    coords[1] = points[i].y;
                    gluTessVertex(m_gluTess, coords, &points[i]);
					break;
				case PT_CLOSEFIGURE:
					gluTessEndContour(m_gluTess);
					break;
				case PT_LINETO:
					coords[0] = points[i].x;
                    coords[1] = points[i].y;
                    gluTessVertex(m_gluTess, coords, &points[i]);
					break;
				}
			}
			gluTessEndPolygon(m_gluTess);

			DeleteObject(font);
			delete [] fontName;
			delete [] wstr;
			delete [] points;
			delete [] types;

			return stringData;
		}
	}
}
