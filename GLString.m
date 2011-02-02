//
//  GLString.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GLString.h"
#import <OpenGL/gl.h>

@implementation GLString

- (id)init {
    if (self = [super init]) {
        texId = 0;
    }
    
    return self;
}

- (id)initWithString:(NSString *)theString {
    if (theString == nil)
        [NSException raise:NSInvalidArgumentException format:@"string must not be nil"];
    
    if (self = [self init]) {
        string = [theString retain];
    }
    
    return self;
}

- (void)render {
    glPushAttrib(GL_ENABLE_BIT | GL_TEXTURE_BIT | GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    if (texId == 0) {
        NSAttributedString* attrString = [[NSAttributedString alloc] initWithString:string attributes:[NSMutableDictionary dictionaryWithObject:[NSColor whiteColor] forKey:NSForegroundColorAttributeName]];
        
        [[NSGraphicsContext currentContext] setShouldAntialias:YES];
        texSize = NSMakeSize([attrString size].width + 16, [attrString size].height + 1);
        NSImage* image = [[NSImage alloc] initWithSize:texSize];
        [image lockFocus];
        
        NSBezierPath* path = [NSBezierPath bezierPathWithRoundedRect:NSMakeRect(0, 0, texSize.width, texSize.height) xRadius:7 yRadius:7];
//        [[NSColor grayColor] set];
//        [path fill];

        [attrString drawAtPoint:NSMakePoint(8, 1)];
        
        NSBitmapImageRep* bitmap = [[NSBitmapImageRep alloc] initWithFocusedViewRect:NSMakeRect(0, 0, texSize.width, texSize.height)];
        [image unlockFocus];

        NSBitmapImageRep* grayscale= [bitmap bitmapImageRepByConvertingToColorSpace:[NSColorSpace genericGrayColorSpace] renderingIntent:NSColorRenderingIntentDefault];
        
        NSData *data = [grayscale representationUsingType: NSPNGFileType properties: nil];
        [data writeToFile: @"/test.png" atomically: NO];

        NSLog(@"bitmap: %i, grayscale: %i", [bitmap bitsPerPixel], [grayscale bitsPerPixel]);
        
        glGenTextures(1, &texId);
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, texId);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexImage2D(GL_TEXTURE_RECTANGLE_EXT, 0, GL_LUMINANCE_ALPHA, texSize.width, texSize.height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, [grayscale bitmapData]);
        
        [bitmap release];
        [image release];
        [attrString release];
        [string release];
        string = nil;
    } else {
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, texId);
    }

    glDisable (GL_DEPTH_TEST);
    // glEnable (GL_BLEND);
//    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    
    glColor4f(1, 0, 0, 1);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glBegin (GL_QUADS);
    glTexCoord2f(0, 0);
    glVertex2f(0, 0);
    
    glTexCoord2f(texSize.width, 0);
    glVertex2f(texSize.width, 0);

    glTexCoord2f(texSize.width, texSize.height);
    glVertex2f(texSize.width, texSize.height);
    
    glTexCoord2f(0, texSize.height);
    glVertex2f(0, texSize.height);
    glEnd ();    

    glPopAttrib();
}

- (void)dispose {
    if (texId != 0) {
        glDeleteTextures(1, &texId);
        texId = 0;
    }
}

- (void)dealloc {
    [string release];
    [super dealloc];
}

@end
