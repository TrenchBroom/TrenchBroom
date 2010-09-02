//
//  Polygon2D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 28.08.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Edge2D.h"

@interface Polygon2D : NSObject {
    NSMutableArray* vertices;
    NSMutableArray* firstUpper;
    NSMutableArray* firstLower;
}

- (id)initWithVertices:(NSArray *)newVertices;
- (Edge2D *)firstUpper;
- (Edge2D *)firstLower;

- (NSArray *)vertices;
- (Polygon2D *)intersectWith:(Polygon2D *)polygon;

@end
