Over the past month I've been hard at work building a PBR Path-Tracing Engine developed in OpenGL.  This project served as an opportunity to finally delve head first into the intimidating prospect of learning an graphics API for the first time, which has been quite the challenge yet deeply rewarding learning many of the technical fundamentals of 3D graphics.

The primary objective for this project was to build a reasonably flexible engine that enables the creation, manipulation, and rendering (image or video) of scenes with relative ease, utilizing standardized pipelines and conventions such as PBR materials.  At the end of the project I wanted a user to be able to build and render scenes, from scratch, without needing to interface with any of the underlying C++ code in the process.  This involved implementing quality of life features, such as a scene importer/exporter, dedicated editor window, translation gizmo, and model importer.  While there are still issues that should ideally be ironed out at some-point, such as the OBJ importer not having a native triangulation solution, a handful of unresolved crashes, improving overall performance—in general this engine successfully fulfills this core objective.

<img width="1201" height="825" alt="1782879261456" src="https://github.com/user-attachments/assets/30c269e7-c5c1-43c3-bb90-c53c1dc5ae4a" />

Utilizing all these systems allowed me to build, and render, entire scenes with reasonable flexibility.  While it's nowhere near perfect, and will by no means be a personal replacement for other industry-standard tools I tend to use, such as Blender, I think it was highly successful as technical study and proof of concept.

In addition to my primary objective, some of the technical goals I had in regards to rendering included an implementation of a PBR pipeline (Albedo, Normal, Roughness and Metallic maps), Cook-Torrance specular BRDF microfacet reflectance model, and Monte Carlo sampling.  For performance, I aimed to implement a Z-prepass to cut down overdraw, alongside model-level bounding boxes for collision detection.

<img width="800" height="800" alt="1782879732312" src="https://github.com/user-attachments/assets/29ff3d48-9fa9-4f4d-b14f-eba6f0817a6e" />

Adopting a PBR workflow unlocked a vast array of possibilities in regards to object variety.  Achieving this was a major milestone in this project, as it dramatically elevated the potential for realism in renders.  I was now able to properly integrate my own, or any widely-available third party, PBR assets.

Below is a render of a high-fidelity [soccer-ball model](https://www.fab.com/listings/ffba6d80-3166-4648-a7e3-3f90e6a60131), which illustrates the physical accuracy of the PBR pipeline, utilizing Albedo, Roughness, Normal, and Metallic maps to interact naturally with the scene lighting.

https://www.youtube.com/watch?v=AvZem1g2c6Y

To accommodate video renders, I implemented a flexible animation system, which allows the user to create and hook animation functions to meshes or cameras.  These functions execute every frame, and allow various changes to an object's position, rotation, scale, emission, texture-set, tint etc.

Some of the basic functions I implemented for this showcase includes hue spin, linear mesh translation, sin-wave move and camera orbit functions.  This render below utilizes the hue spin and mesh spin animations.

https://github.com/user-attachments/assets/50a54fdc-03c8-4691-abd0-e01327e0bc39

Overall, I am incredibly pleased with how this project turned out and am excited to have cleared the beginner’s hurdle of graphics programming. I look forward to utilizing everything I've learned in future graphics projects, and I plan to explore more APIs down the road.
