#pragma once
#include "System.h"

class SetInputSystem :
    public System
{
public:
    //For now just takes all input. Have plans to change components input depending on node tree and its settings.
    void SetInput();
};

