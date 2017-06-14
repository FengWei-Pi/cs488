# Compilation
Standard compilation instructions

```BASH
> premake4 gmake
> make
> ./A3
```

I tested this program on CS007681 and verified that it worked.

# Manual
1. In the assignment, it says that the head cannot be an undecorated sphere as it would be difficult to observe head rotation that way. In my assignment, I make the head an undecorated cube. Therefore, even though the head is undecorated, it is possible to observe head rotation.
2. The virtual trackball is literally just a virtual input device. If the mouse is on the device, and the right button is pressed and the mouse is moved, then the puppet will roate about the y axis. If while dragging, the mouse moves off the input device, the puppet will stop rotating. If the device is invisible, then dragging with the right button clicked does nothing.
3. Selecting an item should change its visual appearance. Instead of modifying the material, I made its alpha value vary sinusoidally vary.

## Additional features
None.
