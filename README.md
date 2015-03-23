# OptiX

Creating a raytracer using NVIDIA OptiX.  End goal is to implement global illumination, hopefully with decent fps.

Updated to real time rendering to an OpenGL VBO which is drawn on the screen (see Viewer.cpp)

Sponza in real time, averages 60fps: https://www.youtube.com/watch?v=yyoih5evPgU&feature=youtu.be

Buggy torus video, actually gets closer to 400 fps https://www.youtube.com/watch?v=HRdshKPPdg4

Rendering with no geometry is about 800fps on my computer

Example teapot scene with reflections
![teapot1](https://raw.github.com/jkevin1/OptiX/master/reflect.png)

Example teapot scene without reflections
![teapot2](https://raw.github.com/jkevin1/OptiX/master/flat.png)

Example sponza scene
![sponza](https://raw.github.com/jkevin1/OptiX/master/sponza.png)

NOTES:
Torus shadow bug
Debug/Release made no difference, both cpu and gpu seem to be perfcapped?
3 spheres scene with reflections and 4xMSAA: ~150fps...lots of rays, should be at least close to decent pathtracer w/o MSAA
Refractions...but it seems to have noise, at least for spheres
Environment Map, looks a lot more interesting and helps visualize reflections, but might contribute to refraction noise
...

