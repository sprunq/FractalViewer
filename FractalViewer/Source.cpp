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
	double min_real_x;
	double max_real_x;
	double min_im_y;
	double max_im_y;
	double offset_re_x;
	double offset_im_y;
};

enum class FractalTypes {
	mandelbrot = 1,
	tricorn,
	mandelbrot_tricorn_animation,
	burning_ship,
	todo
} fractal_type;


// Constants
const auto aspect_ratio = 16.0 / 9.0;
const int img_width = 1000;
const int img_height = static_cast<int>(img_width / aspect_ratio);

// UI Settings
const float zoom_factor = 1.5;
const float move_factor = 0.05;
float animation_tick = 0.025;
bool show_sys_info = true;
bool zoom_center = true;
bool dynamic_iterations = true;

// Fractal Settings
int max_iterations = 32;
int num_of_colors = 2;
float escape_radius = 1000;

// Init other vars. Don't touch
double zoom_val = 1;
double sum_of_iterations = 0;
double time_d = 0;
bool screenshot_zoom = false;

// Fractal limits and offsets.
Limits current_frac;
Limits limit_mandelbrot = { -2.5, 1.0, -1.0, 1.0 , 1, 0 };
Limits limit_tricorn = { -2.5, 1.0, -1.0, 1.0 , 1, 0 };
Limits limit_mandelbrot_tricorn_animation = { -2.5, 1.0, -1.0, 1.0 , 1, 0 };
Limits limit_burning_ship = { -2.5, 1.0, -1.0, 1.0 , 0, 0 };
Limits limit_todo = { -2.5, 1.0, -1.0, 1.0 , 1, 0 };

void renderFractal(Image& img, vector<Color> colors);
void set_fractal(FractalTypes frac_type, int iterations, double scale);
void screen_zoom(tuple<int, int> cursor_pos, double factor, bool zoom_center);
void screenshot(Texture& texture);
void saveColors(vector<Color>& colors);
vector<Color> getRandomColors(int amount);
Color linear_interpolation(const Color& v, const Color& u, double a);

int main()
{
	srand(time(NULL));
	set_fractal(FractalTypes::mandelbrot, 32, 1.5);
	vector<Color> colors = gradient0;

	Image img;
	Texture texture;
	Sprite sprite;
	Font font;
	Text text;
	Clock clock;
	Clock clock_anim;
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
				double width_step = (current_frac.max_real_x - current_frac.min_real_x) * move_factor;
				double height_step = (current_frac.max_im_y - current_frac.min_im_y) * move_factor;
				if (event.key.code == Keyboard::A) {
					current_frac.min_real_x -= width_step;
					current_frac.max_real_x -= width_step;
				}
				else if (event.key.code == Keyboard::D) {
					current_frac.min_real_x += width_step;
					current_frac.max_real_x += width_step;
				}
				else if (event.key.code == Keyboard::W) {
					current_frac.min_im_y -= height_step;
					current_frac.max_im_y -= height_step;
				}
				else if (event.key.code == Keyboard::S) {
					current_frac.min_im_y += height_step;
					current_frac.max_im_y += height_step;
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
					set_fractal(fractal_type, 32, 1.5);
				}

				// Screenshot
				if (event.key.code == Keyboard::H) {
					time_t now = time(0);
					tm ltm;
					localtime_s(&ltm, &now);
					char buffer[128];
					strftime(buffer, sizeof(buffer), "pic_%m%d%y%H%M%S.png", &ltm);
					texture.copyToImage().saveToFile(buffer);
				}

				// Screenshot session
				if (event.key.code == Keyboard::Z) {
					screenshot_zoom = true;
				}

				// Toggle System Info
				if (event.key.code == Keyboard::F) {
					show_sys_info = !show_sys_info;
					text.setString("");
				}

				// Toggle Dynamic Iterations
				if (event.key.code == Keyboard::I) {
					dynamic_iterations = !dynamic_iterations;
					max_iterations = 32;
				}

				// Switch between different fractals and reset view
				if (event.key.code == Keyboard::Num1) {
					set_fractal(FractalTypes::mandelbrot, 32, 1.5);
				}
				else if (event.key.code == Keyboard::Num2) {
					set_fractal(FractalTypes::tricorn, 32, 2);
				}
				else if (event.key.code == Keyboard::Num3) {
					set_fractal(FractalTypes::mandelbrot_tricorn_animation, 32, 2);
				}
				else if (event.key.code == Keyboard::Num4) {
					set_fractal(FractalTypes::burning_ship, 32, 2);
				}
				else if (event.key.code == Keyboard::Num5) {
					set_fractal(FractalTypes::todo, 32, 2);
				}

				// Up, Down: Amount of colors
				// Left, Right: Animation speed
				if (event.key.code == Keyboard::Up) {
					colors = getRandomColors(++num_of_colors);;
				}
				else if (event.key.code == Keyboard::Down) {
					colors = getRandomColors(num_of_colors--);
					if (num_of_colors <= 2)
						num_of_colors = 2;
				}
				else if (event.key.code == Keyboard::Left) {
					animation_tick *= 2;
				}
				else if (event.key.code == Keyboard::Right) {
					animation_tick *= 0.5;
					if (animation_tick <= 0)
						animation_tick = 0.1;
				}
			}

			if (event.type == Event::MouseButtonPressed) {
				// Iterations
				if (event.mouseButton.button == Mouse::Left) {
					max_iterations *= 2;

				}
				else if (event.mouseButton.button == Mouse::Right) {
					max_iterations *= 0.5;
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
						}
						else {
							screen_zoom({ event.mouseWheelScroll.x, event.mouseWheelScroll.y }, 1/zoom_factor, zoom_center);
						}
					}
				}
			}
		}
		window.clear();
		renderFractal(img, colors);
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
		if (clock_anim.getElapsedTime().asSeconds() > animation_tick) {
			time_d += 0.05;
			clock_anim.restart();
		}
		if (screenshot_zoom) {
			screenshot(texture);
			screen_zoom({ 0,0 }, 1 / 1.3, true);
			if (zoom_val <= 0.5) {
				screenshot_zoom = false;
			}
		}
	}
	return 0;
}

// Renders the Mandelbrot Set onto an image.
void renderFractal(Image& img, vector<Color> colors) {
	if (dynamic_iterations)
		max_iterations = 50 * pow((log10(img_width / (current_frac.max_im_y - current_frac.min_im_y))), 1.25);
#pragma omp parallel for
	for (int y = 0; y < img_height; y++) {
		for (int x = 0; x < img_width; x++) {
			double x0 = current_frac.min_real_x + (current_frac.max_real_x - current_frac.min_real_x) * x / img_width;
			double y0 = current_frac.min_im_y + (current_frac.max_im_y - current_frac.min_im_y) * y / img_height;
			double re = 0, im = 0, tmp;
			int current_iteration = 0;
			for (current_iteration; current_iteration < max_iterations; current_iteration++) {
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
					im = 2.0 * sin(time_d) * re * im + y0;
					re = tmp;
					break;
				case FractalTypes::burning_ship:
					tmp = re * re - im * im + x0;
					im = 2.0 * std::abs(re * im) + y0;
					re = tmp;
					break;
				case FractalTypes::todo:
					re = 1;
					im = 1;
					break;
				}
				if (re * re + im * im > escape_radius) {
					sum_of_iterations += current_iteration;
					break;
				}
			}

			// Coloring
			if (current_iteration == max_iterations)
				current_iteration = 0;
			unsigned int max_color = colors.size() - 1;
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
	zoom_val *= factor;
	double zoom_to_x = 0;
	double zoom_to_y = 0;
	if (zoom_center) {
		zoom_to_x = current_frac.min_real_x + (current_frac.max_real_x - current_frac.min_real_x) * (img_width / 2.0) / img_width;
		zoom_to_y = current_frac.min_im_y + (current_frac.max_im_y - current_frac.min_im_y) * (img_height / 2.0) / img_height;
		}
	else {
		zoom_to_x = current_frac.min_real_x + (current_frac.max_real_x - current_frac.min_real_x) * get<0>(cursor_pos) / img_width;
		zoom_to_y = current_frac.min_im_y + (current_frac.max_im_y - current_frac.min_im_y) * get<1>(cursor_pos) / img_height;
	}

	double new_min_real = zoom_to_x - (current_frac.max_real_x - current_frac.min_real_x) / 2 / factor;
	current_frac.max_real_x = zoom_to_x + (current_frac.max_real_x - current_frac.min_real_x) / 2 / factor;
	current_frac.min_real_x = new_min_real;

	double new_min_im = zoom_to_y - (current_frac.max_im_y - current_frac.min_im_y) / 2 / factor;
	current_frac.max_im_y = zoom_to_y + (current_frac.max_im_y - current_frac.min_im_y) / 2 / factor;
	current_frac.min_im_y = new_min_im;
}

// Set a new fractal and reset the view.
void set_fractal(FractalTypes new_frac_type, int iterations, double scale)
{
	Limits limits_frac = { 0,0,0,0,0,0 };
	switch (new_frac_type)
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
	case FractalTypes::todo:
		limits_frac = limit_todo;
		break;
	default:
		break;
	}
	fractal_type = new_frac_type;
	max_iterations = iterations;
	zoom_val = 1;
	time_d = 0;
	current_frac.min_real_x = limits_frac.min_real_x * scale + limits_frac.offset_re_x;
	current_frac.max_real_x = limits_frac.max_real_x * scale + limits_frac.offset_re_x;
	current_frac.min_im_y = limits_frac.min_im_y * scale + limits_frac.offset_im_y;
	current_frac.max_im_y = limits_frac.max_im_y * scale + limits_frac.offset_im_y;
}

// Screenshots the current image. Will save the file with a prefix if it is part of a zoom.
void screenshot(Texture& texture) {
	static int ss_counter = 0;
	char buffer1[128];
	char buffer2[128];
	time_t now = time(0);
	tm ltm;
	localtime_s(&ltm, &now);
	if (screenshot_zoom) {
		sprintf_s(buffer1, sizeof(buffer1), "%d", ss_counter++);
		strftime(buffer2, sizeof(buffer2), "_zoom_%m%d%y%H%M%S.png", &ltm);
		strcat_s(buffer1, sizeof(buffer1), buffer2);
		texture.copyToImage().saveToFile(buffer1);
	}
	else {
		strftime(buffer2, sizeof(buffer2), "screenshot_%m%d%y%H%M%S.png", &ltm);
		texture.copyToImage().saveToFile(buffer2);
	}
}

// Interpolates two colors.
Color linear_interpolation(const Color& col1, const Color& col2, double t)
{
	auto const b = 1 - t;
	return Color(b * col1.r + t * col2.r, b * col1.g + t * col2.g, b * col1.b + t * col2.b);
}

vector<Color> getRandomColors(int amount) {
	vector<Color> temp;
	temp.push_back(Color(0, 0, 0));
	for (int i = 0; i < amount - 1; i++) {
		temp.push_back(Color(rand() % 255, rand() % 255, rand() % 255));
	}
	return temp;
}

void saveColors(vector<Color>& colors) {
	cout << "vector<Color> saved_grad{" << endl;
	for (auto col : colors)
		cout << "\t{" << (int)col.r << ", " << (int)col.g << ", " << (int)col.b << ", " << " " << endl;
	cout << "};" << endl;
}