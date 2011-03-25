//
//  Math3D.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.10.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Math3D.h"
#import "Vector3f.h"

int smallestVertex(NSArray *vertices) {
    if ([vertices count] == 1)
        return 0;
    
    int s = 0;
    Vector3f* sv = [vertices objectAtIndex:0];
    
    int i;
    for (i = 1; i < [vertices count]; i++) {
        Vector3f* v = [vertices objectAtIndex:i];
        if ([v compareToVector:sv] == NSOrderedAscending) {
            s = i;
            sv = v;
        }
    }
    
    return s;
}