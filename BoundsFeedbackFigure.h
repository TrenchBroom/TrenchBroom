//
//  BoundsFeedbackFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 05.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Figure.h"
#import "Math.h"

@interface BoundsFeedbackFigure : NSObject <Figure> {
    TBoundingBox bounds;
}

- (void)setBounds:(TBoundingBox *)theBounds;

@end
