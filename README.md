# ZeroGPU (WIP)
A CPU-Based software renderer for the Raspberry Pi Zero 2 W. Zero "dedicated" GPU!

## Currently working but I am fixing bugs before public release.

## Donut
<img width="400" height="310" alt="image" src="https://github.com/user-attachments/assets/69408224-4923-4918-b445-685b3809bd62" />
<img width="400" height="310" alt="image" src="https://github.com/user-attachments/assets/8005a854-48f0-456e-81cc-1b002709c9cb" />

## Cube
<img width="400" height="310" alt="image" src="https://github.com/user-attachments/assets/73391a00-cc86-4cad-9fee-a983d867f797" />
<img width="400" height="310" alt="image" src="https://github.com/user-attachments/assets/c3a74053-f6b4-451b-9af1-923982dfd3ac" />



# About 
- I made this project to learn more about the OpenGL, DirectX, and Vulkan Graphics Pipeline. 
- Despite the name, the PI Zero 2 W is just the platform this software will be developed on. Theoretically, you could run this renderer on anything from a powerful desktop to a Samsung Smart Fridge. I'm not your boss.
- Think of this less of a hardware project, and more of a "can we make a microcontroller optimized version of OpenGL?"
- I picked the PI Zero 2 W for a couple reasons.
    - It's really cheap. (15$)
    - While there is an even less powerful and even cheaper microcontroller (Pi Pico), I wouldn't be able to use C++, and most of the effort wouldn't be on the renderer but just getting to start point of the Pi Zero.
  
# How it Works
- The Graphics Pipeline is OBJ load → Vertex transform (rotation + scale) → Projection → Triangle assembly → Rasterization (barycentric + Z-buffer) → Framebuffer output → Display (SDL2)
