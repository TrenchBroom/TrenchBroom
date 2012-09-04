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

#include "StringVectorizer.h"

#include "IO/FileManager.h"
#include "Renderer/Text/PathBuilder.h"
#include "Utility/Console.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        namespace Text {
            FT_Face StringVectorizer::makeFont(const FontDescriptor& fontDescriptor) {
                FontCache::iterator it = m_fontCache.find(fontDescriptor);
                if (it != m_fontCache.end())
                    return it->second;
                
                IO::FileManager fileManager;
                String fontPath = fileManager.resolveFontPath(fontDescriptor.name());
                
                FT_Face face;
                FT_Error error = FT_New_Face(m_library, fontPath.c_str(), 0, &face);
                if (error != 0) {
                    m_console.error("Error loading font '%s', size %i (FT error: %i)", fontDescriptor.name().c_str(), fontDescriptor.size(), error);
                    return NULL;
                }
                
                FT_Set_Pixel_Sizes(face, 0, fontDescriptor.size());
                m_fontCache[fontDescriptor] = face;
                return face;
            }

            StringVectorizer::StringVectorizer(Utility::Console& console) :
            m_console(console),
            m_library(NULL) {
                FT_Error error = FT_Init_FreeType(&m_library);
                if (error != 0) {
                    m_console.error("Error initializing FreeType (FT error: %i)", error);
                    m_library = NULL;
                }
            }
            
            StringVectorizer::~StringVectorizer() {
                FontCache::iterator it, end;
                for (it = m_fontCache.begin(), end = m_fontCache.end(); it != end; ++it)
                    FT_Done_Face(it->second);
                m_fontCache.clear();
                
                if (m_library != NULL) {
                    FT_Done_FreeType(m_library);
                    m_library = NULL;
                }
            }
            
            PathPtr StringVectorizer::makePath(const FontDescriptor& fontDescriptor, const String& string) {
                FT_Face face = makeFont(fontDescriptor);
                if (face == NULL)
                    return PathPtr(NULL);
                
                float width = 0.0f;
                float height = static_cast<float>(fontDescriptor.size());

                Path* path = new Path();
                PathBuilder pathBuilder(path);
                
                for (unsigned int i = 0; i < string.length(); i++) {
                    char c = string[i];
                    FT_Error error = FT_Load_Char(face, c, FT_LOAD_NO_BITMAP);
                    if (error != 0) {
                        m_console.error("Error loading glyph (FT error: %i)", error);
                        delete path;
                        return PathPtr(NULL);
                    }
                    
                    FT_GlyphSlot glyph = face->glyph;
                    width += glyph->metrics.horiAdvance;
                    
                    FT_Outline* outline = &glyph->outline;
                    
                    unsigned int numContours = outline->n_contours;
                    unsigned int numPoints = outline->n_points;
                    PathContour::Winding winding = (outline->flags & FT_OUTLINE_EVEN_ODD_FILL) ? PathContour::EvenOdd : PathContour::NonZero;
                    
                    unsigned int start = 0;
                    unsigned int end = 0;
                    unsigned int count;
                    
                    Vec2f previousPoint, currentPoint, nextPoint, nextNextPoint;
                    
                    
                    for (unsigned int j = 0; j < numContours; j++) {
                        start = end;
                        end = outline->contours[j] + 1;
                        count = end - start;

                        setPoint(outline->points, pred(start, count), previousPoint);
                        setPoint(outline->points, start, currentPoint);
                        
                        pathBuilder.beginContour(winding);
                        for (unsigned int k = start; k < end; k++) {
                            setPoint(outline->points, succ(k, count), nextPoint);
                            
                            if (linearPoint(outline->tags[k])) {
                                pathBuilder.addPoint(currentPoint);
                            } else if (quadraticBezierPoint(outline->tags[k])) {
                                Vec2f startPoint, endPoint;
                                if (quadraticBezierPoint(outline->tags[pred(k, count)])) {
                                    startPoint = (currentPoint + previousPoint) / 2.0f;
                                    pathBuilder.addPoint(startPoint);
                                } else {
                                    startPoint = previousPoint;
                                }
                                
                                if (quadraticBezierPoint(outline->tags[succ(k, count)])) {
                                    endPoint = (currentPoint + nextPoint) / 2.0f;
                                } else {
                                    endPoint = nextPoint;
                                }
                                
                                pathBuilder.addQuadraticBezierCurve(startPoint, currentPoint, endPoint);
                            } else if (cubicBezierPoint(outline->tags[k]) &&
                                       cubicBezierPoint(outline->tags[succ(k, count)])) {
                                setPoint(outline->points, succ(k, count, 2), nextNextPoint);
                                pathBuilder.addCubicBezierCurve(previousPoint, currentPoint, nextPoint, nextNextPoint);
                            }

                            previousPoint = currentPoint;
                            currentPoint = nextPoint;
                        }
                        
                        pathBuilder.endContour();
                    }
                }
                
                path->setBounds(width, height);
                return PathPtr(path);
            }
        }
    }
}