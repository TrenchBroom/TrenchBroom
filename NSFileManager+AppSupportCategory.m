//
//  NSFileManagerPRAdditions.m
//  TrenchBroom
//
//  Created by Kristian Duske on 06.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "NSFileManager+AppSupportCategory.h"


@implementation NSFileManager (AppSupportCategory)

- (NSString *)findSystemFolderType:(int)folderType forDomain:(int)domain
{
    FSRef folder;
    OSErr err = noErr;
    CFURLRef url;
    NSString *result = nil;
    
    err = FSFindFolder(domain, folderType, false, &folder);
    if (err == noErr) {
        url = CFURLCreateFromFSRef(kCFAllocatorDefault, &folder);
        result = [(NSURL *)url path];
        CFRelease(url);
    }
    
    return result;
}

- (NSString *)findApplicationSupportFolder {
    NSString* applicationSupportFolder = [self findSystemFolderType:kApplicationSupportFolderType forDomain:kUserDomain];
    NSString* bundlePath = [[NSBundle mainBundle] bundlePath];
    NSString* appName = [[self displayNameAtPath:bundlePath] stringByDeletingPathExtension];
    
    return [NSString pathWithComponents:[NSArray arrayWithObjects:applicationSupportFolder, appName, nil]];
}

@end