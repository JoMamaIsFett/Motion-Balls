#include <SDL.h>
#include <vector>
#include <cmath>
#include <string>
#include <random>
#include <iostream>
#include <Windows.h>

using namespace std;


const int width = GetSystemMetrics(SM_CXSCREEN);
const int height = GetSystemMetrics(SM_CYSCREEN);

SDL_Window* window = SDL_CreateWindow("motionBalls", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN);
SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
SDL_Event e;
bool running = true;

const int targetFPS = 60;
const float friction = 0.99f;
const int pull_distance = 400;
const float pull_strength = 1;
const int ball_number = 20000;

const int frameDelay = 1000 / targetFPS;
Uint32 frameStart, frameTime;


SDL_Color HSVtoRGB(float h, float s, float v) {
    int i;
    float f, p, q, t;

    h *= 360;

    if (s == 0) {
        return SDL_Color{ static_cast<Uint8>(v * 255), static_cast<Uint8>(v * 255), static_cast<Uint8>(v * 255), 255 };
    }

    h /= 60;
    i = static_cast<int>(floor(h));
    f = h - i;
    p = v * (1 - s);
    q = v * (1 - s * f);
    t = v * (1 - s * (1 - f));

    switch (i) {
    case 0:
        return SDL_Color{ static_cast<Uint8>(v * 255), static_cast<Uint8>(t * 255), static_cast<Uint8>(p * 255), 255 };
    case 1:
        return SDL_Color{ static_cast<Uint8>(q * 255), static_cast<Uint8>(v * 255), static_cast<Uint8>(p * 255), 255 };
    case 2:
        return SDL_Color{ static_cast<Uint8>(p * 255), static_cast<Uint8>(v * 255), static_cast<Uint8>(t * 255), 255 };
    case 3:
        return SDL_Color{ static_cast<Uint8>(p * 255), static_cast<Uint8>(q * 255), static_cast<Uint8>(v * 255), 255 };
    case 4:
        return SDL_Color{ static_cast<Uint8>(t * 255), static_cast<Uint8>(p * 255), static_cast<Uint8>(v * 255), 255 };
    default:
        return SDL_Color{ static_cast<Uint8>(v * 255), static_cast<Uint8>(p * 255), static_cast<Uint8>(q * 255), 255 };
    }
}

class Ball
{
private:
    SDL_Color color;
    SDL_Point middle;
    float radius = 5;
    static const int resolution = 7;
    const double angle_step = (2 * M_PI) / resolution;
    float Xoffset[resolution + 1];
    float Yoffset[resolution + 1];
    SDL_Vertex vertices[resolution * 3];

    float speed_x = 0;
    float speed_y = 0;

    void calculate_vertices()
    {
        setColor();

        for (int i = 0; i < resolution; i++)
        {
            float Xmiddle = static_cast<float>(middle.x);
            float Ymiddle = static_cast<float>(middle.y);

            vertices[i * 3] = { { Xmiddle, Ymiddle }, color, 0 };
            vertices[i * 3 + 1] = { { Xoffset[i] + Xmiddle, Yoffset[i] + Ymiddle }, color, 0 };
            vertices[i * 3 + 2] = { { Xoffset[i + 1] + Xmiddle, Yoffset[i + 1] + Ymiddle }, color, 0 };
        }
    }

    void calculate_offsets()
    {
        double angle = -M_PI;

        for (int i = 0; i <= resolution; i++)
        {
            Xoffset[i] = cos(angle) * radius;
            Yoffset[i] = sin(angle) * radius;

            angle += angle_step;
        };
    }

public:
    Ball(SDL_Point m)
    {
        middle = m;
        calculate_offsets();
        calculate_vertices();
    }

    void setColor()
    {
        float speed = sqrt(speed_x * speed_x + speed_y * speed_y);

        float hue = min(0.95f, max(0.0f, speed / 60));
        float saturation = 1.0;
        float value = min(0.95f, max(0.0f, speed / 5));

        color = HSVtoRGB(hue, saturation, value);
    }

    void move()
    {
        middle.x += speed_x * 0.5;
        middle.y += speed_y * 0.5;

        if (0 > middle.x || middle.x > width)
        {
            speed_x = -speed_x;
        };
        if (0 > middle.y || middle.y > height)
        {
            speed_y = -speed_y;
        };

        speed_x *= friction;
        speed_y *= friction;

        middle.x = min(width, max(0, middle.x));
        middle.y = min(height, max(0, middle.y));

        calculate_vertices();
    }

    void move_to(float pos_x, float pos_y, string direction)
    {
        float x_distance = pos_x - middle.x;
        if (x_distance > pull_distance)
        {
            return;
        }

        float y_distance = pos_y - middle.y;
        if (y_distance > pull_distance)
        {
            return;
        }

        float distance = sqrt(x_distance * x_distance + y_distance * y_distance);

        if (distance <= pull_distance)
        {
            if (distance == 0)
            {
                distance = 0.0001;
            };

            float ratio = pull_strength / distance;
            float x_pull = x_distance * ratio;
            float y_pull = y_distance * ratio;

            if (direction == "+")
            {
                speed_x += x_pull;
                speed_y += y_pull;
            }
            else if (direction == "-")
            {
                speed_x -= x_pull;
                speed_y -= y_pull;
            };
        };
        
        calculate_vertices();
    }
    
    void render()
    {
        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_RenderGeometry(renderer, NULL, vertices, size(vertices), NULL, 0);
    }
};

vector<Ball> balls;

void render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    for (Ball& ball : balls)
    {
        ball.render();
    }
    SDL_RenderPresent(renderer);
};

void mousehandler()
{
    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

    if (mouseState & SDL_BUTTON(1))
    {
        for (Ball& ball : balls)
        {
            ball.move_to(mouseX, mouseY, "+");
        }
    }
    else if (mouseState & SDL_BUTTON(3))
    {
        for (Ball& ball : balls)
        {
            ball.move_to(mouseX, mouseY, "-");
        };
    };
};

void move_balls()
{
    for (Ball& ball : balls)
    {
        ball.move();
    }
}

int main(int argc, char* argv[])
{
    random_device dev;
    mt19937 rng(dev());
    uniform_int_distribution<std::mt19937::result_type> dist_width(1, width - 1);
    uniform_int_distribution<std::mt19937::result_type> dist_height(1, height - 1);

    for (int i = 0; i < ball_number; i++)
    {
        balls.push_back(Ball(SDL_Point{ static_cast<int>(dist_width(rng)), static_cast<int>(dist_height(rng)) }));
    };

    while (running)
    {
        frameStart = SDL_GetTicks();

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
            if (e.type == SDL_KEYDOWN)
            {
                if (e.key.keysym.sym == SDLK_ESCAPE)
                {
                    running = false;
                }
            }
        }

        move_balls();
        mousehandler();
        render();

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
