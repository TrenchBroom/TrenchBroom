//
//  GLFontManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 03.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GLFontManager.h"
#import <OpenGL/gl.h>
#import "VBOBuffer.h"
#import "GLString.h"
#import "GLStringData.h"
#import "Math.h"

void gluTessBeginData(GLenum type, GLStringData* data) {
    [data begin:type];
}

void gluTessVertexData(NSPoint* vertex, GLStringData* data) {
    [data appendVertex:vertex];
}

void gluTessCombineData(GLdouble coords[3], void *vertexData[4], GLfloat weight[4], void **outData, GLStringData* data) {
    NSPoint* vertex = malloc(sizeof(NSPoint));
    vertex->x = coords[0];
    vertex->y = coords[1];
    *outData = vertex;
}

void gluTessEndData(GLStringData* data) {
    [data end];
}

@interface GLFontManager (private)

- (void)resizePointArrayTo:(int)capacity;

@end

@implementation GLFontManager (private)

- (void)resizePointArrayTo:(int)capacity {
    NSPoint* temp = malloc(capacity * sizeof(NSPoint));
    memcpy(temp, points, fmin(pointCapacity, capacity) * sizeof(NSPoint));
    free(points);
    points = temp;
    pointCapacity = capacity;
}

@end

@implementation GLFontManager

- (id)init {
    if (self = [super init]) {
        vbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        glStrings = [[NSMutableDictionary alloc] init];
        
        gluTess = gluNewTess();
        gluTessProperty(gluTess, GLU_TESS_BOUNDARY_ONLY, GL_FALSE);
        gluTessProperty(gluTess, GLU_TESS_TOLERANCE, 0);
        
        gluTessCallback(gluTess, GLU_TESS_BEGIN_DATA, &gluTessBeginData);
        gluTessCallback(gluTess, GLU_TESS_VERTEX_DATA, &gluTessVertexData);
        gluTessCallback(gluTess, GLU_TESS_COMBINE_DATA, &gluTessCombineData);
        gluTessCallback(gluTess, GLU_TESS_END_DATA, &gluTessEndData);
        gluTessNormal(gluTess, 0, 0, -1);

        textStorage = [[NSTextStorage alloc] init];
        textContainer = [[NSTextContainer alloc] init];
        layoutManager = [[NSLayoutManager alloc] init];
        
        [layoutManager addTextContainer:textContainer];
        [textStorage addLayoutManager:layoutManager];
        
        [textContainer setLineFragmentPadding:0];
        pointCapacity = 128;
        points = malloc(pointCapacity * sizeof(NSPoint));
    }
    
    return self;
}

- (GLString *)glStringFor:(NSString *)theString font:(NSFont *)theFont {
    NSAssert(theString != nil, @"string must not be nil");
    
    NSMutableDictionary* stringsForFont = [glStrings objectForKey:theFont];
    if (stringsForFont == nil) {
        stringsForFont = [[NSMutableDictionary alloc] init];
        [glStrings setObject:stringsForFont forKey:theFont];
        [stringsForFont release];
    }
    
    GLString* glString = [stringsForFont objectForKey:theString];
    if (glString == nil) {
        NSAttributedString* attrString = [[NSAttributedString alloc] initWithString:theString];
        [textStorage setAttributedString:attrString];
        [attrString release];
        
        [layoutManager ensureLayoutForTextContainer:textContainer];
        
        NSRange glyphRange = [layoutManager glyphRangeForCharacterRange:NSMakeRange(0, [theString length]) actualCharacterRange:NULL];
        NSRect bounds = [layoutManager boundingRectForGlyphRange:glyphRange inTextContainer:textContainer];
        NSGlyph* glyphs = malloc((glyphRange.length + 1) * sizeof(NSGlyph));
        NSUInteger count = [layoutManager getGlyphs:glyphs range:glyphRange];
        
        NSBezierPath* path = [NSBezierPath bezierPath];
        [path setFlatness:1];
        [path moveToPoint:NSMakePoint(0, 0)];
        [path appendBezierPathWithGlyphs:glyphs count:count inFont:theFont];
        free(glyphs);
        
        path = [path bezierPathByFlatteningPath];
        
        if ([path windingRule] == NSNonZeroWindingRule)
            gluTessProperty(gluTess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);
        else
            gluTessProperty(gluTess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
        
        GLStringData* glStringData = [[GLStringData alloc] init];
        gluTessBeginPolygon(gluTess, glStringData);
        
        GLdouble coords[3];
        coords[2] = 0;
        
        while ([path elementCount] > pointCapacity)
            [self resizePointArrayTo:2 * pointCapacity];
        
        for (int i = 0; i < [path elementCount]; i++) {
            NSBezierPathElement element = [path elementAtIndex:i associatedPoints:&points[i]];
            // points[i].y = bounds.size.height - points[i].y;
            switch (element) {
                case NSMoveToBezierPathElement:
                    gluTessBeginContour(gluTess);
                    coords[0] = points[i].x;
                    coords[1] = points[i].y;
                    gluTessVertex(gluTess, coords, &points[i]);
                    break;
                case NSClosePathBezierPathElement:
                    gluTessEndContour(gluTess);
                    break;
                case NSLineToBezierPathElement:
                    coords[0] = points[i].x;
                    coords[1] = points[i].y;
                    gluTessVertex(gluTess, coords, &points[i]);
                    break;
                default:
                    break;
            }
        }
        gluTessEndPolygon(gluTess);

        glString = [[GLString alloc] initWithVbo:vbo data:glStringData size:bounds.size];
        [glStringData release];
        
        [stringsForFont setObject:glString forKey:theString];
        [glString release];
    }

    
    return glString;
}

- (void)activate {
    [vbo activate];
}

- (void)deactivate {
    [vbo deactivate];
}

- (void)dealloc {
    [textStorage release];
    [textContainer release];
    [layoutManager release];
    gluDeleteTess(gluTess);
    [glStrings release];
    [vbo release];
    free(points);
    [super dealloc];
}
@end
