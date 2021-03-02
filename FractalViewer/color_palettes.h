#pragma once
#include <stdio.h>
#include <cstdlib>
#include <SFML/Graphics.hpp>
using namespace std;
using namespace sf;

vector<Color> getRandomColors(int amount) {
	vector<Color> temp;
	temp.push_back(Color(0,0,0));
	for (int i = 0; i < amount-1; i++) {
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

vector<Color> simple_gradient{
			{0,7,100},
			{32,107,203},
			{237,255,255},
			{255,170,0},
			{0,2,0}
};

vector<Color> grad2{
	{0,0,0},
	{255,0,0},
	{0,255,0},
	{0,0,255},
	{255,255,255}
};

