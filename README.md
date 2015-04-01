# OptiX

Creating a raytracer using NVIDIA OptiX.  End goal is to implement global illumination, hopefully with decent fps.

Sphere scene: http://youtu.be/TFm6v1NxiR4  

Rendering 1024x768 with no geometry is about 800fps on my computer  
i5 4670k @ 4.2GHz  
16gb 1866MHz ram  
GTX 770 2GB

Here is a sample of naive path tracing
![pathtrace](https://raw.github.com/jkevin1/OptiX/master/PathTracing/tests/8192s4d.png)

Here is a sample of a partial port of Kevin Beason's smallpt (http://www.kevinbeason.com/smallpt/)
![smallpt](https://raw.github.com/jkevin1/OptiX/master/PathTracing/SmallPT Port/512s4d.png)

Here is a high res rendering of the sphere scene
![spheres](https://raw.github.com/jkevin1/OptiX/master/spheres.png)

Example teapot scene without reflections
![teapot](https://raw.github.com/jkevin1/OptiX/master/flat.png)

---------------------------------------NOTES----------------------------------------  
FPS is a major issue with naive path tracing, with really noisy images it was still barely 1 fps  
Major improvements must be made to reduce the necessary samples for convergence:  
  -Direct lighting  
  -Smarter sampling distributions/weights  
  -Convert algorithm to iterative/accumulative instead of recursive as much as possible  
  -Although not an actual software improvement, I could overclock my GPU  
Uses 100% of gpu, which can lead to driver timeouts since I am using the same GPU as my OS  
  -Possibly consider tile-based rendering so that OS can render between tiles  
  -Will have small performance hit, but will make testing more reliable  
  -Once speed is improved, number of tiles can be reduced or maybe even removed entirely  
