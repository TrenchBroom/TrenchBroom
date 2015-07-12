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

#include "OffscreenRenderer.h"

#include <cassert>
#include <cstring>

namespace TrenchBroom {
    namespace Renderer {
        OffscreenRenderer::OffscreenRenderer(bool multisample, GLint samples) :
        m_framebufferId(0),
        m_colorbufferId(0),
        m_depthbufferId(0),
        m_valid(false),
        m_width(0),
        m_height(0),
        m_multisample(multisample),
        m_samples(samples),
        m_readBuffers(NULL) {}

        OffscreenRenderer::~OffscreenRenderer() {
            if (m_depthbufferId != 0) {
                glDeleteRenderbuffers(1, &m_depthbufferId);
                m_depthbufferId = 0;
            }
            if (m_colorbufferId != 0) {
                glDeleteRenderbuffers(1, &m_colorbufferId);
                m_colorbufferId = 0;
            }
            if (m_framebufferId != 0) {
                glDeleteFramebuffers(1, &m_framebufferId);
                m_framebufferId = 0;
            }
            if (m_readBuffers != NULL) {
                delete m_readBuffers;
                m_readBuffers = NULL;
            }
        }

        void OffscreenRenderer::preRender() {
            assert(m_width > 0 && m_height > 0);

            // scampie's Vista machine crashes between here...
            if (m_framebufferId == 0)
                glGenFramebuffers(1, &m_framebufferId); // most likely here
            if (!m_valid) {
                if (m_colorbufferId != 0) {
                    glDeleteRenderbuffers(1, &m_colorbufferId);
                    m_colorbufferId = 0;
                }
                if (m_depthbufferId != 0) {
                    glDeleteRenderbuffers(1, &m_depthbufferId);
                    m_depthbufferId = 0;
                }
            }
            // ...and here

            if (m_colorbufferId == 0)
                glGenRenderbuffers(1, &m_colorbufferId);
            if (m_depthbufferId == 0)
                glGenRenderbuffers(1, &m_depthbufferId);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebufferId);

            glBindRenderbuffer(GL_RENDERBUFFER, m_colorbufferId);
            if (!m_valid) {
                if (m_multisample)
                    glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, GL_RGBA, static_cast<GLint>(m_width), static_cast<GLint>(m_height));
                else
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, static_cast<GLint>(m_width), static_cast<GLint>(m_height));
            }
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_colorbufferId);

            glBindRenderbuffer(GL_RENDERBUFFER, m_depthbufferId);
            if (!m_valid) {
                if (m_multisample)
                    glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_samples, GL_DEPTH_COMPONENT, static_cast<GLint>(m_width), static_cast<GLint>(m_height));
                else
                    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, static_cast<GLint>(m_width), static_cast<GLint>(m_height));
            }
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthbufferId);

            GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            assert(status == GL_FRAMEBUFFER_COMPLETE);

            m_valid = true;
        }

        void OffscreenRenderer::postRender() {
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        wxImage* OffscreenRenderer::getImage() {
            assert(m_valid);

            if (m_multisample && m_samples > 0) {
                if (m_readBuffers == NULL)
                    m_readBuffers = new OffscreenRenderer(false);

                m_readBuffers->setDimensions(m_width, m_height);
                m_readBuffers->preRender();

                glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebufferId);
                glBlitFramebuffer(0, 0, static_cast<GLint>(m_width), static_cast<GLint>(m_height),
                                  0, 0, static_cast<GLint>(m_width), static_cast<GLint>(m_height),
                                  GL_COLOR_BUFFER_BIT, GL_LINEAR);

                m_readBuffers->postRender();
                return m_readBuffers->getImage();
            }

            glBindFramebuffer(GL_READ_FRAMEBUFFER, m_framebufferId);

            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glPixelStorei(GL_PACK_ROW_LENGTH, 0);
            glPixelStorei(GL_PACK_SKIP_ROWS, 0);
            glPixelStorei(GL_PACK_SKIP_PIXELS, 0);

            unsigned char* imageData = new unsigned char[m_width * m_height * 3];
            unsigned char* alphaData = new unsigned char[m_width * m_height];
            glReadPixels(0, 0, static_cast<GLint>(m_width), static_cast<GLint>(m_height), GL_RGB, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid*>(imageData));
            glReadPixels(0, 0, static_cast<GLint>(m_width), static_cast<GLint>(m_height), GL_ALPHA, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid*>(alphaData));

            /*
            unsigned char* imageLine = new unsigned char[m_width * 3];
            unsigned char* alphaLine = new unsigned char[m_width];

            for (unsigned int y = 0; y < m_height / 2; y++) {
                size_t sourceLine = y * m_width;
                size_t destLine = (m_height - y - 1) * m_width;

                memcpy(imageLine, &imageData[destLine * 3], m_width * 3);
                memcpy(&imageData[destLine * 3], &imageData[sourceLine * 3], m_width * 3);
                memcpy(&imageData[sourceLine * 3], imageLine, m_width * 3);

                memcpy(alphaLine, &alphaData[destLine], m_width);
                memcpy(&alphaData[destLine], &alphaData[sourceLine], m_width);
                memcpy(&alphaData[sourceLine], alphaLine, m_width);
            }

            delete [] imageLine;
            imageLine = NULL;
            delete [] alphaLine;
            alphaLine = NULL;
             */

            return new wxImage(static_cast<int>(m_width), static_cast<int>(m_height), imageData, alphaData);
        }
    }
}
