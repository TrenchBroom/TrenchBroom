//
//  ShaderProgram.h
//  TrenchBroom
//
//  Created by Kristian Duske on 09.10.12.
//  Copyright (c) 2012 Kristian Duske. All rights reserved.
//

#ifndef __TrenchBroom__ShaderProgram__
#define __TrenchBroom__ShaderProgram__

#include <GL/glew.h>
#include "Utility/String.h"
#include "Utility/VecMath.h"

#include <map>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class Texture;
    }
    
    namespace Utility {
        class Console;
    }
    
    namespace Renderer {
        class Shader;
        
        class ShaderProgram {
        private:
            typedef std::map<String, GLint> UniformVariableMap;
            
            String m_name;
            GLuint m_programId;
            UniformVariableMap m_uniformVariables;
            bool m_needsLinking;
            Utility::Console& m_console;
            
            GLint uniformLocation(const String& name);
        public:
            ShaderProgram(const String& name, Utility::Console& console);
            ~ShaderProgram();
            
            
            inline GLuint programId() const {
                return m_programId;
            }
            
            void attachShader(Shader& shader);
            void detachShader(Shader& shader);
            
            bool activate();
            void deactivate();
            
            bool setUniformVariable(const String& name, bool value);
            bool setUniformVariable(const String& name, int value);
            bool setUniformVariable(const String& name, float value);
            bool setUniformVariable(const String& name, const Vec2f& value);
            bool setUniformVariable(const String& name, const Vec3f& value);
            bool setUniformVariable(const String& name, const Vec4f& value);
            bool setUniformVariable(const String& name, const Mat2f& value);
            bool setUniformVariable(const String& name, const Mat3f& value);
            bool setUniformVariable(const String& name, const Mat4f& value);
        };
    }
}

#endif /* defined(__TrenchBroom__ShaderProgram__) */
