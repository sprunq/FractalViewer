#include <stdio.h>
#include <string>
#include <SFML/Graphics.hpp>
#include <iostream>
#include "ArialFont.h"
#include <math.h> 
using namespace std;
using namespace sf;

/*
Fractals
	- Num1: Mandelbrot
	- Num2: Tricorn
	- Num3: Mandelbrot Tricorn Animation
	- Num4: Bruning Ship

Movement:
	- W: Up
	- A: Left
	- S: Down
	- D: Right
	- Hold Middle Mouse Button: Drag

Fractal Controls:
	- R: Reset
	- F: Toggle System Info
	- I: Toggle Dynamic Iterations
	- Left Click: Increase Iterations
	- Rigth Click: Decrease Iterations
	- Arrow Left/Right: Change Animation Speed for Mandelbrot-Tricorn Animation

Colors:
	- Arrow Up/Down: Increase/Decrease Color Count 
	- Space: New Random Colors
	- Enter: Print Colors to Console

Screenshot:
	- H: Single Screenshot
	- Z: Zoom out and take Screenshots (for Animations)
*/

// Constants
const float aspect_ratio = 16.0 / 9.0;
const int win_width = 1920;
const int win_height = static_cast<int>(win_width / aspect_ratio);

// UI Settings
const float zoom_factor = 1.25;
const float move_factor = 0.05;
const float screenshot_zoom_fact = 1.0 / 1.2;
float animation_tick = 0.025;
bool show_sys_info = true;
bool zoom_into_center = true;
bool dynamic_iterations = true;

// Fractal Settings
float escape_radius = 1000;
int max_colors = 200;
const int init_iterations = 32;

// Don't touch
int max_iterations = init_iterations;
double zoom_val = 1;
double time_d = 0;

// Declarations
struct FractalSettings {
	double min_real_x;
	double max_real_x;
	double min_im_y;
	double max_im_y;
	double offset_re_x;
	double offset_im_y;
	float scale;
};

enum class FractalTypes {
	mandelbrot = 1,
	tricorn,
	mandelbrot_tricorn_animation,
	burning_ship,
	tree
} fractal_type;

// Fractal limits and offsets.
// min_re, max_re, min_im, max_im, offset_re, offset_im, scale 
FractalSettings current_frac;
FractalSettings limit_mandelbrot = { -2.5, 1.0, -1.0, 1.0 , 0.5, 0, 1.5 };
FractalSettings limit_tricorn = { -2.5, 1.0, -1.0, 1.0 , 1.5, 0, 2 };
FractalSettings limit_mandelbrot_tricorn_animation = { -2.5, 1.0, -1.0, 0.75 , 1, 0, 2 };
FractalSettings limit_burning_ship = { -2.5, 1.0, -1.0, 1.0 , 1, -0.75, 1.5 };

// Color palettes
vector<Color> gradient_ultra_fractal{
	{0,0,0},
	{0,7,100},
	{32,107,203},
	{237,255,255},
	{255,170,0},
	{0,2,0}
};

// Functions
void renderFractal(Image& img, vector<Color> colors);
void setFractal(FractalTypes frac_type);
void screenZoom(tuple<int, int> cursor_pos, double factor, bool center_zoom);
void screenshot(Texture& texture, bool is_animation);
void saveColors(vector<Color>& colors);
vector<Color> getRandomColors(int amount);
Color linear_interpolation(const Color& v, const Color& u, double a);

int main()
{
	srand(time(NULL));
	setFractal(FractalTypes::mandelbrot);

	// Colors are saved in another vector so you get the same colors if you change the amount. 
	vector<Color> colors = gradient_ultra_fractal;
	vector<Color> random_colors = colors;
	vector<Color> temp = getRandomColors(max_colors - colors.size());
	random_colors.insert(random_colors.end(), temp.begin(), temp.end());
	temp.clear();
	bool screenshot_zoom = false;
	bool dragging = false;

	Image img;
	Texture texture;
	Sprite sprite;
	Font font;
	Text text;
	Clock clock;
	Clock clock_anim;
	Event event;
	Vector2i prev_drag;

	RenderWindow window(VideoMode(win_width, win_height), "Mandelbrot Set");
	img.create(win_width, win_height);

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
					colors = getRandomColors(colors.size());
				}

				// Print Colors to console
				if (event.key.code == Keyboard::Enter) {
					saveColors(colors);
				}

				// Reset
				if (event.key.code == Keyboard::R) {
					max_iterations = init_iterations;
					zoom_val = 1;
					setFractal(fractal_type);
				}

				// Screenshot
				if (event.key.code == Keyboard::H) {
					screenshot(texture, false);
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
					max_iterations = init_iterations;
				}

				// Switch between different fractals and reset view
				if (event.key.code == Keyboard::Num1) {
					setFractal(FractalTypes::mandelbrot);
				}
				else if (event.key.code == Keyboard::Num2) {
					setFractal(FractalTypes::tricorn);
				}
				else if (event.key.code == Keyboard::Num3) {
					setFractal(FractalTypes::mandelbrot_tricorn_animation);
				}
				else if (event.key.code == Keyboard::Num4) {
					setFractal(FractalTypes::burning_ship);
				}


				// Up, Down: Amount of colors
				// Left, Right: Animation speed
				if (event.key.code == Keyboard::Up) {
					if (colors.size() < random_colors.size())
						colors.push_back(random_colors.at(colors.size()));
				}
				else if (event.key.code == Keyboard::Down) {
					if (colors.size() > 1) {
						colors.pop_back();
					}
					else {
						colors.push_back(random_colors.at(colors.size()));
					}
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
				// Dragging
				else if (event.mouseButton.button == Mouse::Middle) {
					dragging = true;
					prev_drag = { event.mouseButton.x, event.mouseButton.y };
				}
			}

			if (event.type == Event::MouseButtonReleased) {
				// Dragging
				if (event.mouseButton.button == Mouse::Middle) {
					dragging = false;
				}
			}
			if (event.type == sf::Event::MouseMoved) {
				if (dragging) {
					double drag_factor = 1 / (double(win_height)*aspect_ratio);
					Vector2i curDrag = { event.mouseMove.x, event.mouseMove.y };
					double width_step = (current_frac.max_real_x - current_frac.min_real_x) * drag_factor;
					double height_step = (current_frac.max_im_y - current_frac.min_im_y) * drag_factor;
					double re_x_movement = ((double)prev_drag.x - (double)curDrag.x) * width_step;
					double im_y_movement = ((double)prev_drag.y - (double)curDrag.y) * width_step;
					current_frac.min_real_x += re_x_movement;
					current_frac.max_real_x += re_x_movement;
					current_frac.min_im_y += im_y_movement;
					current_frac.max_im_y += im_y_movement;
					prev_drag = curDrag;
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
							screenZoom({ event.mouseWheelScroll.x, event.mouseWheelScroll.y }, zoom_factor, zoom_into_center);
						}
						else {
							screenZoom({ event.mouseWheelScroll.x, event.mouseWheelScroll.y }, 1/zoom_factor, zoom_into_center);
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
			float time_per_frame = clock.getElapsedTime().asSeconds();
			clock.restart();
			char buff[100];
			snprintf(buff, sizeof(buff), 
				"Iterations: %d\n"
				"Zoom: x%2.2lf\n"
				"Time per frame: %0.5lf", 
				max_iterations, zoom_val, time_per_frame);
			text.setString(buff);
		}
		window.draw(text);
		window.display();
		if (clock_anim.getElapsedTime().asSeconds() > animation_tick) {
			time_d += 0.05;
			clock_anim.restart();
		}
		if (screenshot_zoom) {
			screenshot(texture, true);
			screenZoom({ 0,0 }, screenshot_zoom_fact, true);
			if (zoom_val <= 0.5) {
				screenshot_zoom = false;
			}
		}
	}
	return 0;
}

// Renders the Mandelbrot Set onto an image.
void renderFractal(Image& img, vector<Color> colors) {
	if (dynamic_iterations) {
		max_iterations = 50 * pow((log10(win_width / (current_frac.max_im_y - current_frac.min_im_y))), 1.25);
	}
#pragma omp parallel for
	for (int y = 0; y < win_height; y++) {
		for (int x = 0; x < win_width; x++) {
			double x0 = current_frac.min_real_x + (current_frac.max_real_x - current_frac.min_real_x) * x / win_width;
			double y0 = current_frac.min_im_y + (current_frac.max_im_y - current_frac.min_im_y) * y / win_height;
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
				case FractalTypes::tree:
					re = 1;
					im = 1;
					break;
				}
				if (re * re + im * im > escape_radius) {
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
void screenZoom(tuple<int, int> cursor_pos, double factor, bool zoom_center)
{
	zoom_val *= factor;
	double zoom_to_x = 0;
	double zoom_to_y = 0;
	if (zoom_center) {
		zoom_to_x = current_frac.min_real_x + (current_frac.max_real_x - current_frac.min_real_x) * (win_width / 2.0) / win_width;
		zoom_to_y = current_frac.min_im_y + (current_frac.max_im_y - current_frac.min_im_y) * (win_height / 2.0) / win_height;
		}
	else {
		zoom_to_x = current_frac.min_real_x + (current_frac.max_real_x - current_frac.min_real_x) * get<0>(cursor_pos) / win_width;
		zoom_to_y = current_frac.min_im_y + (current_frac.max_im_y - current_frac.min_im_y) * get<1>(cursor_pos) / win_height;
	}

	double new_min_real = zoom_to_x - (current_frac.max_real_x - current_frac.min_real_x) / 2 / factor;
	current_frac.max_real_x = zoom_to_x + (current_frac.max_real_x - current_frac.min_real_x) / 2 / factor;
	current_frac.min_real_x = new_min_real;

	double new_min_im = zoom_to_y - (current_frac.max_im_y - current_frac.min_im_y) / 2 / factor;
	current_frac.max_im_y = zoom_to_y + (current_frac.max_im_y - current_frac.min_im_y) / 2 / factor;
	current_frac.min_im_y = new_min_im;
}

// Set a new fractal and reset the view.
void setFractal(FractalTypes new_frac_type)
{
	FractalSettings limits_frac = { 0,0,0,0,0,0 };
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
	default:
		break;
	}
	fractal_type = new_frac_type;
	max_iterations = init_iterations;
	zoom_val = 1;
	time_d = 0;
	current_frac.min_real_x = limits_frac.min_real_x * limits_frac.scale + limits_frac.offset_re_x;
	current_frac.max_real_x = limits_frac.max_real_x * limits_frac.scale + limits_frac.offset_re_x;
	current_frac.min_im_y = limits_frac.min_im_y * limits_frac.scale + limits_frac.offset_im_y;
	current_frac.max_im_y = limits_frac.max_im_y * limits_frac.scale + limits_frac.offset_im_y;
}

// Screenshots the current image. Will save the file with a prefix if it is part of a zoom.
void screenshot(Texture& texture, bool is_animation) {
	static int ss_counter = 0;
	char buffer1[128];
	char buffer2[128];
	time_t now = time(0);
	tm ltm;
	localtime_s(&ltm, &now);
	if (is_animation) {
		sprintf_s(buffer1, sizeof(buffer1), "../Pictures/%d", ss_counter++);
		strftime(buffer2, sizeof(buffer2), "_zoom_%m%d%y%H%M%S.png", &ltm);
		strcat_s(buffer1, sizeof(buffer1), buffer2);
		texture.copyToImage().saveToFile(buffer1);
	}
	else {
		strftime(buffer2, sizeof(buffer2), "../Pictures/screenshot_%m%d%y%H%M%S.png", &ltm);
		texture.copyToImage().saveToFile(buffer2);
	}
}

// Interpolates two colors.
Color linear_interpolation(const Color& col1, const Color& col2, double t)
{
	auto const b = 1 - t;
	return Color(b * col1.r + t * col2.r, b * col1.g + t * col2.g, b * col1.b + t * col2.b);
}

// Returns an array of n random colors.
vector<Color> getRandomColors(int amount) {
	vector<Color> temp;
	temp.push_back(Color(0, 0, 0));
	for (int i = 0; i < amount - 1; i++) {
		temp.push_back(Color(rand() % 255, rand() % 255, rand() % 255));
	}
	return temp;
}

// Prints the currently selected colors to the console.
void saveColors(vector<Color>& colors) {
	cout << "vector<Color> saved_grad{" << endl;
	for (auto col : colors)
		cout << "\t{" << (int)col.r << ", " << (int)col.g << ", " << (int)col.b << ", " << " " << endl;
	cout << "};" << endl;
}
