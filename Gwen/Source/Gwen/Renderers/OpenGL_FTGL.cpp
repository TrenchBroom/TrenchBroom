/*
 GWEN
 Copyright (c) 2012 Kristian Duske
 See license in Gwen.h
 */

#include "OpenGL_FTGL.h"
#include "Gwen/Platform.h"
#include "GL/GLee.h"
#include <string>
#include <fstream>
#include <cmath>

namespace Gwen {
    namespace Renderer {
        bool operator<(FontDescriptor const& left, FontDescriptor const& right) {
            int cmp = left.name.compare(right.name);
            if (cmp < 0) return true;
            if (cmp > 0) return false;
            return left.size < right.size;
        }

        OpenGL_FTGL::OpenGL_FTGL() {}

        OpenGL_FTGL::~OpenGL_FTGL() {}

        const FontInfo& OpenGL_FTGL::loadFont(Gwen::Font* pFont) {
            FontDescriptor descriptor(pFont->facename, pFont->size);
            FontCache::iterator it = m_fontCache.find(descriptor);
            if (it == m_fontCache.end()) {
                String fontPath = Gwen::Platform::ResolveFontPath(pFont);
                FTFont* font = new FTTextureFont(fontPath.c_str());
                FontPtr fontPtr(font);
                fontPtr->FaceSize(static_cast<int>(pFont->size));
                fontPtr->UseDisplayList(true);

                FTBBox bounds = font->BBox("Ayg");
                float height = bounds.Upper().Yf() - bounds.Lower().Yf();
                float offset = bounds.Upper().Yf();

                m_fontCache[descriptor] = FontInfo(fontPtr, height, offset);
                return m_fontCache[descriptor];
            } else {
                return it->second;
            }
        }

        void OpenGL_FTGL::RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text ) {
            const FontInfo& fontInfo = loadFont(pFont);
			Gwen::String convertedText = Gwen::Utility::UnicodeToString( text );

            glPushMatrix();
            int offset = static_cast<int>(ceilf(fontInfo.offset));
            glTranslatef(static_cast<float>(m_RenderOffset.x + pos.x - 1), static_cast<float>(m_RenderOffset.y + pos.y + offset + 2), 0);
            glScalef(1, -1, 1);
            fontInfo.font->Render(convertedText.c_str());
            glPopMatrix();
        }

        Gwen::Point OpenGL_FTGL::MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text ) {
            const FontInfo& fontInfo = loadFont(pFont);
			Gwen::String convertedText = Gwen::Utility::UnicodeToString( text );

            FTBBox bounds = fontInfo.font->BBox(convertedText.c_str());
            int length = static_cast<int>(ceilf(bounds.Upper().Xf() - bounds.Lower().Xf())) - 1;
            int height = static_cast<int>(ceilf(fontInfo.height)) + 2;

            return Gwen::Point(length, height);
        }
    }
}
