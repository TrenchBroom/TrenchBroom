//
//  GLStringData.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "GLStringData.h"
#import "FloatData.h"

@implementation GLStringData

- (void)begin:(GLenum)theType {
    type = theType;
    
    switch (type) {
        case GL_TRIANGLES:
            if (triangleSet == nil)
                triangleSet = [[FloatData alloc] init];
            break;
        case GL_TRIANGLE_STRIP:
            if (triangleStrips == nil)
                triangleStrips = [[NSMutableArray alloc] init];
            FloatData* stripData = [[FloatData alloc] init];
            [triangleStrips addObject:stripData];
            [stripData release];
        case GL_TRIANGLE_FAN:
            if (triangleFans == nil)
                triangleFans = [[NSMutableArray alloc] init];
            FloatData* fanData = [[FloatData alloc] init];
            [triangleFans addObject:fanData];
            [fanData release];
            break;
        default:
            [NSException raise:NSInvalidArgumentException format:@"unknown GL primitive type: %i", theType];
            break;
    }
}

- (void)appendVertex:(NSPoint *)vertex{
    switch (type) {
        case GL_TRIANGLES:
            [triangleSet appendFloat:vertex->x];
            [triangleSet appendFloat:vertex->y];
            break;
        case GL_TRIANGLE_STRIP: {
            FloatData* stripData = [triangleStrips lastObject];
            [stripData appendFloat:vertex->x];
            [stripData appendFloat:vertex->y];
            break;
        }
        case GL_TRIANGLE_FAN: {
            FloatData* fanData = [triangleFans lastObject];
            [fanData appendFloat:vertex->x];
            [fanData appendFloat:vertex->y];
            break;
        }
        default:
            break;
    }
    vertexCount++;
}

- (void)end {
    // nothing to do
}

- (FloatData *)triangleSet {
    return triangleSet;
}

- (NSArray *)triangleStrips {
    return triangleStrips;
}

- (NSArray *)triangleFans {
    return triangleFans;
}

- (int)vertexCount {
    return vertexCount;
}

@end
