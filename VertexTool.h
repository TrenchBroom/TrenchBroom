//
//  VertexTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "DefaultTool.h"

@class MapWindowController;

@interface VertexTool : DefaultTool {
    MapWindowController* windowController;
    NSMutableSet* vertexHits;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

@end
