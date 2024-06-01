#include <SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <Windows.h>
#include <vector>
#include <iostream>
#include <random>
#include <fstream>
#include <string>
#include <omp.h>

using namespace std;

const int WINDOW_WIDTH = GetSystemMetrics(SM_CXSCREEN);
const int WINDOW_HEIGHT = GetSystemMetrics(SM_CYSCREEN);

SDL_Window* window = nullptr;
SDL_GLContext context;
GLuint vbo;
const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
bool running = true;

int targetFPS = 60;
float friction = 0.99f;
int pull_distance = 400;
float pull_strength = 1;
int ball_number = 1'000'000;

const int frameDelay = 1000 / targetFPS;
Uint32 frameStart, frameTime;

struct Point {
    float x, y;
    float r, g, b;
    float velX, velY;
};

vector<Point> points;

void setColor(Point& point, float speed) {
    int i;
    float f, p, q, t;
    float h, s, v;

    h = min(0.95f, max(0.0f, speed / 60));
    s = 1.0f;
    v = min(0.95f, max(0.0f, speed / 5));

    h *= 6;
    i = static_cast<int>(floor(h));
    f = h - i;
    p = v * (1 - s);
    q = v * (1 - s * f);
    t = v * (1 - s * (1 - f));

    switch(i) {
    case 0:
        point.r = v; point.g = t; point.b = p;
        break;
    case 1:
        point.r = q; point.g = v; point.b = p;
        break;
    case 2:
        point.r = p; point.g = v; point.b = t;
        break;
    case 3:
        point.r = p; point.g = q; point.b = v;
        break;
    case 4:
        point.r = t; point.g = p; point.b = v;
        break;
    default:
        point.r = v; point.g = p; point.b = q;
        break;
    }
}

void movePoints() {
    #pragma omp parallel for
    for (int i = 0; i < points.size(); ++i) {
        Point& point = points[i];

        point.x += point.velX * 0.5f;
        point.y += point.velY * 0.5f;

        if (point.x < 0 || point.x >= WINDOW_WIDTH) {
            point.velX = -point.velX;
        }
        if (point.y < 0 || point.y >= WINDOW_HEIGHT) {
            point.velY = -point.velY;
        }

        point.velX *= friction;
        point.velY *= friction;

        point.x = min(WINDOW_WIDTH - 1, max(0, point.x));
        point.y = min(WINDOW_HEIGHT - 1, max(0, point.y));

        float speed = sqrt(point.velX * point.velX + point.velY * point.velY);
        setColor(point, speed);
    }
}

void movePointsRealitiveTo(int posX, int posY, INT8 direction) {
    #pragma omp parallel for
    for (int i = 0; i < points.size(); ++i) {
        Point& point = points[i];
        float xDist, yDist, distance;

        xDist = point.x - posX;
        yDist = point.y - posY;
        distance = sqrt(xDist * xDist + yDist * yDist);

        if (distance <= pull_distance) {
            if (distance == 0.0f) {
                distance = 0.0001;
            }

            float ratio = pull_strength / distance;

            point.velX += xDist * ratio * direction;
            point.velY += yDist * ratio * direction;
        }
    }
}

void handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        else if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }
    }

    int mouseX, mouseY;
    Uint32 mouseState = SDL_GetMouseState(&mouseX, &mouseY);

    if (mouseState & SDL_BUTTON(1))
    {
        movePointsRealitiveTo(mouseX, mouseY, -1);
    }
    else if (mouseState & SDL_BUTTON(3))
    {
        movePointsRealitiveTo(mouseX, mouseY, 1);
    };
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);

    glLoadIdentity();

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Point) * points.size(), points.data());
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(Point), reinterpret_cast<void*>(0));
    glColorPointer(3, GL_FLOAT, sizeof(Point), reinterpret_cast<void*>(8));

    glDrawArrays(GL_POINTS, 0, points.size());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    SDL_GL_SwapWindow(window);
}

void readConfigFile(const string& filename) {
    ifstream file(filename);

    string line;

    while (getline(file, line)) {
        size_t delimiterPos = line.find('=');

        if (delimiterPos != string::npos) {
            string key = line.substr(0, delimiterPos);
            string value = line.substr(delimiterPos + 1);

            if (key == "friction") {
                friction = stof(value);
            }
            else if (key == "pull_distance") {
                pull_distance = stoi(value);
            }
            else if (key == "pull_strength") {
                pull_strength = stof(value);
            }
            else if (key == "ball_number") {
                ball_number = stoi(value);
            }
        }
    }

    file.close();
}

void initVBO() {
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * points.size(), points.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void initPoints() {
    random_device dev;
    mt19937 rng(dev());
    uniform_real_distribution<float> x_rng(1.0f, static_cast<float>(WINDOW_WIDTH - 1));
    uniform_real_distribution<float> y_rng(1.0f, static_cast<float>(WINDOW_HEIGHT - 1));

    for (int i = 0; i < ball_number; i++) {
        Point p;
        p.x = x_rng(rng);
        p.y = y_rng(rng);
        p.r = 0.0f;
        p.g = 0.0f;
        p.b = 0.5f;
        p.velX = 0.0f;
        p.velY = 0.0f;
        points.push_back(p);
    }
}

void init() {
    readConfigFile("config.txt");

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        cerr << "Failed to initialize SDL: " << SDL_GetError() << endl;
        exit(EXIT_FAILURE);
    }

    window = SDL_CreateWindow("OpenGL 2D", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
    if (!window) {
        cerr << "Failed to create SDL window: " << SDL_GetError() << endl;
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    context = SDL_GL_CreateContext(window);
    if (!context) {
        cerr << "Failed to create OpenGL context: " << SDL_GetError() << endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    GLenum err = glewInit();
    if (err != GLEW_OK) {
        cerr << "Failed to initialize GLEW: " << glewGetErrorString(err) << endl;
        SDL_GL_DeleteContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
        exit(EXIT_FAILURE);
    }

    initPoints();
    initVBO();

    glDisable(GL_DEPTH_TEST);
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, WINDOW_WIDTH, WINDOW_HEIGHT, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void cleanup() {
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main(int argc, char* argv[]) {
    init();

    while (running) {
        frameStart = SDL_GetTicks();

        handleEvents();
        movePoints();
        render();

        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime) {
            SDL_Delay(frameDelay - frameTime);
        }
    }

    cleanup();
    return 0;
}
