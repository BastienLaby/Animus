Animus
======
OpenGL advanced rendering demo with some funny features.

Installation (linux only)

  Move to the cloning directory path :
  > cd cloning_path/Animus/
  
  If you don't have premake4, install it :
  > sudo apt-get install premake4
  
  Generate makefile :
  > premake4 gmake
  
  Compile and link :
  > make
  
  This is a temporary issue to solve FMOD library linking :
  > export LD_LIBRARY_PATH=cloning_path/Animus/lib/fmod/lib/:$LD_LIBRARY_PATH
  
  Run the project :
  > ./Animusd

Coming features :
  - Camera Managin [OK]
  - Light Improvment (just pointLight for now)
