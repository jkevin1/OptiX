# OptiX

Creating a raytracer using NVIDIA OptiX.  End goal is to implement global illumination, hopefully with decent fps.

Sphere scene: http://youtu.be/TFm6v1NxiR4  

Rendering 1024x768 with no geometry is about 800fps on my computer  
i5 4670k @ 4.2GHz  
16gb 1866MHz ram  
GTX 770 2GB

Here is a sample of naive path tracing with 8192 samples per pixel  
![pathtrace](https://raw.github.com/jkevin1/OptiX/master/PathTracing/tests/8192s4d.png)  
Look at https://github.com/jkevin1/OptiX/blob/master/PathTracing/tests/timing.txt

Here is a sample of a partial port of Kevin Beason's smallpt with 4x128 samples per pixel (http://www.kevinbeason.com/smallpt/)  
![smallpt](https://raw.github.com/jkevin1/OptiX/master/PathTracing/SmallPT Port/512s4d.png)

Here is a high res rendering of the sphere scene  
![spheres](https://raw.github.com/jkevin1/OptiX/master/spheres.png)

Example teapot scene without reflections  
![teapot](https://raw.github.com/jkevin1/OptiX/master/flat.png)

---------------------------------------NOTES----------------------------------------  
FPS is a major issue with naive path tracing, with really noisy images it was still barely 1 fps  
Major improvements must be made to reduce the necessary samples for convergence:  
--Direct lighting  
--Smarter sampling distributions/weights  
--Convert algorithm to iterative/accumulative instead of recursive as much as possible  
--Although not an actual software improvement, I could overclock my GPU  
Uses 100% of gpu, which can lead to driver timeouts since I am using the same GPU as my OS  
--Possibly consider tile-based rendering so that OS can render between tiles  
--Will have small performance hit, but will make testing more reliable  
--Once speed is improved, number of tiles can be reduced or maybe even removed entirely  
There are A LOT of rays:  
--For the sample 512x512 image above (8192s4d.png)  
----512x512 = 262144 pixels  
----262,144 * 8192 = 2,147,483,648 samples  
----Best case: 2,147,483,648 calls to rtTrace() if every hit is a light/miss (no diffuse reflections)  
----Worst case: 10,737,418,240 calls to rtTrace() if every hit is a diffuse surface (1st call + 4 maximum ray depth)  
--Previous tests had the advantage of only spawning new rays if it was a reflective/refractive surface, direct lighting should add a huge speedup  
The SDK has some path tracing examples, they seemed to be running about ~20-30fps for the cornell box scene but they used accumulation buffers to reduce number of samples further
