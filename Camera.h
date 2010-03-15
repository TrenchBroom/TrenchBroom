//
//  Camera.h
//  TrenchBroom
//
//  Created by Kristian Duske on 15.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Vector3f.h"

@interface Camera : NSObject {
	Vector3f* position;
	Vector3f* viewDir;
	Vector3f* up;
	Vector3f* right;
}

@end
