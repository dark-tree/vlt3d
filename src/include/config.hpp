#pragma once

// set to CL_TRUE or CL_FALSE,
// defines if OpenCL implementation should copy the given buffer before 
// sending it to the GPU. it sounds like a really cpu and memory heavy
// operation but I haven't observed any significant performance
// impact so i disabled it for the time being, as it can introduce
// problems if the buffer to be sent is modified (or deleted) between
// issuing the copy command and execution of OpenCL kernel
#define LT3D_OPENCL_COPY_ON_WRITE CL_TRUE

// set to true or false,
// defines what the renderer should do when flushing OpenGL state
// setting this to false ensures maximum driver compatibility
// setting this to true makes the renderer use a shortcut that can fail on SOME drivers
#define LT3D_FAST_GLFINISH false

// set to true or false,
// defines if the fast buffer sould be verbose about its realocations,
// this can be useful for debugging that little pice of shit
#define LT3D_VERBOSE_FAST_BUFFER true
