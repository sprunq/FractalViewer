#include "Fractal.h"
#include <iostream>


Fractal::Fractal(Image* img, bool dynamicIterations, float escapeRadius) {
    this->img = img;
    this->dynamic_iterations = dynamicIterations;
    this->max_iterations = 32;
    this->escape_radius = escapeRadius;
    setFractalType(FractalTypes::mandelbrot);
}

Fractal::~Fractal() {
}

void Fractal::setIterations(int amount) {
    this->max_iterations = amount;
}

int Fractal::getIterations() const {
    return max_iterations;
}

FractalTypes Fractal::getFractalType() {
    return this->fractal_type;
}

const char *Fractal::getName() {
    int item = (int)this->getFractalType();
    return this->FractalTypesNames[item-1];
}
// Set a new fractal and reset the view.
void Fractal::setFractalType(FractalTypes newFracType)
{
    FractalSettings limits_frac = { 0,0,0,0,0,0 };
    switch (newFracType)
    {
        case FractalTypes::mandelbrot:
            limits_frac = limit_mandelbrot;
            break;
        case FractalTypes::tricorn:
            limits_frac = limit_tricorn;
            break;
        case FractalTypes::mandelbrot_tricorn_animation:
            limits_frac = limit_mandelbrot_tricorn_animation;
            break;
        case FractalTypes::burning_ship:
            limits_frac = limit_burning_ship;
            break;
        default:
            limits_frac = { -2.5, 1.0, -1.0, 1.0 , 0, 0, 1.5 };
            break;
    }
    this->fractal_type = newFracType;
    this->max_iterations = 32;
    this->current_frac_settings.min_real_x = limits_frac.min_real_x * limits_frac.scale + limits_frac.offset_re_x;
    this->current_frac_settings.max_real_x = limits_frac.max_real_x * limits_frac.scale + limits_frac.offset_re_x;
    this->current_frac_settings.min_im_y = limits_frac.min_im_y * limits_frac.scale + limits_frac.offset_im_y;
    this->current_frac_settings.max_im_y = limits_frac.max_im_y * limits_frac.scale + limits_frac.offset_im_y;
}

// Interpolates two colors.
Color Fractal::linearInterpolation(const Color& col1, const Color& col2, double t)
{
    auto const b = 1 - t;
    return {static_cast<Uint8>((b * col1.r + t * col2.r)),
            static_cast<Uint8>((b * col1.g + t * col2.g)),
            static_cast<Uint8>((b * col1.b + t * col2.b))};
}

void Fractal::toggleIterationMode() {
    this->dynamic_iterations = !this->dynamic_iterations;
    this->max_iterations = 32;
}

FractalSettings Fractal::getFracSettings() const {
    return this->current_frac_settings;
}

void Fractal::setFracSettings(FractalSettings newSettings) {
    this->current_frac_settings = newSettings;
}

void Fractal::setImage(Image* newImage)
{
    this->img = newImage;
}

// Renders The Fractal Using OpenMp
void Fractal::renderFractal(vector<Color> colors, int width, int height, double time_delta) {
    if (this->dynamic_iterations) {
        this->max_iterations = static_cast<int>(50 * pow((log10(width / (current_frac_settings.max_im_y - current_frac_settings.min_im_y))), 1.25));
    }
#pragma omp parallel for
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double x0 = current_frac_settings.min_real_x + (current_frac_settings.max_real_x - current_frac_settings.min_real_x) * x / width;
            double y0 = current_frac_settings.min_im_y + (current_frac_settings.max_im_y - current_frac_settings.min_im_y) * y / height;
            double re = 0, im = 0, tmp;
            int current_iteration = 0;
            for (current_iteration; current_iteration < this->max_iterations; current_iteration++) {
                switch (fractal_type)
                {
                    case FractalTypes::mandelbrot:
                        tmp = re * re - im * im + x0;
                        im = 2.0 * re * im + y0;
                        re = tmp;
                        break;
                    case FractalTypes::tricorn:
                        tmp = re * re - im * im + x0;
                        im = -2 * re * im + y0;
                        re = tmp;
                        break;
                    case FractalTypes::mandelbrot_tricorn_animation:
                        tmp = re * re - im * im + x0;
                        im = 2.0 * sin(time_delta) * re * im + y0;
                        re = tmp;
                        break;
                    case FractalTypes::burning_ship:
                        tmp = re * re - im * im + x0;
                        im = 2.0 * std::abs(re * im) + y0;
                        re = tmp;
                        break;
                    case FractalTypes::experiment:
                        tmp = re * re - im * im + cos(x0);
                        im = fmod(tmp * im,2) + cos(y0);
                        re = fmod(cos(tmp*re)*4,2);
                        break;
                }
                if (re * re + im * im > 4) {
                    break;
                }
            }

            // Coloring
            // Has to be inside of this function because I can't figure out
            // how to use OpenMp with external functions calls
            if (current_iteration == this->max_iterations)
                current_iteration = 0;
            unsigned int max_color = colors.size() - 1;
            auto color_value = (static_cast<double>(current_iteration) / this->max_iterations) * max_color;
            auto i_col = static_cast<unsigned int>(color_value);
            Color color1 = colors[i_col];
            Color color2 = colors[min(i_col + 1, max_color)];
            Color col = linearInterpolation(color1, color2, color_value - i_col);
            this->img->setPixel(x, y, col);
        }
    }
}

