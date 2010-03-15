//
//  MapWindowController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MapView2D.h"
#import "MapView3D.h"


@interface MapWindowController : NSWindowController {
	IBOutlet MapView2D* view2D;
	IBOutlet MapView3D* view3D;
	IBOutlet NSOpenGLView* textureBrowser;
}

@end
