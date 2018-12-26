#pragma once
#ifndef DESCRIPTORS_H_INCLUDED
#define DESCRIPTORS_H_INCLUDED
#include <vector>
#include <string>
#include <fstream>
#include <deque>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include "bmp.h"

class Descriptors
{
public:
	Descriptors(std::vector<std::pair<PIXEL, double>> special_points, std::vector<std::vector<PIXEL>> pixels);
	~Descriptors();
	void brief(); // Вычисление дискрипторов для каждой особой точки
	void save_descriptors(std::string file_name, std::string image_name); // Сохранение данных изображения с вычисленными дискрипторами для каждой особой точки в файле file_name
	std::vector<std::pair<std::pair<PIXEL, double>, std::vector<bool>>> descriptors; // Вектор дискрипторов для каждой особой точки
private:
	const unsigned int height = { 240 };
	const unsigned int width = { 320 };
	std::vector<std::pair<PIXEL, double>> special_points; // Вектор уже вычесленных особых точек 
	std::vector<std::vector<PIXEL>> pixels; //  Вектор данных для каждого пикселя
	int size = { 10 }; // Размер патча
	const unsigned int nd_dimensional = { 256 }; // Число сравнений в патче (размерность двоичных дескрипторов)
	const unsigned int radius = { 3 }; // Радиус окружности с центром в рассматриваемой точке (пикселе)
	std::deque<std::pair<unsigned int, unsigned int>> circle; // очередь - окружность, 1-16 пиксели
	double get_tetta(const unsigned int & x, const unsigned int & y); // Вычисление угла ориентации особой точки с координатами (x,y)
	void make_circle(const unsigned int & y, const unsigned int & x); // Заполняет очередь координатами пткселей, лежащих на окружности
	std::pair<unsigned int, unsigned int> get_blurred_pixel_intensity_values(const int & x_1, const int & y_1, const int & x_2, const int & y_2); // Получение значений интенсивностей для двух пикселей на размытом изображении
};

#endif // DESCRIPTORS_H_INCLUDEDs