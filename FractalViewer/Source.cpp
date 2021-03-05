#include <SFML/Graphics.hpp>
#include <cstdio>
#include <string>
#include <iostream>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <climits>
#include "ArialFont.h"
#include "Fractal.h"
using namespace std;
using namespace sf;

/*
Fractals
	- Num1: Mandelbrot
	- Num2: Tricorn
	- Num3: Mandelbrot Tricorn Animation
	- Num4: Burning Ship

Movement:
	- W: Up
	- A: Left
	- S: Down
	- D: Right
	- Hold Middle Mouse Button: Drag

Fractal Controls:
	- F: Toggle System Info
	- I: Toggle Dynamic Iterations
	- Left Click: Increase Iterations
	- Right Click: Decrease Iterations
	- Arrow Left/Right: Change Animation Speed for Mandelbrot-Tricorn Animation

Colors:
	- Arrow Up/Down: Increase/Decrease Color Count 
	- Space: New Random Colors
	- Enter: Print Colors to Console

Screenshot:
	- H: Single Screenshot
	- Z: Zoom out and take Screenshots (for Animations)
*/

struct WindowSettings{
    float aspect_ratio;
    int width;
    int height;
};

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
void screenZoom(WindowSettings windowSettings, Fractal* fractal, tuple<int, int> cursorPos, double factor, bool zoomCenter);
void screenshot(Texture& texture, bool isAnimation);
void saveColors(vector<Color>& colors);
vector<Color> getRandomColors(int amount);

int main(int argc, char *argv[])
{
    float aspect_ratio = 0;
    int win_width = 0;
    int win_height = 0;
    if (argc == 2) {
        errno = 0;
        char *p;
        long conv = strtol(argv[1], &p, 10);
        if (errno != 0 || *p != '\0' || conv > INT_MAX || conv < INT_MIN) {
            cout << "Input Error" << endl;
            exit(EXIT_FAILURE);
        }
        else{
            aspect_ratio = 16.0 / 9.0;
            win_width = conv;
            win_height = static_cast<int>(win_width / aspect_ratio);
        }
    }
    else{
        aspect_ratio = 16.0 / 9.0;
        win_width = 640;
        win_height = static_cast<int>(win_width / aspect_ratio);
    }

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
    int max_colors = 2000;
    vector<Color> colors = gradient_ultra_fractal;

    // Don't touch
    double zoom_val = 1;
    double time_d = 0;
    bool screenshot_zoom = false;
    bool dragging = false;
    srand(time(nullptr));
    WindowSettings window_size = {aspect_ratio, win_width, win_height};
    Image img;
    Texture texture;
    Sprite sprite;
    Font font;
    Text text;
    Clock clock;
    Clock clock_anim;
    Event event{};
    Vector2i prev_drag;

	// Colors are saved in another vector so you get the same colors if you change the amount.
	vector<Color> extra_random_colors = colors;
	vector<Color> temp = getRandomColors(max_colors - colors.size());
	extra_random_colors.insert(extra_random_colors.end(), temp.begin(), temp.end());
	temp.clear();

	// Create Window, Font and Fractal
	RenderWindow window(VideoMode(window_size.width, window_size.height), "Fractal Viewer");
	img.create(window_size.width, window_size.height);
    auto fractal = new Fractal(img, dynamic_iterations, escape_radius);

	if (!font.loadFromMemory(&arial_ttf, arial_ttf_len))
	{
		cout << "Loading Font Failed!" << endl;
		return 0;
	}
	text.setFont(font);
	text.setCharacterSize(18);
	text.setFillColor(Color::White);

	// Game Loop
	while (window.isOpen()) {
		while (window.pollEvent(event)) {
			if (event.type == Event::Closed) {
				window.close();
			}

			if (event.type == Event::KeyPressed) {
				// Moving around with WASD
				bool changed_frac_settings = false;
				auto p_fractal = fractal->getFracSettings();
				double width_step = (p_fractal.max_real_x - p_fractal.min_real_x) * move_factor;
				double height_step = (p_fractal.max_im_y - p_fractal.min_im_y) * move_factor;
                switch (event.key.code) {
				    case Keyboard::W:
				        // Move Up
                        p_fractal.min_im_y -= height_step;
                        p_fractal.max_im_y -= height_step;
                        changed_frac_settings = true;
				        break;
                    case Keyboard::A:
                        // Move Left
                        p_fractal.min_real_x -= width_step;
                        p_fractal.max_real_x -= width_step;
                        changed_frac_settings = true;
                        break;
                    case Keyboard::S:
                        // Move Down
                        p_fractal.min_im_y += height_step;
                        p_fractal.max_im_y += height_step;
                        changed_frac_settings = true;
                        break;
                    case Keyboard::D:
                        // Move Right
                        p_fractal.min_real_x += width_step;
                        p_fractal.max_real_x += width_step;
                        changed_frac_settings = true;
                        break;
                    case Keyboard::Num1:
                        // Change to Mandelbrot
                        fractal->setFractalType(FractalTypes::mandelbrot);
                        zoom_val = 1;
                        break;
                    case Keyboard::Num2:
                        // Change to Tricorn
                        fractal->setFractalType(FractalTypes::tricorn);
                        zoom_val = 1;
                        break;
                    case Keyboard::Num3:
                        // Change to Animation
                        fractal->setFractalType(FractalTypes::mandelbrot_tricorn_animation);
                        zoom_val = 1;
                        break;
                    case Keyboard::Num4:
                        // Change to Burning Ship
                        fractal->setFractalType(FractalTypes::burning_ship);
                        zoom_val = 1;
                        break;
                    case Keyboard::Up:
                        // Increase Color Count
                        if (colors.size() < extra_random_colors.size())
                            colors.push_back(extra_random_colors.at(colors.size()));
                        break;
                    case Keyboard::Down:
                        // Decrease Color Count
                        if (colors.size() > 1) {
                            colors.pop_back();
                        }
                        else {
                            colors.push_back(extra_random_colors.at(colors.size()));
                        }
                        break;
                    case Keyboard::Left:
                        // Slow Down Animation
                        animation_tick *= 2;
                        break;
                    case Keyboard::Right:
                        // Speed Up Animation
                        animation_tick *= 0.5;
                        if (animation_tick <= 0)
                            animation_tick = 0.1;
                        break;
                    case Keyboard::Space:
                        // Create New Random Colors
                        colors = getRandomColors(colors.size());
                        break;
                    case Keyboard::Enter:
                        // Print Current Colors
                        saveColors(colors);
                        break;
                    case Keyboard::H:
                        // Screenshot Screen
                        screenshot(texture, false);
                        break;
                    case Keyboard::Z:
                        // Screenshot Animation While Zooming Out
                        screenshot_zoom = true;
                        break;
                    case Keyboard::F:
                        // Toggle Information Display
                        show_sys_info = !show_sys_info;
                        text.setString("");
                        break;
                    case Keyboard::I:
                        // Toggle Iteration Mode (Dynamic - Manual)
                        fractal->toggleIterationMode();
                        break;
                    default:
                        break;
                }
                if (changed_frac_settings)
                    fractal->setFracSettings(p_fractal);
			}

			if (event.type == Event::MouseButtonPressed) {
				// Manually Change Iteration Level
                int iterations = fractal->getIterations();
                switch (event.mouseButton.button) {
				    case Mouse::Left:
				        // Increase Iterations
                        fractal->setIterations(iterations*2);
				        break;
                    case Mouse::Right:
                        // Decrease Iterations
                        fractal->setIterations(iterations*0.5);
                        if (iterations < 1)
                            fractal->setIterations(1);
                        break;
                    case Mouse::Middle:
                        // Begin Dragging
                        dragging = true;
                        prev_drag = { event.mouseButton.x, event.mouseButton.y };
                        break;
                    default:
                        break;
                }
			}

			if (event.type == Event::MouseButtonReleased) {
				// End Dragging
				if (event.mouseButton.button == Mouse::Middle) {
					dragging = false;
				}
			}
			if (event.type == sf::Event::MouseMoved) {
			    // Dragging Action
				if (dragging) {
                    auto p_fractal = fractal->getFracSettings();
					double drag_factor = 1 / (double(win_height)*aspect_ratio);
					Vector2i curDrag = { event.mouseMove.x, event.mouseMove.y };
					double step = (p_fractal.max_real_x - p_fractal.min_real_x) * drag_factor;
					double re_x_movement = ((double)prev_drag.x - (double)curDrag.x) * step;
					double im_y_movement = ((double)prev_drag.y - (double)curDrag.y) * step;
                    p_fractal.min_real_x += re_x_movement;
                    p_fractal.max_real_x += re_x_movement;
                    p_fractal.min_im_y += im_y_movement;
                    p_fractal.max_im_y += im_y_movement;
                    fractal->setFracSettings(p_fractal);
					prev_drag = curDrag;
				}
			}

			if (event.type == Event::MouseWheelScrolled)
			{
				// Zoom
				if (event.mouseWheelScroll.wheel == Mouse::VerticalWheel) {
				    if (event.mouseWheelScroll.delta > 0) {
				        screenZoom(window_size, fractal, {event.mouseWheelScroll.x, event.mouseWheelScroll.y}, zoom_factor,
                                   zoom_into_center);
				        zoom_val *= zoom_factor;
				    } else {
				        screenZoom(window_size, fractal, {event.mouseWheelScroll.x, event.mouseWheelScroll.y}, 1 / zoom_factor,
                                   zoom_into_center);
				        zoom_val /= zoom_factor;
				    }
				}
			}
		}

		window.clear();
		fractal->renderFractal(colors, window_size.width, window_size.height, time_d);
		texture.loadFromImage(img);
		sprite.setTexture(texture);
		window.draw(sprite);
		int frac_type = fractal->getFractalType();
		if (show_sys_info) {
			float time_per_frame = clock.getElapsedTime().asSeconds();
			clock.restart();
			char buff[100];
			snprintf(buff, sizeof(buff),
            "Fractal: %s\n"
				"Iterations: %d\n"
				"Zoom: x%2.2lf\n"
				"Time per frame: %0.5lf\n",
				fractal->getName(),
				fractal->getIterations(), zoom_val,
				time_per_frame);
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
			screenZoom(window_size, fractal, {event.mouseWheelScroll.x, event.mouseWheelScroll.y}, screenshot_zoom_fact,
                       true);
            zoom_val *= screenshot_zoom_fact;
			if (zoom_val <= 1.0) {
				screenshot_zoom = false;
			}
		}
	}
	return 0;
}

// Function zooms into the Mandelbrot set either following the cursor or in the center.
void screenZoom(WindowSettings windowSettings, Fractal* fractal, tuple<int, int> cursorPos, double factor, bool zoomCenter)
{
    auto p_fractal = fractal->getFracSettings();
	double zoom_to_x = 0;
	double zoom_to_y = 0;
	if (zoomCenter) {
		zoom_to_x = p_fractal.min_real_x + (p_fractal.max_real_x - p_fractal.min_real_x) * (windowSettings.width / 2.0) / windowSettings.width;
		zoom_to_y = p_fractal.min_im_y + (p_fractal.max_im_y - p_fractal.min_im_y) * (windowSettings.height / 2.0) / windowSettings.height;
		}
	else {
		zoom_to_x = p_fractal.min_real_x + (p_fractal.max_real_x - p_fractal.min_real_x) * get<0>(cursorPos) / windowSettings.width;
		zoom_to_y = p_fractal.min_im_y + (p_fractal.max_im_y - p_fractal.min_im_y) * get<1>(cursorPos) / windowSettings.height;
	}

	double new_min_real = zoom_to_x - (p_fractal.max_real_x - p_fractal.min_real_x) / 2 / factor;
    p_fractal.max_real_x = zoom_to_x + (p_fractal.max_real_x - p_fractal.min_real_x) / 2 / factor;
    p_fractal.min_real_x = new_min_real;

	double new_min_im = zoom_to_y - (p_fractal.max_im_y - p_fractal.min_im_y) / 2 / factor;
    p_fractal.max_im_y = zoom_to_y + (p_fractal.max_im_y - p_fractal.min_im_y) / 2 / factor;
    p_fractal.min_im_y = new_min_im;

    fractal->setFracSettings(p_fractal);
}


// Screenshots the current image. Will save the file with a prefix if it is part of a zoom.
void screenshot(Texture& texture, bool isAnimation) {
	static int ss_counter = 0;
	char buffer1[128];
	char buffer2[128];
	time_t rawtime;
	struct tm* timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	if (isAnimation) {
		sprintf(buffer1, "../Images/Animations/%d", ss_counter++);
        strftime(buffer2, sizeof(buffer2), "_zoom_%m%d%y%H%M%S.png", timeinfo);
		strcat(buffer1, buffer2);
		texture.copyToImage().saveToFile(buffer1);
	}
	else {
		strftime(buffer2, sizeof(buffer2), "../Images/Screenshots/ss_%m%d%y%H%M%S.png", timeinfo);
		texture.copyToImage().saveToFile(buffer2);
	}
}

// Returns an array of n random colors.
vector<Color> getRandomColors(int amount) {
	vector<Color> temp;
	temp.emplace_back(0, 0, 0);
	for (int i = 0; i < amount - 1; i++) {
		temp.emplace_back(rand() % 255, rand() % 255, rand() % 255);
	}
	return temp;
}

// Prints the currently selected colors to the console.
void saveColors(vector<Color>& colors) {
	cout << "vector<Color> saved_grad{" << endl;
	for (auto col : colors)
		cout << "\t{" << (int)col.r << ", " << (int)col.g << ", " << (int)col.b << " } " << endl;
	cout << "};" << endl;
}
