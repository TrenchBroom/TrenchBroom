/*
 GWEN
 Copyright (c) 2012 Kristian Duske
 See license in Gwen.h
 */

#include "OpenGL_FTGL.h"
#include "GL/GLee.h"
#include <string>
#include <fstream>

namespace Gwen {
    namespace Renderer {
        OpenGL_FTGL::OpenGL_FTGL() {}
        OpenGL_FTGL::~OpenGL_FTGL() {}
        
        String OpenGL_FTGL::resolveFontPath(Gwen::Font* pFont) {
            String extensions[2] = {".ttf", "ttc"};
            String facename = String(pFont->facename.begin(), pFont->facename.end());

            for (int i = 0; i < 2; i++) {
                String path = "/System/Library/Fonts/" + facename + extensions[i];
                std::fstream fs1(path.c_str());
                if (fs1.is_open())
                    return path;
                
                path = "/Library/Fonts/" + facename + extensions[i];
                std::fstream fs2(path.c_str());
                if (fs2.is_open())
                    return path;
            }
            
            return "/System/Library/Fonts/LucidaGrande.ttc";
        }

        FTGL::FTGLfont* OpenGL_FTGL::loadFont(Gwen::Font* pFont) {
            FTGL::FTGLfont* ftglFont = static_cast<FTGL::FTGLfont*>(pFont->data);
            if (ftglFont == NULL) {
                String fontPath = resolveFontPath(pFont);
                ftglFont = FTGL::ftglCreatePixmapFont(fontPath.c_str());
                FTGL::ftglSetFontFaceSize(ftglFont, pFont->size, 72);
                pFont->data = ftglFont;
            }
            return ftglFont;
        }

        void OpenGL_FTGL::RenderText( Gwen::Font* pFont, Gwen::Point pos, const Gwen::UnicodeString& text ) {
            FTGL::FTGLfont* ftglFont = loadFont(pFont);
			Gwen::String convertedText = Gwen::Utility::UnicodeToString( text );
            
            float bbox[6];
            FTGL::ftglGetFontBBox(ftglFont, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, bbox);
            int height = static_cast<int>(ceilf(bbox[4]));

            glRasterPos2f(pos.x + m_RenderOffset.x - 1, pos.y + m_RenderOffset.y + height);
            FTGL::ftglRenderFont(ftglFont, convertedText.c_str(), FTGL::RENDER_ALL);
        }
        
        Gwen::Point OpenGL_FTGL::MeasureText( Gwen::Font* pFont, const Gwen::UnicodeString& text ) {
			Gwen::String convertedText = Gwen::Utility::UnicodeToString( text );
            FTGL::FTGLfont* ftglFont = loadFont(pFont);

            float bbox[6];
            FTGL::ftglGetFontBBox(ftglFont, convertedText.c_str(), convertedText.length(), bbox);
            int length = static_cast<int>((bbox[3] - bbox[0]))+2;
            FTGL::ftglGetFontBBox(ftglFont, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 52, bbox);
            int height = static_cast<int>(ceilf(bbox[4] - bbox[1]));
            return Gwen::Point(length, height);
        }
    }
}