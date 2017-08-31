# Compilation
For this assignment some library code was modified. Therefore, to run this project, you need to remake the library code:

```Bash
> cd Assignments
> premake4 gmake
> make
```

Then, you can make the actual assignment:

```Bash
> cd A5
> premake4 gmake
> make
```

This project has the additional dependency of OpenAL, which has only been configured to compile with MacOS. To have this run on the linux machines in the lab, some headers may need to be changed inside A5.hpp. Of course, other things may also unfortunately break. I'll bring my laptop to the demo, so this shouldn't be a problem.

# Manual
All my objectives have been implemented. I did half an additional feature, which is the skybox. I'll elaborate more in the documentation I'll actually submit.

# Objectives

Modeling the Scene - World objects are rendered on to the screen correctly.
UI - The mouse and keyboard can be used by the player to control the game objects and camera. A GUI also exists for controlling game settings.
Texture mapping - It is evident that texture mapping has been employed at least once.
Keyframe Animation - When the player walks, the puppet plays a walking animation. The current frame is interpolated from its surrounding frames using linear interpolation.
Static collisions - On each stationary platform, the puppet does not fall through the platform and die.
Dynamic collisions - On each moving platform, the puppet does not fall through the platform and die.
Synchronized sound - When the player interacts with the game by moving the puppet, there is audible feedback.
Physics Engine - The puppet is subject to gravitational forces (parameter: g) and wind force (parameter: F_w). The puppet is also subject to static friction when it's standing on a platform. The static friction is a function of the puppet's mass (parameter: M) and the platform's normal.
Transparency - Transparency will be implemented using the alpha channel. At least one game object is transparent.
Shadows - Shadows are implemented using shadow maps.
