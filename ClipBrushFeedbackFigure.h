//
//  ClipBrushFeedbackFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Figure.h"

@protocol Brush;
@class ClipPlane;

@interface ClipBrushFeedbackFigure : NSObject <Figure> {
    id <Brush> brush1;
    id <Brush> brush2;
}

- (id)initWithBrush:(id <Brush>)theBrush clipPlane:(ClipPlane *)theClipPlane;

@end
