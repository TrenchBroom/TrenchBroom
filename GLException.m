//
//  GLException.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GLException.h"
#import <OpenGL/GL.h>

@implementation GLException

+ (void)raise {
    GLenum error = glGetError();
    NSString* reason = nil;
    switch (error) {
        case GL_INVALID_ENUM:
            reason = @"GL_INVALID_ENUM (An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.)";
            break;
        case GL_INVALID_VALUE:
            reason = @"GL_INVALID_VALUE (A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.)";
            break;
        case GL_INVALID_OPERATION:
            reason = @"GL_INVALID_OPERATION (The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.)";
            break;
        case GL_STACK_OVERFLOW:
            reason = @"GL_STACK_OVERFLOW (This command would cause a stack overflow. The offending command is ignored and has no other side effect than to set the error flag.)";
            break;
        case GL_STACK_UNDERFLOW:
            reason = @"GL_STACK_UNDERFLOW (This command would cause a stack underflow. The offending command is ignored and has no other side effect than to set the error flag.)";
            break;
        case GL_OUT_OF_MEMORY:
            reason = @"GL_OUT_OF_MEMORY (There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.)";
            break;
        case GL_TABLE_TOO_LARGE:
            reason = @"GL_TABLE_TOO_LARGE (The specified table exceeds the implementation's maximum supported table size. The offending command is ignored and has no other side effect than to set the error flag.)";
            break;
        default:
            reason = [NSString stringWithFormat:@"Unknown OpenGL error: %i", error];
            break;
    }
    
    [NSException raise:@"GLException" format:@"%@", reason];
}

@end
