/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "AppDelegate.h"
#import "Model/Preferences.h"
#import "Model/Map/EntityDefinition.h"
#import "IO/Pak.h"
#import "Model/Assets/Alias.h"
#import "Model/Assets/Bsp.h"
#import "MacPreferences.h"

@implementation AppDelegate

- (void)applicationWillFinishLaunching:(NSNotification *)notification {
    TrenchBroom::Model::Preferences::sharedPreferences = new TrenchBroom::Model::MacPreferences();
    TrenchBroom::Model::Preferences::sharedPreferences->init();
    TrenchBroom::Model::EntityDefinitionManager::sharedManagers = new TrenchBroom::Model::EntityDefinitionManagerMap();
    TrenchBroom::IO::PakManager::sharedManager = new TrenchBroom::IO::PakManager();
    TrenchBroom::Model::Assets::AliasManager::sharedManager = new TrenchBroom::Model::Assets::AliasManager();
    TrenchBroom::Model::Assets::BspManager::sharedManager = new TrenchBroom::Model::Assets::BspManager();
}

- (void)applicationWillTerminate:(NSNotification *)notification {
    delete TrenchBroom::Model::Assets::BspManager::sharedManager;
    TrenchBroom::Model::Assets::BspManager::sharedManager = NULL;
    delete TrenchBroom::Model::Assets::AliasManager::sharedManager;
    TrenchBroom::Model::Assets::AliasManager::sharedManager = NULL;
    delete TrenchBroom::IO::PakManager::sharedManager;
    TrenchBroom::IO::PakManager::sharedManager = NULL;
    delete TrenchBroom::Model::EntityDefinitionManager::sharedManagers;
    TrenchBroom::Model::EntityDefinitionManager::sharedManagers = NULL;
    delete TrenchBroom::Model::Preferences::sharedPreferences;
    TrenchBroom::Model::Preferences::sharedPreferences = NULL;
}

- (BOOL)applicationShouldOpenUntitledFile:(NSApplication *)sender {
    return NO;
}

@end
