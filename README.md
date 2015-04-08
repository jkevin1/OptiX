# OptiX

Creating a raytracer using NVIDIA OptiX.  End goal is to implement global illumination, hopefully with decent fps.

Sphere scene: http://youtu.be/TFm6v1NxiR4  

Rendering 1024x768 with no geometry is about 800fps on my computer  
i5 4670k @ 4.2GHz  
16gb 1866MHz ram  
GTX 770 2GB

Here is a sample of path tracing with 32 samples per pixel and 64 iterations  
![pathtrace](https://raw.github.com/jkevin1/OptiX/master/PathTracing/tests/32x64.png)  
Look at https://github.com/jkevin1/OptiX/blob/master/PathTracing/tests/times.txt

Here is a sample of a partial port of Kevin Beason's smallpt with 4x128 samples per pixel (http://www.kevinbeason.com/smallpt/)  
![smallpt](https://raw.github.com/jkevin1/OptiX/master/PathTracing/SmallPT Port/512s4d.png)

Here is a high res rendering of the sphere scene  
![spheres](https://raw.github.com/jkevin1/OptiX/master/spheres.png)

Example teapot scene without reflections  
![teapot](https://raw.github.com/jkevin1/OptiX/master/flat.png)

There are A LOT of rays:  
--For the sample 512x512 image above (8192s4d.png) which took 40.977344 seconds:  
----512x512 = 262,144 pixels  
----262,144x8,192 = 2,147,483,648 samples, each with 1-5 calls to rtTrace()  
----Best case: 2,147,483,648 calls to rtTrace() if every ray hit a light or missed (no diffuse reflections)  
----Worst case: 10,737,418,240 calls to rtTrace() if every ray hit a diffuse surface (1st call + 4 maximum ray depth)  
--Previous tests had the advantage of only spawning new radiance rays if it was a reflective/refractive surface, and only spawning 1 shadow ray.

---------------------------------------NOTES----------------------------------------  
32 samples per pixel can get just over 6 fps  
30 fps is around 7-8 samples per pixel currently  
Improvements:  
--Direct lighting, although its different, more flexible but probably slower  
--Cosine weighted sampling  
--Accumulation buffer, allows for multiple passes to improve effective fps  
--Minor optimizations and cleaner code  
TODO:  
--Improve prng to avoid extreme noise  
--Render to a window instead of image with interactivity  
--Noise reduction, probably in shader, not sure how effective it can be  
--Optimizations  
--Write document about process  
--Crunch time with 2 other projects  
Uses 100% of gpu, which can lead to driver timeouts since I am using the same GPU as my OS  
--Tiles didn't seem to help much without there being some break between them  
--Using less samples per call and accumulation seems like a better option  
The cornell box scene in the SDK that uses mis gets around 6 fps as well, but has less noise
