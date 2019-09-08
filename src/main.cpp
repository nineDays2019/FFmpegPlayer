#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

using namespace std;

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        cout << "SDL_Init Error: " << SDL_GetError() << endl;
        return 1;
    }
    SDL_Window *win = SDL_CreateWindow("Hello World!",
                                       100, 100, 871, 564,
                                       SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        cout << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        return 1;
    }
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1,
                                           SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr) {
        cout << "SDL_CreateRenderer Error: " << SDL_GetError() << endl;
        return 1;
    }
    SDL_Surface *surface = IMG_Load("./img/demo.jpg");
    if (surface == nullptr) {
        cout << "SDL_LoadBMP Error: " << SDL_GetError() << endl;
        return 1;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(ren, surface);
    SDL_FreeSurface(surface);
    if (texture == nullptr) {
        cout << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << endl;
        return 1;
    }
    SDL_RenderClear(ren);
    SDL_RenderCopy(ren, texture, NULL, NULL);
    SDL_RenderPresent(ren);

    SDL_Delay(2000);

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);

    SDL_Quit();
    cout << "Hello" << endl;
    return 0;
}