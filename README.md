# Image-Effects

How to Compile the Program:
1. Change into the CPSC453A2 directory
2. In terminal, type: make
3. In terminal, type: ./boilerplate.out

How to Operate the Program:
1. Press 1,2,…6 to switch between respective images.
2. Press the R key on the keyboard to rotate image
3. Scroll up to zoom in, scroll down to zoom out
4. Click the left button on the mouse to change position of image on screen.
5. Press A for luminance effect 1, S for luminance effect 2, D for luminance effect 3
Press F for Sophia effect
6. Press H for Sobel horizontal
7. Press J for Sobel vertical
8. Press K for Sobel unsharp mask
9. Press L for gaussian blur for 3x3 kernel 
10.Press Q for gaussian blur for 5x5 kernel
11. Press W for gaussian blur for 7x7 kernel
12. Press 0 to get original image back after selected effect


Platform:
Ubuntu 16.4

Compiler:
g++ 5.4.0 20160609


Please note:
When working on this at home, I had to change the openGL core version from 4.1 to 3.3. By doing that, I also had to change the version numbers in fragment.glsl and vertex.glsl from  #410 to #330. 
