//
//  EdgeFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 22.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "LineFigure.h"

@class Edge;
@class VBOMemBlock;

@interface EdgeFigure : NSObject <LineFigure> {
    @private
    Edge* edge;
    VBOMemBlock* block;
}

- (id)initWithEdge:(Edge *)theEdge;

@end
