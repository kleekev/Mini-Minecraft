# The Creeper Company Mini Minecraft
## Features Implemented
Jeffrey Yang: Game Engine Tick Function and Player Physics, Cave Systems, Distance Fog, Post-Processing Camera Overlay, Procedural Sky, Creeper NPC

Justin Duhamel: Procedural Terrain, Multithreaded Terrain Generation, Additional Biomes, Procedurally Placed Assets (terrain population queue system), Efficient VBO creation and deletion

Kevin Lee: Efficient Terrain Rendering and Chunking, Texturing and Texture Animation, Water Waves, Procedural Grass Color

## Milestone 1 Features
### Procedural Terrain
Assignee: Justin Duhamel

Implementation Details:
- I used (x,z) Perlin noise with very low grid size to create the biome mapping with a biome division at a value of .5.
- I then use smoothstep to interpolate between heights of the different biomes to have a smooth transition at the biome border
- I used fractal Brownian motion and Perlin noise to generate the plains and mountain biomes. 
- Both biomes use multiple samples of Perlin noise at different grid sizes and amplitudes to get different effects. e.g., rolling hills effects for the grasslands and very localized mountain features for the mountains biome.
- FBM serves as some high frequency noise to add details to the landscape.
- For terrain generation around the player I call chunk generation in a 3x3 grid around the chunk the player currently resides in.

Encountered Difficulties:
- It is very difficult to get exactly the terrain generation you want and requires a lot of generation. I think the grasslands biome looks pretty good, but the mountains biome could use some work with different noise types.


### Efficient Terrain Rendering and Chunking
Assignee: Kevin Lee

Implementation Details:
- In the chunk class, I added multiple member variables such as two integers to represent the corner origin of the chunk in world space and I added a boolean to indicate whether a chunk has created its VBO data or not.
- I created chunkhelper.h that contains different structs such as Vertex, VertexData, and BlockFace in order to store data in an organized manner. The header also contains unordered maps that will get color from a BlockType as the key.
- I updated the chunk class so that it inherits from Drawable. To create the interleaved VBO data, I check each position in the chunk. If the BlockType is not an Empty block, check every direction around the block to see if it contains an Empty BlockType. If it does then append the vertex data of the face into the VBO. When appending the vertex data into the vector, I created a Vertex that contains the data and append it into the data.
- I created a new draw function in shaderprogram that draws based on interleaved VBO data.
- I created a new function in terrain that takes in the players x,z position and adds the chunk to the terrain if an edge of the chunk is within 16 blocks of the player and is not connected to an existing chunk.

Encountered Difficulties:

- I had difficulty how to interleave the different vectors such as pos, col, and nor all into one VBO. To resolve the issue, I created a Vertex struct that contains the different vectors and insert them into the VBO. Then in the shader program draw function, I made sure to use the correct values and pointers to buffer the correct data.

### Game Engine Tick Function and Player Physics
Assignee: Jeffrey Yang

Implementation Details:
- I added a member variable that stores the last recorded tick to compute dT, the time since the last tick. This is then used to call the player's tick function and the camera's tick function
- I altered the Camera class to store theta(rotation about world x) and phi (rotation about world y) to implement Polar camera pivot model. The orientation of the camera changes by tracking the mouse movement in each tick.
- I updated keyboard inputs so that they update booleans inside the InputBundle sent to the player. Storing the keyboard state allows the player to move in more than one direction and continue moving while the keys are pressed down.
- Player movement is implemented by moving the player by reinterpreting the displacement in world space and calling the moveGlobal functions.
- The player's velocity is multiplied by 0.95 to decelerate the player while moving. This also simulates friction and drag.
- I implemented collision detection by grid marching along all three cardinal axes from all the player vertices and setting distance traveled along each axis before collision. I downscaled the distance traveled along a axis by 0.99 when it collides with terrain to prevent the player from going into the terrain due to floating point error.
- Gravity is applied when the player is not grounded, which is checked by grid marching in the negative y axis.
- Block placement and removal is implemented by grid marching 3 units along the camera's forward vector. For block removal, I just remove the block that is hit. For block placement, I check all 6 potential neighbors of the block hit and place a block in the location that has centerpoint closest to the ray intersection point.

Encountered Difficulties:
- I encounted a lot of floating point rounding errors when performing actions such as collision detection and checking if the player is grounded. I was able to resolve all of these issues by allowing epsilon approximations.

## Milestone 2 Features
### Cave Systems
Assignee: Jeffrey Yang

Implementation Details: 
- I used 3D perlin noise adopted from the lecture slides to implement cave systems. I sampled the noise function by dividing the x, y, z coordinates of the block by 16.f which I found to give interesting cave and lava patterns.
- The noise function is sampled using the SetRangeAndGenCaves function which samples the 3D Perlin noise for blocks in the range [1, 128]. If Perlin noise is negative, block becomes Empty and if Y < 25 block becomes lava.
- Player collision function ignores WATER and LAVA blocks. I made small changes to original grid march to treat WATER and LAVA blocks as EMPTY. However, there is an additional check to see if any of our player model's vertices are in LAVA or WATER. If it is, then we will 2/3 the overall velocity of the player.
- If a player is in WATER/LAVA, they cannot jump and when spacebar is pressed, they will ascend at a constant speed.
- I set up WATER and LAVA post processing overlays by using the provided FrameBuffer class. I stored the Rendered Texture in the frame buffer and applied a red/blue post processing tint based on whether the player's camera is in LAVA/WATER. If the player is in neither WATER/LAVA, the Rendered Texture will be sent to a No Op post processing shader.

Encountered Difficulties:
- There was some issues setting up the dimensions for the post processing shader. I tried to set up the dimensions in the constructor of MyGL which gave me incorrect values. To fix this, I set default values in the constructor and resized the dimensions once I got to the initializeGL function.
- Different tick rates on our different machines led to swim functionality working differently. To counteract this, I added additional upward velocity to counteract the effects gravity.

### Texturing and Texture Animation
Assignee: Kevin Lee

Implementation Details:
- Updated MyGL files to load in the texture map and created a new member integer variable to keep track of number of ticks. I updated the initialization function to include alpha channel.
- Updated the chunk helper class to include UVs in Vertex data and to create an unordered map that hard codes the UV data of each block. I also created unordered sets that contain BlockType that were transparent or needed to be animated. 
- Separated the VBO into two VBOs: one for opaque blocks and one for transparent blocks then combined the VBO into one such that all the opaque VBO is first.
- Updated lambert shader to take in UV and also have UV be a vec4 so that the third component can be used as a flag to see if it is animated. The fourth component can be used as a flag for future purposes.
- Added animation of water and lava blocks by adding an offset to the UV based on time in the lambert fragment shader.

Encountered Difficulties:
- I had trouble animating the water/lava blocks at first since they were initially changing at all when I added an offset to the UVs based on time. Initially, my offset were based on mod 3 which means that the offset could only take place on 3 different values. To resolve this, I instead just modded by 100 in order to have offset be in 100 different values across the row for the first two water/lava blocks.

### Multithreaded Terrain Generation
Assignee: Justin Duhamel

Implementation Details:
- I implemented a BlockTypeWorker that implements QRunnable. Each worker is assigned a chunk queue to run terrain generation on. completed chunks pointers are added to a set of completed chunks in terrain.
- After chunks finish generation they are marked for VBO data generation and put into a queue of chunks that need to have their VBO data generated. After the neighbors of the chunk that needs VBO data have finished generating the chunk pointer is given to a VBOWorker thread that computes the VBO data for the chunk and stores it in a struct in the chunk. 
- The chunks are then marked as drawable and rendered on the next pass
- Chunks are generated in a 7x7 region around the player with a 5x5 region rendered at any time

Encountered Difficulties:
- There was some complications with deciding when to dispatch VBO workers since chunks can arrive in any order. This means that if chunks are immediately sent to have their VBO data generated their VBO data could be wrong after neighboring chunks are generated. This is solved by waiting until all neighboring chunks are generated before generating VBO data


## Milestone 3 Features
### Distance Fog
Assignee: Jeffrey Yang

Implementation Details: 
- I implemented this by passing the camera position into the lambert shader. Within the vertex shader, I calculate the distance that the vertex is away from the camera and pass this information along to the fragment shader. In the fragment shader, I perform a smoothstep between the terrain color and the fog color when the vertex is at distance 150 to 175 from the camera.

### Procedural Sky
Assignee: Jeffrey Yang

Implementation Details: 
- There is a sky shader program drawn at the beginning of paintGL which draws the procedural sky and the fixed position sun. 
- I translated pixel space coordinates into world space coordinates by setting w,z, multiplying by far clip plane, and then multiplying by the inverse view projection matrix.
- To calculate uv coordinates, I find the ray from the camera to the world space pixel, and then convert the spherical ray to uv using polar coordinates.
- I also use 3D FBM + Worley noise to distort the uv coordinates.
- I chose 5 different greenish colors and interpolated between them based on uv y coordinates.

### Post-process Camera Overlay
Assignee: Jeffrey Yang

Implementation Details:
- I passed an additional time variable to the fragment shaders for lava and water post processing overlays
- For the water post processing, I used small amplitude sin and cos distortions to distort vision underwater over time. I also added a low light distortion based on Worley noise warped by FBM that changes based on time.
- For the lava post processing, I used a more noticeable distortion that changed the intensity of red seen underwater. This noise function is also Worley noise warped by FBM changing over time.

### Creeper NPC
Assignee: Jeffrey Yang

Implementation Details:
- I created a Creeper class that inherited from the Entity class, implementing the tick function and performing physics calculations. 
- Creepers can be spawned in the same way that you can place blocks. Instead of left clicking a block, you can press C on a block to spawn a creeper.
- I implemented a 3D Transformation node class and created a custom scene graph to draw a creeper.
- I used three types of cubes to draw the HEAD, BODY, and the LEGS of the creeper. Each of the different cubes sample different parts of a creeper texture file I downloaded from online.
- Creepers will evaporate as soon as it touches water.
- Creepers will follow the player when the player is within distance 15 from the creeper and stop at distance 1.5. Otherwise the creeper will walk in random directions changing every 1000 ticks. In the case that the creeper collides with a wall, it will keep trying to jump until it changes direction.
- The body of the creeper will always face the direction it is walking. The head of the creeper will track the player if the player is close enough. The creepers feet will rotate back and forth while it is moving and stop when the creeper stops.

### Water Waves
Assignee: Kevin Lee

Implementation Details:
- I updated the lambert vertex shader such that if the uv coordinates belong to water, it would change the y displacement by a certain amount.
- The height is changing by multiple sinusoidal functions added together that are dependent on the location of the block and time.

### Procedural Grass Color
Assignee: Kevin Lee

Implementation Details:
- Included a new float member variable in the chunk class called humidity to interpolate the grass color based on this value.
- When creating the VBO data of each face, I interpolated the humidity value based on distance away from the chunk's neighbors in the XPOS, XNEG, ZNEG, and ZPOS direction. After finding those values, I average them up and buffer them into the VBO data as the humidity value.
- Updated the lambert fragment shader such that if the uv coordinate is a grass face then use the humidty factor to interpolate the value of the green to change the color of the grass. This creates a smooth blend between the grass without any noticeable cut in the color if we move to a different chunk.


### Additional Biomes 
Assignee: Justin Duhamel
- I use two perlin noise (x,z) functions for humidity and temperature to decide on the biome at each coordinate. The heights for the four biomes are linearly interpolated by the temperature and then humidity values to ensure a smooth transition between biomes.
- The four biomes use different height generators with varying amounts of perlin 2d fmb and mixed noise to get the desired effect of rolling hills, smooth deserts, tall mountains, and icy plains. The biome at the (x,z) center of each chunk is sampled and then stored in the chunk's data for later use in terrain population.


### Procedurally Placed Assets - terrain population queue system
Assignee: Justin Duhamel
- Each biome has special assets that can be generated in the second stage of terrrain generation. population.
- There are some issues placing trees and structures in a chunk-based world. namely what if the object you're placing extends in a neighboring chunk that isn't generated?
- To solve these issues, I implemented a multithreaded dispatch queue system for terrain generation and population. First after the player enters the areas the chunks are created and send to the terrain generator workers to have their terrain generated. After terrain generation is completed they are added to a queue to await readiness for population. After all the neighboring chunks in a 3x3 around the chunk have completed terrain generation the chunk is sent to a population worker. This ensures that placed assets can extend into neighboring chunks. 
- The population worker will populate the chunk with assets based on the biome the chunk is in. In grassland forests it will fill it with procedural trees of varying height, in the snow biome it will place mega trees of varying layer height. These trees are generated from blueprints stored as arrays in the population generator and have a variable number of branch layers from 2-8. In the desert biome the worker will place cacti and rarely a large pyramid. The probabilities for these placements are determined by a biome lookup table and a random noise function based on the (x,z) coordinate.

### Efficient VBO creation and deletion
Assignee: Justin Duhamel
- There are several problems created when using multithreaded terrain generators and VBO generation threads. For example, what happens if a player leaves an area while a VBOworker is updating a chunk. If we delete the VBO buffers this will cause a segfault after the thread rejoins. Deleting the VBO data of chunks behind the player is necessary for good performance of mini Minecraft since eventually all VRAM will be saturated.
- to solve this I created a state machine and system of queues that manages VBO data and redrawing dirty chunks around the player. Every game tick the system will check the area around the player for dirty chunks that need to have their VBO data regenerated and send them to workers. It will also send chunks ahead of the player that don't yet have VBO buffers created or data to workers to have their data created. It will also compare the chunks around the player this tick and last tick and add any chunks that are now behind the player's path to a deletion queue. Once no other threads are using the chunks waiting in the deletion queue the VBO buffers and data will be deleted.
