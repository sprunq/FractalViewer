#include <stdio.h>
#include <string>
#include <SFML/Graphics.hpp>
#include <iostream>
#include "ArialFont.h"
#include "color_palettes.h"
#include <math.h> 
using namespace std;
using namespace sf;

struct Limits {
	const int img_width;
	const int img_height;
	double min_real_x;
	double max_real_x;
	double min_im_y;
	double max_imaginary_y;
};

enum FractalTypes {
	MANDELBROT = 1,
	BURNING_SHIP,
	TRICORN,
	MANDELBROT_TRICORN_ANIMATION
};

// Constants
const auto aspect_ratio = 16.0 / 9.0;
const int img_width = 1000;
const int img_height = static_cast<int>(img_width / aspect_ratio);

// UI Settings
const float zoom_factor = 1.5;
const float move_factor = 0.05;
bool show_sys_info = true;
bool zoom_center = false;

// Fractal Settings
int max_iterations = 32;
int num_of_colors = 2;
FractalTypes fractal_type = MANDELBROT;


Limits limits = { img_width, img_height, -2.5, 1, -1, 1 };

void renderFractal(Image& img, double& sum_of_iterations, vector<Color> colors, int time);
void screen_zoom(tuple<int, int> cursor_pos, double factor, bool zoom_center);
Color linear_interpolation(const Color& v, const Color& u, double a);

int main()
{
	srand(time(NULL));
	vector<Color> colors = getRandomColors(num_of_colors);

	double zoom_val = 1;
	double sum_of_iterations = 0;
	double time = 0;

	Image img;
	Texture texture;
	Sprite sprite;
	Font font;
	Text text;
	Clock clock;
	Event event;

	RenderWindow window(VideoMode(img_width, img_height), "Mandelbrot Set");
	img.create(img_width, img_height);

	if (!font.loadFromMemory(&arial_ttf, arial_ttf_len))
	{
		cout << "Loading Font Failed!" << endl;
		return 0;
	}
	text.setFont(font);
	text.setCharacterSize(18);
	text.setFillColor(Color::White);

	while (window.isOpen()) {
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) {
				window.close();
			}
			if (event.type == Event::KeyPressed) {
				// Moving around with ASWD
				double width_step = (limits.max_real_x - limits.min_real_x) * move_factor;
				double height_step = (limits.max_imaginary_y - limits.min_im_y) * move_factor;
				if (event.key.code == Keyboard::A) {
					limits.min_real_x -= width_step;
					limits.max_real_x -= width_step;
				}
				if (event.key.code == Keyboard::D) {
					limits.min_real_x += width_step;
					limits.max_real_x += width_step;
				}
				if (event.key.code == Keyboard::W) {
					limits.min_im_y -= height_step;
					limits.max_imaginary_y -= height_step;
				}
				if (event.key.code == Keyboard::S) {
					limits.min_im_y += height_step;
					limits.max_imaginary_y += height_step;
				}

				// Get Random Colors
				if (event.key.code == Keyboard::Space) {
					colors = getRandomColors(num_of_colors);
				}

				// Print Colors to console
				if (event.key.code == Keyboard::Enter) {
					saveColors(colors);
				}

				// Reset
				if (event.key.code == Keyboard::R) {
					max_iterations = 32;
					zoom_val = 1;
					limits.min_real_x = -2.5;
					limits.max_real_x = 1;
					limits.min_im_y = -1;
					limits.max_imaginary_y = 1;
				}

				// Toggle System Info
				if (event.key.code == Keyboard::F) {
					show_sys_info = !show_sys_info;
					text.setString("");
				}

				// Switch between different fractals
				if (event.key.code == Keyboard::Num1) {
					fractal_type = MANDELBROT;
				}
				if (event.key.code == Keyboard::Num2) {
					fractal_type = BURNING_SHIP;
				}
				if (event.key.code == Keyboard::Num3) {
					fractal_type = TRICORN;
				}
				if (event.key.code == Keyboard::Num4) {
					fractal_type = MANDELBROT_TRICORN_ANIMATION;
				}
			}

			if (event.type == Event::MouseButtonPressed) {
				// Iterations
				if (event.mouseButton.button == Mouse::Left) {
					max_iterations *= 2;

				}
				else if (event.mouseButton.button == Mouse::Right) {
					max_iterations /= 2;
					if (max_iterations < 1)
						max_iterations = 1;
				}
			}

			if (event.type == Event::MouseWheelScrolled)
			{
				// Zoom
				if (event.MouseWheelScrolled)
				{
					if (event.mouseWheelScroll.wheel == Mouse::VerticalWheel)
					{
						if (event.mouseWheelScroll.delta > 0) {
							screen_zoom({ event.mouseWheelScroll.x, event.mouseWheelScroll.y }, zoom_factor, zoom_center);
							zoom_val *= zoom_factor;
						}
						else {
							screen_zoom({ event.mouseWheelScroll.x, event.mouseWheelScroll.y }, 1/zoom_factor, zoom_center);
							zoom_val /= zoom_factor;
						}
					}
				}
			}
		}
		window.clear();
		renderFractal(img, sum_of_iterations, colors, time);
		texture.loadFromImage(img);
		sprite.setTexture(texture);
		window.draw(sprite);
		if (show_sys_info) {
			double avg_iterations = static_cast<double>(img_width * img_height) / sum_of_iterations;
			float time_per_frame = clock.getElapsedTime().asSeconds();
			clock.restart();
			sum_of_iterations = 0;
			char buff[100];
			snprintf(buff, sizeof(buff), 
				"Iterations: %d\n"
				"Zoom: %2.2lf\n"
				"Avg Iterations: %0.2lf\n"
				"Time per frame: %0.5lf", 
				max_iterations, zoom_val, avg_iterations, time_per_frame);
			text.setString(buff);
		}
		window.draw(text);
		window.display();
		time += 0.1;
	}
	return 0;
}

// Renders the Mandelbrot Set onto an image.
void renderFractal(Image& img, double& sum_of_iterations, vector<Color> colors, int frames_since_beginning) {
#pragma omp parallel for
	for (int y = 0; y < limits.img_height; y++) {
		for (int x = 0; x < limits.img_width; x++) {
			double x0 = limits.min_real_x + (limits.max_real_x - limits.min_real_x) * x / limits.img_width;
			double y0 = limits.min_im_y + (limits.max_imaginary_y - limits.min_im_y) * y / limits.img_height;
			double re = 0, im = 0, tmp;
			int current_iteration = 0;
			for (current_iteration; current_iteration < max_iterations; current_iteration++) {
				switch (fractal_type)
				{
				case MANDELBROT:
					tmp = re * re - im * im + x0;
					im = 2.0 * re * im + y0;
					re = tmp;
					break;
				case BURNING_SHIP:
					tmp = re * re - im * im + x0;
					im = 2.0 * std::abs(re * im) + y0;
					re = tmp;
					break;
				case TRICORN:
					tmp = re * re - im * im + x0;
					im = -2 * re * im + y0;
					re = tmp;
					break;
				case MANDELBROT_TRICORN_ANIMATION:
					tmp = re * re - im * im * cos(frames_since_beginning) + x0;
					im = 2*sin(frames_since_beginning) * re * im + y0;
					re = tmp;
					break;
				}
				if (re * re + im * im > 2.0 * 2.0) {
					sum_of_iterations += current_iteration;
					break;
				}
			}

			// Coloring
			if (current_iteration == max_iterations)
				current_iteration = 0;
			auto max_color = colors.size() - 1;
			auto color_value = (static_cast<double>(current_iteration) / max_iterations) * max_color;
			auto i_col = static_cast<unsigned int>(color_value);
			Color color1 = colors[i_col];
			Color color2 = colors[min(i_col + 1, max_color)];
			Color col = linear_interpolation(color1, color2, color_value - i_col);
			img.setPixel(x, y, Color(col));
		}
	}
}
// Function zooms into the Mandelbrot set either following the cursor or in the center.
void screen_zoom(tuple<int, int> cursor_pos, double factor, bool zoom_center)
{
	double zoom_to_x = 0;
	double zoom_to_y = 0;
	if (zoom_center) {
		zoom_to_x = limits.min_real_x + (limits.max_real_x - limits.min_real_x) * (limits.img_width / 2.0) / limits.img_width;
		zoom_to_y = limits.min_im_y + (limits.max_imaginary_y - limits.min_im_y) * (limits.img_height / 2.0) / limits.img_height;
		}
	else {
		zoom_to_x = limits.min_real_x + (limits.max_real_x - limits.min_real_x) * get<0>(cursor_pos) / limits.img_width;
		zoom_to_y = limits.min_im_y + (limits.max_imaginary_y - limits.min_im_y) * get<1>(cursor_pos) / limits.img_height;
	}

	double new_min_real = zoom_to_x - (limits.max_real_x - limits.min_real_x) / 2 / factor;
	limits.max_real_x = zoom_to_x + (limits.max_real_x - limits.min_real_x) / 2 / factor;
	limits.min_real_x = new_min_real;

	double new_min_im = zoom_to_y - (limits.max_imaginary_y - limits.min_im_y) / 2 / factor;
	limits.max_imaginary_y = zoom_to_y + (limits.max_imaginary_y - limits.min_im_y) / 2 / factor;
	limits.min_im_y = new_min_im;
}

// Interpolates two colors.
Color linear_interpolation(const Color& col1, const Color& col2, double t)
{
	auto const b = 1 - t;
	return Color(b * col1.r + t * col2.r, b * col1.g + t * col2.g, b * col1.b + t * col2.b);
}