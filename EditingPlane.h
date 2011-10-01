//
//  EditingPlane.h
//  TrenchBroom
//
//  Created by Kristian Duske on 01.10.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Math.h"

@class Camera;

@interface EditingPlane : NSObject {
    const TVector3f* xAxis;
    const TVector3f* yAxis;
    const TVector3f* frontAxis;
    const TVector3f* backAxis;
    const TVector3f* upAxis;
    const TVector3f* rightAxis;
    const TVector3f* downAxis;
    const TVector3f* leftAxis;
}

- (id)initWithCamera:(Camera *)theCamera;

- (const TVector3f *)frontAxis;
- (const TVector3f *)backAxis;
- (const TVector3f *)upAxis;
- (const TVector3f *)rightAxis;
- (const TVector3f *)downAxis;
- (const TVector3f *)leftAxis;

- (float)intersectWithRay:(const TRay *)theRay planePosition:(const TVector3f *)thePlanePos;

@end
