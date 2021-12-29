#pragma once

static const float Pi = 3.14159265358979323846;
namespace Janus {
    inline float Radians(float deg) { 
    return (Pi / 180) * deg; 
    }
}