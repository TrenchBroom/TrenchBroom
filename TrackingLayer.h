//
//  TrackingLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Layer.h"

@class BrushGuideRenderer;
@class Camera;
@class GLFontManager;

@interface TrackingLayer : NSObject <Layer> {
    BrushGuideRenderer* brushGuideRenderer;
}

- (id)initWithCamera:(Camera *)theCamera fontManager:(GLFontManager *)theFontManager font:(NSFont *)theFont;

@end
