//
//  Edge.h
//  TrenchBroom
//
//  Created by Kristian Duske on 24.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Vector3f;

@interface Edge : NSObject {
    int startIndex;
    int endIndex;
}
- (id)initWithStartIndex:(int)theStartIndex endIndex:(int)theEndIndex;

- (int)startIndex;
- (int)endIndex;

- (Vector3f *)startVertex:(NSArray *)vertices;
- (Vector3f *)endVertex:(NSArray *)vertices;

- (void)setStartIndex:(int)theStartIndex;
- (void)setEndIndex:(int)theEndIndex;

@end
