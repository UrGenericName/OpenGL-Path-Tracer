<h1>OpenGL PBR Path-Tracing Engine</h1>

<img width="1200" height="800" alt="output_2026-06-30_19-22-03 5584778" src="https://github.com/user-attachments/assets/42e6005e-45df-40a3-893b-235a7429dd2b" />

Over the past month, I've been hard at work building a PBR Path-Tracing Engine developed in OpenGL. This project served as an opportunity to finally delve headfirst into the intimidating prospect of learning a graphics API for the first time. It has been quite the challenge, yet deeply rewarding, teaching me many of the technical fundamentals of 3D graphics.

The primary objective for this project was to build a reasonably flexible engine which utilizes standardized pipelines and conventions such as PBR materials. At the end of the project I wanted a user to be able to build and render scenes from scratch, without needing to interface with any of the underlying C++ code in the process. This involved implementing quality-of-life features such as a scene importer/exporter, dedicated editor window, translation gizmo, and a model importer. 

While there are still issues that should ideally be ironed out at some-point, such as the OBJ importer lacking a native triangulation solution, a handful of unresolved crashes, improving overall performance—in general this engine successfully fulfills its core objective.

<img width="1201" height="825" alt="1782879261456" src="https://github.com/user-attachments/assets/30c269e7-c5c1-43c3-bb90-c53c1dc5ae4a" />

Utilizing all these systems allowed me to build, and render, entire scenes with reasonable flexibility. While it's nowhere near perfect—and will by no means be should be used as a replacement for industry-standard tools like Blender—I think it was highly successful as a technical study and proof of concept.

In addition to my primary objective, some of the technical goals I had in regard to rendering included an implementation of a PBR pipeline (Albedo, Normal, Roughness, and Metallic maps), Cook-Torrance specular BRDF microfacet reflectance model, and Monte Carlo sampling. For performance, I aimed to implement a Z-prepass to cut down on overdraw, alongside model-level bounding boxes for collision detection.

<img width="800" height="800" alt="1782879732312" src="https://github.com/user-attachments/assets/29ff3d48-9fa9-4f4d-b14f-eba6f0817a6e" />

Adopting a PBR workflow unlocked a vast array of possibilities in regard to object variety. Achieving this was a major milestone in the project, as it dramatically elevated the potential for realism in renders. I was now able to properly integrate my own assets or any widely-available, third party PBR assets.

Below is a render of a high-fidelity [soccer-ball model](https://www.fab.com/listings/ffba6d80-3166-4648-a7e3-3f90e6a60131), which illustrates the physical accuracy of the PBR pipeline in my path-tracer.

https://www.youtube.com/watch?v=AvZem1g2c6Y

To accommodate video renders, I implemented a flexible animation system, which allows the user to create and hook animation functions to meshes or cameras. These functions execute every frame and allow for various changes to an object's position, rotation, scale, emission, texture-set, tint etc.

Some of the basic functions I implemented for this showcase include hue spin, linear mesh translation, sine-wave move and camera orbit functions. This render below utilizes the hue spin and mesh spin animations.

https://github.com/user-attachments/assets/50a54fdc-03c8-4691-abd0-e01327e0bc39

Being and offline renderer, maximum performance wasn't the main goal. However, I still felt it important to keep performance in mind throughout the project, optimizing where reasonable to learn OpenGL's limits and bottlenecks. Ensuring buffers are only updated when necessary, using a single texture to avoid redundant duplicates, limiting conditional branching in my fragment shaders, etc.

For the latter, I ended up with some clever bitwise operations as a replacement to conditional branches. For example, to make highlighted objects stand out from similar colors, I needed a dotted pattern. I avoided a shader branch by using a bitwise OR to generate a zero on even X/Y coordinates, which ended up creating the pattern I was looking for. 

```
inputColor = mix(inputColor, highlightColor, strength);
inputColor *= ( (pixelCoords.x % 2) | (pixelCoords.y % 2) );
```
Most of my performance considerations were small choices like this, more focused on avoidance rather than implementing large acceleration structures. One of the big optimizations that felt a bit out of scope for this project was a bounding-volume hierarchy. I still would like to use this at some point in one of my future projects, but for this one I opted to a simpler, less efficient, model & bounding-box pair.


Overall, I am incredibly pleased with how this project turned out and am excited to have cleared the beginner’s hurdle of graphics programming. I look forward to utilizing everything I've learned in future graphics projects, and I plan to explore more APIs down the road.
