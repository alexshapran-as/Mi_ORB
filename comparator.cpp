#include "comparator.h"

Comparator::Comparator(std::vector<std::pair<std::pair<PIXEL, double>, std::vector<bool>>> descriptors) : descriptors(descriptors) {}

Comparator::~Comparator() {}

void Comparator::parser(std::string file_name)
{
	size_t found = file_name.find_last_of("\\");
	path_out = file_name.substr(0, found + 1);
	std::regex reg_img_name("^\"ImageName\": \"(.*)\",$");
	std::regex reg_count("^\"Count\": \"(.*)\",$");
	std::regex reg_descriptors("^\"([0-1].*)\",$");

	std::ifstream fin(file_name);
	if (!fin.is_open())
	{
		return;
	}
	std::string str;
	IMG curr_img;
	unsigned int i = { 0 };
	while (getline(fin, str))
	{
		std::match_results<std::string::const_iterator> result;
		if (std::regex_match(str, reg_img_name))
		{
			std::string::const_iterator iter = str.cbegin();
			while (std::regex_search(iter, str.cend(), result, reg_img_name))
			{
				curr_img.name += result.str(1);
				std::advance(iter, result.position(0) + result.length(0));
			}
		}
		else if (std::regex_match(str, reg_count))
		{
			std::string::const_iterator iter = str.cbegin();
			while (std::regex_search(iter, str.cend(), result, reg_count))
			{
				curr_img.descriptors_count += result.str(1);
				std::advance(iter, result.position(0) + result.length(0));
			}
		}
		else if (std::regex_match(str, reg_descriptors))
		{
			std::string temp;
			std::string::const_iterator iter = str.cbegin();
			while (std::regex_search(iter, str.cend(), result, reg_descriptors))
			{
				temp += result.str(1);
				std::advance(iter, result.position(0) + result.length(0));
			}
			curr_img.descriptors.push_back(temp);
		}
		else if (str == "}")
		{
			imgs.push_back(curr_img);
			curr_img.clear();
		}
	}
	fin.close();
}

void Comparator::hamming_comparison()
{
	for (const auto & img : imgs) // Перебор данных по каждому изображению из пропарсенного файла (по каждому изображению из папки)
	{
		for (const auto & desc1 : descriptors) // Перебор дескрипторов для входного изображения, для которого ищутся похожие изображения
		{
			unsigned int hamming_distance = { 0 }; // Промежуточное расстояние Хэмминга между двумя дескрипторами 
			unsigned int min_hamming_distance = { 256 }; // Минимальное расстояние Хэмминга среди промежуточных
			for (const auto & desc2 : img.descriptors) // Перебор дескрипторов для очередного изображения из пропарсенного файла
			{
				for (unsigned int i = { 0 }; i < 256; ++i)
				{
					if (hamming_distance > hamming_threshold) // Исключение из рассмотрения расстояния Хэмминга, которое уже больше чем порог
						break;
					hamming_distance = hamming_distance + ((desc2[i] - '0') ^ (int)desc1.second[i]);
				}
				if (hamming_distance < min_hamming_distance)
					min_hamming_distance = hamming_distance;
				hamming_distance = { 0 };
			}
			if (min_hamming_distance <= hamming_threshold)
			{
				hamming_distances.push_back(min_hamming_distance);
			}
		}
		if (((float)hamming_distances.size() / descriptors.size()) >= 0.2 && ((float)hamming_distances.size() / std::stoi(img.descriptors_count)) >= 0.2 && ((float)hamming_distances.size() / descriptors.size()) + ((float)hamming_distances.size() / std::stoi(img.descriptors_count)) >= 0.5)
		{
			std::ofstream fout(path_out + "output2.out", std::ios_base::app);
			if (!fout.is_open())
			{
				return;
			}
			fout << img.name << std::endl;
			fout.close();
		}
		hamming_distances.clear();
	}
}
