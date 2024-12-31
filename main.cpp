#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <cmath>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const double G = 6.67430e-11; // Gravitational constant

struct Vector2 {
    double x, y;

    Vector2 operator+(const Vector2& v) const {
        return {x + v.x, y + v.y};
    }

    Vector2 operator-(const Vector2& v) const {
        return {x - v.x, y - v.y};
    }

    Vector2 operator*(double scalar) const {
        return {x * scalar, y * scalar};
    }

    double magnitude() const {
        return std::sqrt(x * x + y * y);
    }

    Vector2 normalize() const {
        double mag = magnitude();
        return {x / mag, y / mag};
    }
};

struct Planet {
    Vector2 position;
    Vector2 velocity;
    double mass;
    int radius;
    SDL_Color color;

    void updatePosition(double dt) {
        position = position + velocity * dt;
    }
};

Vector2 calculateGravitationalForce(const Planet& a, const Planet& b) {
    Vector2 direction = b.position - a.position;
    double distance = direction.magnitude();
    double force = (G * a.mass * b.mass) / (distance * distance);
    return direction.normalize() * force;
}

void renderPlanet(SDL_Renderer* renderer, const Planet& planet) {
    SDL_SetRenderDrawColor(renderer, planet.color.r, planet.color.g, planet.color.b, 255);
    SDL_Rect rect = {
        static_cast<int>(planet.position.x) - planet.radius,
        static_cast<int>(planet.position.y) - planet.radius,
        planet.radius * 2,
        planet.radius * 2
    };
    SDL_RenderFillRect(renderer, &rect);
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "Planetary Orbit Simulation",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Define planets
    std::vector<Planet> planets = {
        {{400, 400}, {0, 0}, 1.989e30, 15, {255, 255, 0}}, // Sun
        {{600, 400}, {0, -30000}, 5.972e24, 5, {0, 0, 255}}, // Earth
        {{700, 400}, {0, -24000}, 6.39e23, 4, {255, 0, 0}}  // Mars
    };

    bool running = true;
    SDL_Event event;
    double dt = 1.0; // Time step

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Clear screen
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Update positions and velocities
        for (size_t i = 0; i < planets.size(); ++i) {
            Vector2 netForce = {0, 0};
            for (size_t j = 0; j < planets.size(); ++j) {
                if (i != j) {
                    netForce = netForce + calculateGravitationalForce(planets[i], planets[j]);
                }
            }
            Vector2 acceleration = netForce * (1.0 / planets[i].mass);
            planets[i].velocity = planets[i].velocity + acceleration * dt;
        }

        for (auto& planet : planets) {
            planet.updatePosition(dt);
        }

        // Render planets
        for (const auto& planet : planets) {
            renderPlanet(renderer, planet);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // Approx 60 FPS
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
