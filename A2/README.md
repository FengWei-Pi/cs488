# Compilation
Standard compilation instructions

```bash
> premake4 gmake
> make
> ./A2
```

I tested this assignment on computer CS009792.

# Manual

For transformations, the y displacement of the mouse is usually ignored and only the x displacement is used.

## Assumptions

1. Initially, the world and the model gnomons are overlapped because the cube renders at the center of the world.
2. The model gnomon is subject to all transformations but model scaling. The world gnomon is subject to view and perspective transformations. It is ambiguous whether the gnomons undergo clipping since clipping is a part of the perspective transformation. I assumed that the gnomons are subject to near and far plane clipping.
3. The viewport is initially set to be 90% window width and height. If, during the lifetime of the applcation, the window is resized, then the reset button is pressed, the viewport will reset to it's original dimensions, not 90% of the current window width and 90% of the current height.
4. The near plane cannot be moved past the far plane. The far plane cannot be moved to lie before the near plane.
5. Initially, the z-axis points into the screen, the x-axis points to the right, and the y-axis points up.
