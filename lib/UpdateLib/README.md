# Update-Library

A library to update github-hosted Qt6 applications on multiple platforms.

## Introduction

The library is flexible and can be adapted to different requirements, update processes
and platforms. It implements an update process that contains the following steps:

- Checking for updates
- Downloading an update
- Preparing the update for installation
- Scheduling the update for installation when the application terminates
- Triggering the update when the application terminates

### Checking for Updates

To check for an update, the library queries the github API to find all releases. The
releases are filtered depending on whether pre-releases should be included, then the
appropriate asset for the platform is selected.

The selection criteria for an asset are configurable by passing a function.

To parse and compare versions, a user defined version type must be provided, along with
a parsing function and a function to describe a version as a string.

### Downloading an Update

If an update has been found and an appropriate asset could be selected, the update can be
downloaded to a temporary location. The library performs the download, providing feedback
about the progress via a dialog.

### Preparing an Update

Once the update is downloaded to a temporary location, it can be prepared. This step is
also configurable - for example, a downloaded .zip file could be extraced in this step.

### Scheduling an Update

Updates are installed when the application quits by launching a script. The library
contains example scripts for Windows, Linux and macOS. These scripts simply install an
update by replacing the application itself with a file or folder.

## How To Use It

To use the update library, a client must create an instance of `Updater` and tie its
lifecycle to the application's lifecycle. It is important that `Updater` gets destroyed
only when the application shuts down so that it can trigger a pending update in its
destructor.

`Updater` takes an instance of `UpdateConfig`, and its properties determine the
configuration of the update process. `UpdateConfig` has the following properties:

- `checkForUpdates` is a function that performs an update check. It gets passed an instance of `UpdateController` and must call `UpdateController::checkForUpdates` with the appropriate parameters.
- `prepareUpdate` is a function that prepares a downloaded update file. It receives the path to the update file and the update config, and it returns the path to the prepared update.
- `installUpdate` is a function that performs the installation. It is called when the application shuts down, and since some platforms prevent the running application to be replaced, this function must start a detached process to actually replace the application binary. This process runs a bash or cmd script to perform the update. The library comes with example scripts for Windows, Linux and macOS.
- `updateScriptPath` the path to an update script that performs the installation. This is passed to `installUpdate`.
- `appFolderPath` is the path to the folder or bundle or file that contains the application. On Windows, this is usually a folder, on Linux this is an AppImage and on macOS, this is an application bundle.
- `relativeAppPath` is the path to the application binary within the app folder path. This path is used to restart the application after the update was installed.
- `workDirPath` is the path to a working directory where the library can place files temporarily.
- `logFilePath` is the path where the library should store its log file.