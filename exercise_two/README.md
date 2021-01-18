# Assigment two

Load a file defining a texture of a particle. Create a random system of particles which does simulate some real system(fire, smoke, ...).
Rotate every particle towards the camera using bilboarding technique.

The solution demonstrates snow falling as a endless system. 
Note: start animation with 'f'
Note: use right click for menu 

## Requirements
  1. freeglut library
  2. SOIL library
  3. g++
  
## Build
  $ g++ -std=c++17 *.cpp -o main -lglut -lGLU -lGL -lSOIL

## Run
  $ ./main
