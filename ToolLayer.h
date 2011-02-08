//
//  ToolLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Layer.h"

@interface ToolLayer : NSObject <Layer> {
    @private
    NSMutableSet* toolFigures;
}

@end
