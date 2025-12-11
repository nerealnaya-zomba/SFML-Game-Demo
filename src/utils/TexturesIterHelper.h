#pragma once
struct texturesIterHelper
{   
    int iterationCounter{};
    int iterationsTillSwitch{};
    //Points element of textures array, that needs to be assigned to all sprites; 
    int ptrToTexture{};
    //Needed for forward-backward iterations based on goForward
    int countOfTextures{};
    //If countOfTextures == texturesArr.size(), then goForward == false
    //Is used for correct displaying sprites textures(forward-backwards loop)
    bool goForward = true;
};