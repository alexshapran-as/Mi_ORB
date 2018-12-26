#pragma once
#ifndef SPECIALPOINTS_H_INCLUDED
#define SPECIALPOINTS_H_INCLUDED
#include <vector>
#include <deque>
#include <fstream>
#include <cmath>
#include <algorithm>
#include "bmp.h"

class SpecialPoints
{
public:
	std::vector<std::pair<PIXEL, double>> special_points; // Вектор особых точек, хранящий пары - данные пикселя, значение меры Харриса
	SpecialPoints(std::vector<std::vector<PIXEL>> pixels);
	~SpecialPoints();
	void fast(); // Поиск особых точек алгоритмом FAST
	void print_special_points(); // Вывод данных особых точек (в реализации алгоритма не используется)
	void get_processed_image(std::string file_name); // Сохранение обработанного изображения с нанесенными на него особыми точками
private:
	const unsigned int height = { 240 };
	const unsigned int width = { 320 };
	std::vector<std::vector<PIXEL>> pixels; //  Вектор данных для каждого пикселя
	const int fast_threshold = { 20 }; // Заранее заданный фиксированный порог по яркости
	const unsigned int radius = { 3 }; // Радиус окружности с центром в рассматриваемой точке (пикселе)
	const float harris_k = { 0.04f }; // Эмпирическая константа, использующаяся при вычислении меры Харриса
	unsigned int harris_threshold = { 4096 }; // Краевой порог
	const unsigned int max_count = { 2048 }; // Максимальное число особых точек
	const float two_pi = {6.28318531};
	const unsigned int sigma = {2}; // Значение сигмы в функции Гаусса (чем меньше сигма, тем боле крутой является функция Гаусса [функция Гаусса будет смотреть на небольшую окрестность изображения])
	std::deque<std::pair<unsigned int, unsigned int>> circle; // очередь - окружность, если используется для алгоритма fast, то очередь включает 1-16 пиксели, после 16 идет дополнительно 1-8 пиксели
	void make_circle(const unsigned int & y, const unsigned int & x, const unsigned int & radius, bool fast_alg); // Заполняет очередь координатами пикселей, лежащих на окружности с центром в (x,y) и радиусом radius
	void harris_selection(); // Выбор N особых точек используя меру Харриса и определения локальных максимумов
	double w(const unsigned int & x_0, const unsigned int & y_0, const unsigned int & x,const unsigned int & y); // Весовая функция - Функция Гаусса, обеспечивающая сглаживание
	bool harris_response(const int & start_x, const int & start_y, const std::vector<std::pair<PIXEL, double>>::iterator & it, std::vector<int> & indexes, double & res); // Вычисление меры Харриса для сдвинутого окна
};

#endif // SPECIALPOINTS_H_INCLUDEDs
