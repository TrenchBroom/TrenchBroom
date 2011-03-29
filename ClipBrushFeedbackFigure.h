//
//  ClipBrushFeedbackFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 29.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "FeedbackFigure.h"

@class MutableBrush;
@protocol Brush;
@class ClipPlane;

@interface ClipBrushFeedbackFigure : NSObject <FeedbackFigure> {
    MutableBrush* brush1;
    MutableBrush* brush2;
}

- (id)initWithBrush:(id <Brush>)theBrush clipPlane:(ClipPlane *)theClipPlane;

@end
