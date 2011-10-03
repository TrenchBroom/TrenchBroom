/*
Copyright (C) 2010-2011 Kristian Duske

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

#import "GLStringData.h"
#import "FloatData.h"

@implementation GLStringData

- (void)dealloc {
    [triangleSet release];
    [triangleStrips release];
    [triangleFans release];
    [super dealloc];
}

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
    // free(vertex);
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
