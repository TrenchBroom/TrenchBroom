//
//  BrushFactory.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Entity;
@class Brush;
@class Vector3i;

@interface BrushFactory : NSObject {

}
+ (BrushFactory *)sharedFactory;

- (Brush *)createCuboidFor:(Entity *)entity atCenter:(Vector3i *)center dimensions:(Vector3i *)dimensions texture:(NSString *)texture;
@end
