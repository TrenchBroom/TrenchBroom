//
//  GLFontManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 03.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <OpenGL/glu.h>

@class VBOBuffer;
@class GLString;

@interface GLFontManager : NSObject {
    @private
    VBOBuffer* vbo;
    GLUtesselator* gluTess;
    NSLayoutManager* layoutManager;
    NSTextStorage* textStorage;
    NSTextContainer* textContainer;
    NSMutableDictionary* glStrings;
    NSPoint* points;
    int pointCapacity;
}

- (GLString *)glStringFor:(NSString *)theString font:(NSFont *)theFont;

- (void)activate;
- (void)deactivate;
@end
