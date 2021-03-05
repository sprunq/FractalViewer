#ifndef FRACTALVIEWER_FRACTAL_H
#define FRACTALVIEWER_FRACTAL_H

#include <SFML/Graphics.hpp>
#include <omp.h>
#include <cstdio>
#include <cmath>
using namespace std;
using namespace sf;

enum class FractalTypes {
    mandelbrot = 1,
    tricorn,
    mandelbrot_tricorn_animation,
    burning_ship
};

struct FractalSettings {
    double min_real_x;
    double max_real_x;
    double min_im_y;
    double max_im_y;
    double offset_re_x;
    double offset_im_y;
    float scale;
};

class Fractal {
    const char * FractalTypesNames[4] = {"Mandelbrot", "Tricorn", "Ma-Tri Animation", "Burning Ship"};
    FractalSettings limit_mandelbrot = { -2.5, 1.0, -1.0, 1.0 , 0.5, 0, 1.5 };
    FractalSettings limit_tricorn = { -2.5, 1.0, -1.0, 1.0 , 1.5, 0, 2 };
    FractalSettings limit_mandelbrot_tricorn_animation = { -2.5, 1.0, -1.0, 0.75 , 1, 0, 2 };
    FractalSettings limit_burning_ship = { -2.5, 1.0, -1.0, 1.0 , 1, -0.75, 1.5 };
    FractalSettings current_frac_settings{};
    Image* img;
    FractalTypes fractal_type;
    float escape_radius;
    bool dynamic_iterations;
    int max_iterations;
    static Color linearInterpolation(const Color& col1, const Color& col2, double t);

public:
    Fractal(Image& img, bool dynamicIterations, float escapeRadius);
    ~Fractal();
    int getIterations() const;
    void setIterations(int amount);
    int getFractalType();
    void setFractalType(FractalTypes newFracType);
    const char* getName();
    FractalSettings getFracSettings() const;
    void setFracSettings(FractalSettings newSettings);
    void toggleIterationMode();
    void renderFractal(vector<Color> colors, int width, int height, double time_delta);
};

#endif //FRACTALVIEWER_FRACTAL_H
