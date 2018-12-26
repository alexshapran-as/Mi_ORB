#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "bmp.h"
#include "specialpoints.h"
#include "descriptors.h"
#include "comparator.h"
namespace fs = std::filesystem;

void preliminary_alg_construct_descriptors(const std::vector<std::string> & commands); // Режим алгоритма для построения дескрипторов
void alg_for_finding_similar_images(const std::vector<std::string> & commands); // Режим алгоритма для поиска похожих изображений
void algorithm_test(const std::string & file_name_1, const std::string & file_name_2); // Режим тестирования


int main(int argc, char *argv[])
{	
	if (argc < 2)	
	{			
		return 0;
	}
	if (argc == 2)
	{
		std::ifstream fin(argv[1]);
		if (!fin.is_open())
		{
			return 0;
		}
		std::vector<std::string> commands;
		std::string command;
		while (getline(fin, command))
		{
			commands.push_back(command);
		}
		fin.close();
		if (commands.size() == 2) // Режим построения дескрипторов
		{
			preliminary_alg_construct_descriptors(commands);
		}
		else if (commands.size() == 3) // Режим поиска похожего изображения в папке
		{ 
			alg_for_finding_similar_images(commands);
		}
	}
	else if (argc == 3) // Режим тестирования
	{
		std::string file_name_1 = argv[1];
		std::string file_name_2 = argv[2];
		algorithm_test(file_name_1, file_name_2);
	}
	return 0;
}

void preliminary_alg_construct_descriptors(const std::vector<std::string> & commands)
{
	std::string path = commands[0]; // Путь к фотографиям для обработки
	std::string proc_path = commands[1]; // Путь к директории, в которой будут сохраняться обработанные изображения
	for (const auto & p : fs::directory_iterator(path))
	{
		std::string img_path = p.path().string(); // Путь к текущему изобажению из директории
		BMP bmp_file(img_path);
		bmp_file.image_read(img_path);
		SpecialPoints special_points(bmp_file.pixels);
		special_points.fast();
		special_points.get_processed_image(proc_path + "\\processed_img_" + p.path().filename().string());
		Descriptors desc(special_points.special_points, bmp_file.pixels);
		desc.brief();
		std::size_t found = img_path.find_last_of("\\");
		std::string path_out = img_path.substr(0, found);
		found = path_out.find_last_of("\\");
		path_out = path_out.substr(0, found + 1);
		desc.save_descriptors(path_out + "output1.out", img_path);
	}
}

void alg_for_finding_similar_images(const std::vector<std::string> & commands)
{
	std::string img_path = commands[0]; // Путь к изображению, для которого выполняем поиск
	std::string file_name = commands[1]; // Путь к файлу с дескрипторами
	std::string proc_path = commands[2]; // Путь к директории, в которой будут сохраняться обработанные изображения
	std::size_t found = img_path.find_last_of("\\");
	std::string img_name = img_path.substr(found + 1);
	BMP bmp_file(img_path);
	bmp_file.image_read(img_path);
	SpecialPoints special_points(bmp_file.pixels);
	special_points.fast();
	special_points.get_processed_image(proc_path + "\\processed_img_for_search_" + img_name);
	Descriptors desc(special_points.special_points, bmp_file.pixels);
	desc.brief();
	Comparator cmp(desc.descriptors);
	cmp.parser(file_name);
	cmp.hamming_comparison();
}

void algorithm_test(const std::string & file_name_1, const std::string & file_name_2)
{
	std::ifstream fin(file_name_1);
	if (!fin.is_open())
	{
		return;
	}
	std::vector<std::string> commands;
	std::string command;
	while (getline(fin, command))
	{
		commands.push_back(command);
	}
	fin.close();
	preliminary_alg_construct_descriptors(commands);
	
	fin.open(file_name_2);
	if (!fin.is_open())
	{
		return;
	}
	commands.clear();
	while (getline(fin, command))
	{
		commands.push_back(command);
	}
	fin.close();
	alg_for_finding_similar_images(commands);
	
	std::string img_path = commands[0]; // Путь к изображению, для которого выполняем поиск
	std::size_t found = img_path.find_last_of("\\");
	std::string path_out = img_path.substr(0, found + 1);
	std::vector<std::string> found_imgs;
	std::string temp;
	fin.open(path_out + "output2.out");
	if (!fin.is_open())
	{
		return;
	}
	while (getline(fin, temp))
	{
		found_imgs.push_back(temp);
	}
	fin.close();
	std::ofstream fout(path_out + "result.r");
	if (!fout.is_open())
	{
		return;
	}
	if (found_imgs.size() > 1 || found_imgs.size() == 0)
	{
		fout << "FALSE";
	}
	else if (found_imgs.size() == 1)
	{
		std::size_t found = found_imgs[0].find_last_of("\\");
		std::string result = found_imgs[0].substr(found + 1);
		found = result.find_last_of(".");
		result = result.substr(0, found);
		found = img_path.find_last_of("\\");
		std::string img_name = img_path.substr(found + 1);
		found = img_name.find_last_of(".");
		img_name = img_name.substr(0, found);
		if ("original_image_" + result == img_name)
		{
			fout << "TRUE";
		}
		else
		{
			fout << "FALSE";
		}
	}
	fout.close();
}
