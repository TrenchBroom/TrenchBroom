//
//  GLStringData.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenGL/gl.h>

@class FloatData;

@interface GLStringData : NSObject {
    GLenum type;
    int vertexCount;
    FloatData* triangleSet;
    NSMutableArray* triangleStrips;
    NSMutableArray* triangleFans;
}

- (void)begin:(GLenum)theType;
- (void)appendVertex:(NSPoint *)vertex;
- (void)end;

- (FloatData *)triangleSet;
- (NSArray *)triangleStrips;
- (NSArray *)triangleFans;
- (int)vertexCount;

@end
