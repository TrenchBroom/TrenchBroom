//
//  GridFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "LineFigure.h"

@class Vector3f;
@class VBOMemBlock;

@interface GridFigure : NSObject <LineFigure> {
    @private
    Vector3f* point1;
    Vector3f* point2;
    VBOMemBlock* block;
}

- (id)initWithPoint1:(Vector3f *)thePoint1 point2:(Vector3f *)thePoint2;

@end
