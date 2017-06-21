A README file, as usual. Your README needs to contain a description of your extra feature and of your unique scene(s). If you implement an acceleration feature, provide a switch to turn it on and off (this can be a compile-time switch, in which case you should provide two executables) and provide comparative timings. If you use external models, please credit where you got them from.

# Compilation
Standard compilation instructions.

```bash
> premake4 gmake
> make
> ./A4 ./Assets/sample.lua
```

# Manual
1. Due to the fact that macho-cows and simple-cows are computationally expensive, they are rendered with 4 rays per pixel super-sampling, instead of the regular 9 rays per pixel.
2. The bounding volume box is simply a sphere.
3. For the non-trivial background, I'm doing a gradient from blue to black, as was done in the online examples. Since this is not a solid colour, I perceived this to be non-trivial.

# Extra feature
I decided to implement anti-aliasing using super-sampling. Within the code, the super-sampling factor is called `ssFactor`. By default, it's set to 2, which means that the ray tracer will shoot 4 rays for each pixel on the screen.

# Unique Scene
It's a cow walking down a red carpet, laid on field of grass, and surrounded by pillars, in ancient Rome.

This scene demonstrates hierarchical rendering and mesh rendering.
