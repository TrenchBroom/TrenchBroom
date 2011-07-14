//
//  TextRenderer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 13.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Math.h"

@class Camera;
@class GLFontManager;
@protocol TextAnchor;

@interface TextRenderer : NSObject {
@private
    Camera* camera;
    GLFontManager* fontManager;
    NSMutableDictionary* strings;
    NSMutableDictionary* anchors;
}

- (id)initWithFontManager:(GLFontManager *)theFontManager camera:(Camera *)theCamera;

- (void)addString:(NSString *)theString forKey:(id <NSCopying>)theKey withFont:(NSFont *)theFont withAnchor:(id <TextAnchor>)theAnchor;
- (void)removeStringForKey:(id <NSCopying>)theKey;

- (void)renderColor:(float *)theColor;

@end
