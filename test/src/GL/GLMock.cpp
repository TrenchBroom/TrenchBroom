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

#include "GLMock.h"

namespace TrenchBroom {
    GLMock::GLMock() {
        glewInitialize.bindMemFunc(this, &GLMock::GlewInitialize);

        glGetError.bindMemFunc(this, &GLMock::GetError);
        glGetString.bindMemFunc(this, &GLMock::GetString);
        
        glEnable.bindMemFunc(this, &GLMock::Enable);
        glDisable.bindMemFunc(this, &GLMock::Disable);
        glClear.bindMemFunc(this, &GLMock::Clear);
        glClearColor.bindMemFunc(this, &GLMock::ClearColor);
        
        glViewport.bindMemFunc(this, &GLMock::Viewport);
        
        glBlendFunc.bindMemFunc(this, &GLMock::BlendFunc);
        glShadeModel.bindMemFunc(this, &GLMock::ShadeModel);
        
        glDepthFunc.bindMemFunc(this, &GLMock::DepthFunc);
        glDepthMask.bindMemFunc(this, &GLMock::DepthMask);
        glDepthRange.bindMemFunc(this, &GLMock::DepthRange);
        
        glLineWidth.bindMemFunc(this, &GLMock::LineWidth);
        glPolygonMode.bindMemFunc(this, &GLMock::PolygonMode);
        glFrontFace.bindMemFunc(this, &GLMock::FrontFace);
        
        glLoadIdentity.bindMemFunc(this, &GLMock::LoadIdentity);
        glLoadMatrixd.bindMemFunc(this, &GLMock::LoadMatrixd);
        glLoadMatrixf.bindMemFunc(this, &GLMock::LoadMatrixf);
        glMatrixMode.bindMemFunc(this, &GLMock::MatrixMode);
        
        glGetIntegerv.bindMemFunc(this, &GLMock::GetIntegerv);
        
        glPixelStoref.bindMemFunc(this, &GLMock::PixelStoref);
        glPixelStorei.bindMemFunc(this, &GLMock::PixelStorei);
        
        glGenTextures.bindMemFunc(this, &GLMock::GenTextures);
        glDeleteTextures.bindMemFunc(this, &GLMock::DeleteTextures);
        glBindTexture.bindMemFunc(this, &GLMock::BindTexture);
        glTexParameterf.bindMemFunc(this, &GLMock::TexParameterf);
        glTexParameteri.bindMemFunc(this, &GLMock::TexParameteri);
        glTexImage2D.bindMemFunc(this, &GLMock::TexImage2D);
        glActiveTexture.bindMemFunc(this, &GLMock::ActiveTexture);
        
        glGenBuffers.bindMemFunc(this, &GLMock::GenBuffers);
        glDeleteBuffers.bindMemFunc(this, &GLMock::DeleteBuffers);
        glBindBuffer.bindMemFunc(this, &GLMock::BindBuffer);
        glBufferData.bindMemFunc(this, &GLMock::BufferData);
        glBufferSubData.bindMemFunc(this, &GLMock::BufferSubData);
        glMapBuffer.bindMemFunc(this, &GLMock::MapBuffer);
        glUnmapBuffer.bindMemFunc(this, &GLMock::UnmapBuffer);
        
        glEnableVertexAttribArray.bindMemFunc(this, &GLMock::EnableVertexAttribArray);
        glDisableVertexAttribArray.bindMemFunc(this, &GLMock::DisableVertexAttribArray);
        glEnableClientState.bindMemFunc(this, &GLMock::EnableClientState);
        glDisableClientState.bindMemFunc(this, &GLMock::DisableClientState);
        glClientActiveTexture.bindMemFunc(this, &GLMock::ClientActiveTexture);
        
        glVertexAttribPointer.bindMemFunc(this, &GLMock::VertexAttribPointer);
        glVertexPointer.bindMemFunc(this, &GLMock::VertexPointer);
        glNormalPointer.bindMemFunc(this, &GLMock::NormalPointer);
        glColorPointer.bindMemFunc(this, &GLMock::ColorPointer);
        glTexCoordPointer.bindMemFunc(this, &GLMock::TexCoordPointer);
        
        glDrawArrays.bindMemFunc(this, &GLMock::DrawArrays);
        glMultiDrawArrays.bindMemFunc(this, &GLMock::MultiDrawArrays);
        
        glCreateShader.bindMemFunc(this, &GLMock::CreateShader);
        glDeleteShader.bindMemFunc(this, &GLMock::DeleteShader);
        glShaderSource.bindMemFunc(this, &GLMock::ShaderSource);
        glCompileShader.bindMemFunc(this, &GLMock::CompileShader);
        glGetShaderiv.bindMemFunc(this, &GLMock::GetShaderiv);
        glGetShaderInfoLog.bindMemFunc(this, &GLMock::GetShaderInfoLog);
        glAttachShader.bindMemFunc(this, &GLMock::AttachShader);
        glDetachShader.bindMemFunc(this, &GLMock::DetachShader);
        
        glCreateProgram.bindMemFunc(this, &GLMock::CreateProgram);
        glDeleteProgram.bindMemFunc(this, &GLMock::DeleteProgram);
        glLinkProgram.bindMemFunc(this, &GLMock::LinkProgram);
        glGetProgramiv.bindMemFunc(this, &GLMock::GetProgramiv);
        glGetProgramInfoLog.bindMemFunc(this, &GLMock::GetProgramInfoLog);
        glUseProgram.bindMemFunc(this, &GLMock::UseProgram);
        
        glUniform1f.bindMemFunc(this, &GLMock::Uniform1f);
        glUniform2f.bindMemFunc(this, &GLMock::Uniform2f);
        glUniform3f.bindMemFunc(this, &GLMock::Uniform3f);
        glUniform4f.bindMemFunc(this, &GLMock::Uniform4f);
        glUniform1i.bindMemFunc(this, &GLMock::Uniform1i);
        glUniform2i.bindMemFunc(this, &GLMock::Uniform2i);
        glUniform3i.bindMemFunc(this, &GLMock::Uniform3i);
        glUniform4i.bindMemFunc(this, &GLMock::Uniform4i);
        glUniform1ui.bindMemFunc(this, &GLMock::Uniform1ui);
        glUniform2ui.bindMemFunc(this, &GLMock::Uniform2ui);
        glUniform3ui.bindMemFunc(this, &GLMock::Uniform3ui);
        glUniform4ui.bindMemFunc(this, &GLMock::Uniform4ui);
        
        glUniform1fv.bindMemFunc(this, &GLMock::Uniform1fv);
        glUniform2fv.bindMemFunc(this, &GLMock::Uniform2fv);
        glUniform3fv.bindMemFunc(this, &GLMock::Uniform3fv);
        glUniform4fv.bindMemFunc(this, &GLMock::Uniform4fv);
        glUniform1iv.bindMemFunc(this, &GLMock::Uniform1iv);
        glUniform2iv.bindMemFunc(this, &GLMock::Uniform2iv);
        glUniform3iv.bindMemFunc(this, &GLMock::Uniform3iv);
        glUniform4iv.bindMemFunc(this, &GLMock::Uniform4iv);
        glUniform1uiv.bindMemFunc(this, &GLMock::Uniform1uiv);
        glUniform2uiv.bindMemFunc(this, &GLMock::Uniform2uiv);
        glUniform3uiv.bindMemFunc(this, &GLMock::Uniform3uiv);
        glUniform4uiv.bindMemFunc(this, &GLMock::Uniform4uiv);
        
        glUniformMatrix2fv.bindMemFunc(this, &GLMock::UniformMatrix2fv);
        glUniformMatrix3fv.bindMemFunc(this, &GLMock::UniformMatrix3fv);
        glUniformMatrix4fv.bindMemFunc(this, &GLMock::UniformMatrix4fv);
        glUniformMatrix2x3fv.bindMemFunc(this, &GLMock::UniformMatrix2x3fv);
        glUniformMatrix3x2fv.bindMemFunc(this, &GLMock::UniformMatrix3x2fv);
        glUniformMatrix2x4fv.bindMemFunc(this, &GLMock::UniformMatrix2x4fv);
        glUniformMatrix4x2fv.bindMemFunc(this, &GLMock::UniformMatrix4x2fv);
        glUniformMatrix3x4fv.bindMemFunc(this, &GLMock::UniformMatrix3x4fv);
        glUniformMatrix4x3fv.bindMemFunc(this, &GLMock::UniformMatrix4x3fv);
        
        glGetUniformLocation.bindMemFunc(this, &GLMock::GetUniformLocation);
        
#ifdef __APPLE__
        glFinishObjectAPPLE.bindMemFunc(this, &GLMock::FinishObjectAPPLE);
#endif
    }
}
