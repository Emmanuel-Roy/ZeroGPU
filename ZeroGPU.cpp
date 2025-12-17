#include <SDL2/SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#define FIX16_SHIFT 16
#define FIX16_ONE (1 << FIX16_SHIFT)
typedef int32_t fix16_t;


fix16_t floatToFx(float f) { return (fix16_t)(f * FIX16_ONE); }
float fxToFloat(fix16_t f) { return (float)f / FIX16_ONE; }
fix16_t fix16_mul(fix16_t a, fix16_t b) { return (fix16_t)(((int64_t)a * b) >> FIX16_SHIFT); }
fix16_t fix16_add(fix16_t a, fix16_t b) { return a + b; }
fix16_t fix16_sub(fix16_t a, fix16_t b) { return a - b; }


struct Vec3 { fix16_t x, y, z; };   // Vertex
struct Triangle { Vec3 v0, v1, v2; }; 

// --- Output to Screen ---
const int WIDTH = 800;
const int HEIGHT = 600;
float zbuffer[WIDTH * HEIGHT];


// STAGE 1: Initialize z-buffer each frame
void clearZBuffer() { std::fill(zbuffer, zbuffer + WIDTH * HEIGHT, 1e9f); }

// STAGE 2: Transform 3D vertices into 2D screen coordinates
void project(const Vec3 &v, int &x, int &y, float &z, float scale) {
    float xf = fxToFloat(v.x) * scale;
    float yf = fxToFloat(v.y) * scale;
    float zf = fxToFloat(v.z) + 3.0f; // translate away from camera
    x = int(WIDTH / 2 + xf * 200 / zf);
    y = int(HEIGHT / 2 - yf * 200 / zf);
    z = zf;
}

// --- Sin/Cos lookup table (for our object rotation) ---
constexpr int SIN_RES = 360;
fix16_t sin_tbl[SIN_RES], cos_tbl[SIN_RES];
void initSinCosTable() {
    for (int i = 0; i < SIN_RES; i++) {
        float rad = 2.0f * M_PI * i / SIN_RES;
        sin_tbl[i] = floatToFx(sin(rad));
        cos_tbl[i] = floatToFx(cos(rad));
    }
}
fix16_t sin_fx(int idx) { return sin_tbl[idx % SIN_RES]; }
fix16_t cos_fx(int idx) { return cos_tbl[idx % SIN_RES]; }


// STAGE 3: Rotate vertices around Y-axis
Vec3 rotateY(const Vec3 &v, int angle_idx) {
    fix16_t s = sin_fx(angle_idx);
    fix16_t c = cos_fx(angle_idx);
    Vec3 r;
    r.x = fix16_sub(fix16_mul(v.x, c), fix16_mul(v.z, s));
    r.y = v.y;
    r.z = fix16_add(fix16_mul(v.x, s), fix16_mul(v.z, c));
    return r;
}

// STAGE 3b: Apply vertex transforms and project to screen space
struct ScreenTri {
    int x0, y0, x1, y1, x2, y2;
    float z0, z1, z2;
};
ScreenTri transformTriangle(const Triangle &tri, int angle_idx, float scale) {
    ScreenTri st;
    Vec3 r0 = rotateY(tri.v0, angle_idx);
    Vec3 r1 = rotateY(tri.v1, angle_idx);
    Vec3 r2 = rotateY(tri.v2, angle_idx);
    project(r0, st.x0, st.y0, st.z0, scale);
    project(r1, st.x1, st.y1, st.z1, scale);
    project(r2, st.x2, st.y2, st.z2, scale);
    return st;
}

float edgeFunction(int x0,int y0,int x1,int y1,int x,int y){
    return float((y0 - y1)*x + (x1 - x0)*y + x0*y1 - x1*y0);
}

// STAGE 4: Barycentric rasterization + Z-buffer
void drawTriangle(SDL_Surface *surf, const ScreenTri &tri, bool wireframe) {
    if(wireframe){
        // Draw only edges (wireframe mode)
        SDL_LockSurface(surf);
        auto putPixel=[&](int x,int y,uint32_t color){
            if(x>=0 && x<WIDTH && y>=0 && y<HEIGHT){
                ((uint32_t*)surf->pixels)[y*WIDTH + x] = color;
            }
        };
        auto drawLine=[&](int x0,int y0,int x1,int y1){
            int dx=std::abs(x1-x0), dy=std::abs(y1-y0);
            int sx=(x0<x1)?1:-1, sy=(y0<y1)?1:-1;
            int err=dx-dy;
            while(true){
                putPixel(x0,y0,0xFFFFFFFF);
                if(x0==x1 && y0==y1) break;
                int e2=2*err;
                if(e2> -dy){ err-=dy; x0+=sx; }
                if(e2 < dx){ err+=dx; y0+=sy; }
            }
        };
        drawLine(tri.x0,tri.y0,tri.x1,tri.y1);
        drawLine(tri.x1,tri.y1,tri.x2,tri.y2);
        drawLine(tri.x2,tri.y2,tri.x0,tri.y0);
        SDL_UnlockSurface(surf);
        return;
    }

    int minX = std::max(0, std::min({tri.x0,tri.x1,tri.x2}));
    int maxX = std::min(WIDTH-1,std::max({tri.x0,tri.x1,tri.x2}));
    int minY = std::max(0, std::min({tri.y0,tri.y1,tri.y2}));
    int maxY = std::min(HEIGHT-1,std::max({tri.y0,tri.y1,tri.y2}));

    float area = edgeFunction(tri.x0,tri.y0,tri.x1,tri.y1,tri.x2,tri.y2);
    if(area==0) return;

    uint32_t *pixels = (uint32_t*)surf->pixels;

    for(int y=minY;y<=maxY;y++){
        for(int x=minX;x<=maxX;x++){
            float w0=edgeFunction(tri.x1,tri.y1,tri.x2,tri.y2,x,y)/area;
            float w1=edgeFunction(tri.x2,tri.y2,tri.x0,tri.y0,x,y)/area;
            float w2=edgeFunction(tri.x0,tri.y0,tri.x1,tri.y1,x,y)/area;
            if(w0>=0 && w1>=0 && w2>=0){
                float z = w0*tri.z0 + w1*tri.z1 + w2*tri.z2; // depth interpolation
                int idx = y*WIDTH + x;
                if(z < zbuffer[idx]){
                    zbuffer[idx] = z;
                    pixels[idx] = 0xFFFFFFFF; // Fragment color
                }
            }
        }
    }
}

// STAGE 0b: Input vertex/triangle data
bool loadOBJ(const std::string &filename, std::vector<Vec3> &vertices, std::vector<Triangle> &triangles){
    std::ifstream file(filename);
    if(!file) return false;
    std::string line;
    while(std::getline(file,line)){
        std::stringstream ss(line);
        std::string tok;
        ss>>tok;
        if(tok=="v"){
            float x,y,z;
            ss>>x>>y>>z;
            vertices.push_back({floatToFx(x),floatToFx(y),floatToFx(z)});
        }else if(tok=="f"){
            std::string a,b,c;
            ss>>a>>b>>c;
            int ia = std::stoi(a.substr(0,a.find('/')))-1;
            int ib = std::stoi(b.substr(0,b.find('/')))-1;
            int ic = std::stoi(c.substr(0,c.find('/')))-1;
            triangles.push_back({vertices[ia],vertices[ib],vertices[ic]});
        }
    }
    return true;
}

int SDL_main(int argc, char **argv){
    std::string filename;
    std::cout<<"Enter path to .obj file: ";
    std::getline(std::cin, filename);

    std::vector<Vec3> vertices;
    std::vector<Triangle> triangles;
    if(!loadOBJ(filename,vertices,triangles)){
        std::cout<<"Failed to load OBJ\n";
        return 1;
    }

    if(SDL_Init(SDL_INIT_VIDEO)<0){ std::cout<<"SDL Init fail\n"; return 1; }
    SDL_Window *win=SDL_CreateWindow("ZeroGPU",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,WIDTH,HEIGHT,0);
    SDL_Surface *surf = SDL_GetWindowSurface(win);
    initSinCosTable();

    bool running = true;
    SDL_Event e;
    int angle = 0;
    bool wireframe=false;
    float scale=1.0f;

    while(running){
        // STAGE 0c: Input
        while(SDL_PollEvent(&e)){
            if(e.type==SDL_QUIT) running=false;
            if(e.type==SDL_KEYDOWN){
                if(e.key.keysym.sym==SDLK_SPACE) wireframe=!wireframe;
                if(e.key.keysym.sym==SDLK_PLUS || e.key.keysym.sym == SDLK_EQUALS || e.key.keysym.sym==SDLK_KP_PLUS) scale*=1.1f;
                if(e.key.keysym.sym==SDLK_MINUS || e.key.keysym.sym==SDLK_KP_MINUS) scale/=1.1f;
            }
        }

        // STAGE 1: Clear buffers
        SDL_FillRect(surf,nullptr,0x000000);
        clearZBuffer();

        // STAGE 2-4: Transform, project, rasterize each triangle
        for(const auto &tri : triangles){
            ScreenTri st = transformTriangle(tri,angle,scale);
            drawTriangle(surf,st,wireframe);
        }

        // STAGE 5: Display / swap buffers
        SDL_UpdateWindowSurface(win);

        // STAGE 3/animation update
        angle = (angle+1)%SIN_RES;

        SDL_Delay(16);
    }

    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
