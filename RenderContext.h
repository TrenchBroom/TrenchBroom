//
//  RenderContext.h
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class Options;

@interface RenderContext : NSObject {
    @private
    Options* options;
}

- (id)initWithOptions:(Options *)theOptions;

- (Options *)options;

@end
