#      Designer Manual
###### Manual Version 1.1  |  Designer Plugin Version 0.1.1  |  Unreal Engine Version 4.18



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
If the plugin is not showing up, close the editor and check if the plugin is installed in the correct directory. If the issues persists check if your unreal engine version matches the plugin engines version mentioned at the top of this document.

## Getting Started
The editor mode is activated by clicking the Designer icon ![alt text][icon] in the Modes window. If the Modes window is not open, open it by clicking "Window -> Modes".

[icon]: https://raw.githubusercontent.com/RoelBartstra/Designer/master/Plugins/Designer/Resources/Icon40.png "Icon"



### Spawning Objects
While in the designer editor mode:
1. Click on a placable asset in the content browser.
2. Hold down the ctrl key.
3. Click and hold the left mouse button.
4. Drag the mouse into a direction to rotate and scale the object.
5. Release the left mouse button.



## FAQ
### It asks to recompile when I launch the project, what should I do?
The plugin might not be compiled for your engine version or for your platform. If you have a C++ project and your plugin is in your project folder just hit recompile.
If this does not work check to see if the engine version matches the plugin engine version mentioned at the top of this document. If your engine version is older it will most likely not work. Download a matching engine version and try again.