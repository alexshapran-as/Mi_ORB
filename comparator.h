#pragma once
#ifndef COMPARATOR_H_INCLUDED
#define COMPARATOR_H_INCLUDED
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <regex>
#include "descriptors.h"
#include "bmp.h"

// Данные для каждого изображения
typedef struct
{
	std::string name;
	std::string descriptors_count;
	std::vector<std::string> descriptors;
	void clear()
	{
		name.clear();
		descriptors_count.clear();
		descriptors.clear();
	}
} IMG;

class Comparator
{
public:
	Comparator(std::vector<std::pair<std::pair<PIXEL, double>, std::vector<bool>>> descriptors);
	~Comparator();
	void parser(std::string file_name); // Парсер для файла, содержащего данные (путь к изображению, количество дескрипторов, дескрипторы) для каждого изображения
	void hamming_comparison(); // Сравнение дескрипторов на основе вычисления расстояния Хэмминга
private:
	std::string path_out; // Путь, куда будет сохранен файл с результатами поиска похожих изображений
	std::vector<IMG> imgs; // Результат парсинга
	std::vector<std::pair<std::pair<PIXEL, double>, std::vector<bool>>> descriptors; // Вектор дискрипторов для изображения, для которого будет производиться поиск
	const unsigned int hamming_threshold = { 105 }; // Порог для расстояния Хэмминга между дескрипторами
	std::vector<unsigned int> hamming_distances; // Расстояния Хэмминга между дескрипторами, которые удовлетворяют условию
};
#endif // COMPARATOR_H_INCLUDEDs