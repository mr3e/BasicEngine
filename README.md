# BasicEngine
Basic 3D engine for computer graphics couce

main.cpp
  Created an input object and got picture from using input.getPicture() then added the result as a texture to the program and rendered it.

input.h:
  Added header file for the assignment included all of the required functions that calculate the final image.
 
input.cpp:
  We parsed the scene.txt file in the constructor and saved everything in vectors.
  Then, we calculated the required image in getPicture() using getHitCoord to get a hitPoint and getFinalColor to apply the colors from the light source.
