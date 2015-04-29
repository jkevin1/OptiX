# OptiX

Creating a raytracer using NVIDIA OptiX.  End goal is to implement global illumination, hopefully with decent fps.

Sphere scene: http://youtu.be/TFm6v1NxiR4  

Rendering 1024x768 with no geometry is about 800fps on my computer  
i5 4670k @ 4.2GHz  
16gb 1866MHz ram  
GTX 770 2GB

Recording of real-time path tracer: https://youtu.be/0p9p8_US_xI

Comparison of the blur effect to reduce noise:  
![blur](https://raw.github.com/jkevin1/OptiX/master/Path Tracing/comparison.png)  

Here is a high res rendering of the sphere scene  
![spheres](https://raw.github.com/jkevin1/OptiX/master/spheres.png)

Example teapot scene without reflections  
![teapot](https://raw.github.com/jkevin1/OptiX/master/flat.png)

There are A LOT of rays:  
--For the sample 512x512 image above (frame1.png) with 6 samples per pixel  
----512x512 = 262,144 pixels  
----1,572,864 samples, each with 1-5 calls to rtTrace()  
----Best case: 1,572,864 calls to rtTrace() if every ray hit a light or missed (no diffuse reflections)  
----Worst case: 7,864,320 calls to rtTrace() if every ray hit a diffuse surface (1st call + 4 maximum ray depth)  
----Each ray may be tested with up to 9 ray-sphere intersections, not counting the lighting equations  
--Previous tests had the advantage of only spawning new radiance rays if it was a reflective/refractive surface, and only spawning 1 shadow ray.
