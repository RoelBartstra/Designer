#      Designer Manual
###### Manual Version 1.0
###### Designer Plugin Version 0.1.0
###### Unreal Engine Version 4.18

## Table of Contents
1. [Introduction](#introduction)
2. [Download](#download)
3. [Install](#install)

## Introduction
Designer is an Editor Mode created as a plugin for Unreal Engine 4. For more in depth information about plugins inside Unreal Engine 4 read their [documentation](https://docs.unrealengine.com/latest/INT/Programming/Plugins/).

The Designer plugin is licensed under the MIT license. The source code is available on [GitHub](https://github.com/RoelBartstra/Designer). Feel free to make improvements, and when possible, share the changes with the community.


## Download
Download the [latest version](https://www.unrealengineer.com/designer) and unzip the archive.


## Install
Plugins are always located in your plugin's directory. In order for plugins to be found, they must be located in one of the valid search paths for plugins in Unreal Engine.

Pasting the Designer folder inside the Engine plugins directory will expose this plugin to all your projects. Any time a new engine version is installed the plugin has to be moved to the new engine versions' folder.

```
Engine Plugin
/UE4 Root/Engine/Plugins/Designer/
```

​Pasting the Designer folder inside the Project plugins directory will expose this plugin only to the specified project.

```
Project Plugin
/My Project/Plugins/Designer/
```

Copy the Designer folder extracted from the zip file to the plugin folder fitting your needs.

Open you project. The extra editor mode should now available. If the mode is not visible, open the Plugins window and check if the plugin is enabled under Editor Modes / Designer. If the plugin is disabled, enable it and restart the editor. 
If the plugin is not showing up, close the editor and check if the plugin is installed in the correct directory. I the issues persists check if the plugin 




* Unordered list can use asterisks
- Or minuses
+ Or pluses