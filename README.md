# OptiX

Creating a raytracer using NVIDIA OptiX.  End goal is to implement global illumination, hopefully with decent fps.

Sphere scene: http://youtu.be/TFm6v1NxiR4  
Torus scene (buggy): http://youtu.be/jZJwEqyQbmo

Rendering with no geometry is about 800fps on my computer  
i5 4670k @ 4.2GHz  
16gb 1866MHz ram  
GTX 770

Here is a high res rendering of the sphere scene
![spheres](https://raw.github.com/jkevin1/OptiX/master/spheres.png)

Example teapot scene without reflections
![teapot](https://raw.github.com/jkevin1/OptiX/master/flat.png)

NOTES:  
Release build saves ~1-2ms render time  
Torus self-intersection bug, probably just avoid using torus, it has much worse fps anyway    
Debug/Release made no difference, both cpu and gpu seem to be perfcapped?  
Rotated-Grid Supersampling used to reduce jagged edges, approx 4x more computation  
3 spheres scene with reflections and 4xMSAA: ~150fps...lots of rays, should be at least close to decent pathtracer w/o MSAA  
Refractions  
Environment Map, looks a lot more interesting and helps visualize reflections, but might contribute to refraction noise  
Added some path tracing tests, based on SmallPT (http://www.kevinbeason.com/smallpt/)
