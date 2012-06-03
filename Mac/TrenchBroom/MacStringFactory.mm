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

#import "MacStringFactory.h"

namespace TrenchBroom {
    namespace Renderer {
		typedef GLvoid (*GluTessCallbackType)();
        
        namespace StringFactoryCallback {
            void gluTessBeginData(GLenum type, StringData* data) {
                data->begin(type);
            }
            
            void gluTessVertexData(NSPoint* vertex, StringData* data) {
                StringData::Point point (vertex->x, vertex->y);
                data->append(point);
            }
            
            void gluTessCombineData(GLdouble coords[3], void *vertexData[4], GLfloat weight[4], void **outData, StringData* data) {
                NSPoint* vertex = new NSPoint();
                vertex->x = coords[0];
                vertex->y = coords[1];
                *outData = vertex;
                tempPoints.push_back(vertex);
            }
            
            void gluTessEndData(StringData* data) {
                data->end();
            }
        }
        
        MacStringFactory::MacStringFactory() : m_gluTess(NULL), m_points(NULL) {
            m_textStorage = [[NSTextStorage alloc] init];
            m_textContainer = [[NSTextContainer alloc] init];
            m_layoutManager = [[NSLayoutManager alloc] init];
            
            [m_layoutManager addTextContainer:m_textContainer];
            [m_textStorage addLayoutManager:m_layoutManager];
            
            [m_textContainer setLineFragmentPadding:0];
        }

        MacStringFactory::~MacStringFactory() {
            [m_textStorage release];
            [m_textContainer release];
            [m_layoutManager release];
            if (m_gluTess != NULL)
                gluDeleteTess(m_gluTess);
        }

        StringData* MacStringFactory::createStringData(const FontDescriptor& descriptor, const std::string& str) {
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
            
            NSString* fontName = [NSString stringWithCString:descriptor.name.c_str() encoding:NSASCIIStringEncoding];
            NSFont* font = [NSFont fontWithName:fontName size:descriptor.size];
            
            NSMutableDictionary* attrs = [[NSMutableDictionary alloc] init];
            [attrs setObject:font forKey:NSFontAttributeName];
            
            NSString* objCStr = [NSString stringWithCString:str.c_str() encoding:NSASCIIStringEncoding];
            NSAttributedString* attrString = [[NSAttributedString alloc] initWithString:objCStr attributes:attrs];
            [m_textStorage setAttributedString:attrString];
            [attrString release];
            [attrs release];
            
            [m_layoutManager ensureLayoutForTextContainer:m_textContainer];
            
            NSRange glyphRange = [m_layoutManager glyphRangeForCharacterRange:NSMakeRange(0, [objCStr length]) actualCharacterRange:NULL];
            NSRect bounds = [m_layoutManager boundingRectForGlyphRange:glyphRange inTextContainer:m_textContainer];
            NSGlyph* glyphs = new NSGlyph[glyphRange.length + 1];
            NSUInteger count = [m_layoutManager getGlyphs:glyphs range:glyphRange];
            
            [NSBezierPath setDefaultFlatness:0.1f];
            NSBezierPath* path = [NSBezierPath bezierPath];
            [path moveToPoint:NSMakePoint(0, 0)];
            [path appendBezierPathWithGlyphs:glyphs count:count inFont:font];
            delete [] glyphs;
            glyphs = NULL;
            
            path = [path bezierPathByFlatteningPath];
            
            NSAffineTransform* transform = [[NSAffineTransform alloc] init];
            [transform translateXBy:0 yBy:bounds.size.height - [m_layoutManager defaultBaselineOffsetForFont:font]];
            
            [path transformUsingAffineTransform:transform];
            [transform release];
            
            if ([path windingRule] == NSNonZeroWindingRule)
                gluTessProperty(m_gluTess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);
            else
                gluTessProperty(m_gluTess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
            
            StringData* stringData = new StringData(NSWidth(bounds), NSHeight(bounds));
            gluTessBeginPolygon(m_gluTess, stringData);
            
            GLdouble coords[3];
            coords[2] = 0;
            
            m_points.resize([path elementCount]);
            
            for (int i = 0; i < [path elementCount]; i++) {
                NSBezierPathElement element = [path elementAtIndex:i associatedPoints:&m_points[i]];
                // points[i].y = bounds.size.height - points[i].y;
                switch (element) {
                    case NSMoveToBezierPathElement:
                        gluTessBeginContour(m_gluTess);
                        coords[0] = m_points[i].x;
                        coords[1] = m_points[i].y;
                        gluTessVertex(m_gluTess, coords, &m_points[i]);
                        break;
                    case NSClosePathBezierPathElement:
                        gluTessEndContour(m_gluTess);
                        break;
                    case NSLineToBezierPathElement:
                        coords[0] = m_points[i].x;
                        coords[1] = m_points[i].y;
                        gluTessVertex(m_gluTess, coords, &m_points[i]);
                        break;
                    default:
                        break;
                }
            }
            gluTessEndPolygon(m_gluTess);
            
            while (!StringFactoryCallback::tempPoints.empty())
                delete StringFactoryCallback::tempPoints.back(), 
                StringFactoryCallback::tempPoints.pop_back();
            
            return stringData;
        }
    }
}