# Assigment one

Load points which define control points of a B-spline. Load an object defined by a .obj file and draw it. 
Animate that object to move along the approximation or interpolation B-spline.

## Requirements
  1. freeglut library
  2. assimp library
  3. g++
  
## Build
  $ g++ -std=c++17 *.cpp -o main -lglut -lGLU -lGL -lassimp

## Run
  $ ./main teddy.obj b_spline_one approximation 0
