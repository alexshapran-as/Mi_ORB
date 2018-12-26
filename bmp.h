#pragma once
#ifndef BMP_H_INCLUDED
#define BMP_H_INCLUDED
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
// https://ru.bmstu.wiki/BMP_(Bitmap_Picture)
// https://ziggi.org/chtenie-bmp-v-c-versiya-2/
// CIEXYZTRIPLE значения
typedef int FXPT2DOT30;

typedef struct 
{
	FXPT2DOT30 ciexyzX;
	FXPT2DOT30 ciexyzY;
	FXPT2DOT30 ciexyzZ;
} CIEXYZ;

typedef struct 
{
	CIEXYZ  ciexyzRed;
	CIEXYZ  ciexyzGreen;
	CIEXYZ  ciexyzBlue;
} CIEXYZTRIPLE;

// Заголовок файла растрового изображения
typedef struct 
{
	unsigned short bfType;
	unsigned int   bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int   bfOffBits;
} BITMAPFILEHEADER;

// Информационный заголовок
typedef struct 
{
	unsigned int   biSize;
	unsigned int   biWidth;
	unsigned int   biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int   biCompression;
	unsigned int   biSizeImage;
	unsigned int   biXPelsPerMeter;
	unsigned int   biYPelsPerMeter;
	unsigned int   biClrUsed;
	unsigned int   biClrImportant;
	unsigned int   biRedMask;
	unsigned int   biGreenMask;
	unsigned int   biBlueMask;
	unsigned int   biAlphaMask;
	unsigned int   biCSType;
	CIEXYZTRIPLE   biEndpoints;
	unsigned int   biGammaRed;
	unsigned int   biGammaGreen;
	unsigned int   biGammaBlue;
	unsigned int   biIntent;
	unsigned int   biProfileData;
	unsigned int   biProfileSize;
	unsigned int   biReserved;
} BITMAPINFOHEADER;

// Данные изображения RGB, значения яркости (интенсивности) для пикселя, его координаты и маркер, показывающий является ли пиксель особой точкой
typedef struct 
{
	unsigned char  rgbBlue;
	unsigned char  rgbGreen;
	unsigned char  rgbRed;
	unsigned char  rgbReserved;
	unsigned int  intensity;
	unsigned int x;
	unsigned int y;
	bool special;
} PIXEL;

// Класс изображение формата bmp с поддержкой 5 версий bmp и с 16-, 24-, 32- битами на пиксель
class BMP
{
public:
	BITMAPFILEHEADER file_header;
	BITMAPINFOHEADER file_info_header;
	std::vector<std::vector<PIXEL>> pixels; //  Вектор данных для каждого пикселя
	BMP() {}
	BMP(std::string file_name);
	~BMP();
	void get_file_header(); //  Заполнение структуры BITMAPFILEHEADER
	void get_file_info_header(); // Заполнение структуры BITMAPINFOHEADER
	void get_pixels_info(); // Получение данных RGB, интенсивности ... каждого пикселя
	void print_pixels_info(); // Печать значений для каждого пикселя изображения (не используется в реализации алгоритма) 
	int image_read(std::string img_path); // Функция получения данных изображения
private:
	const unsigned int height = { 240 };
	const unsigned int width = { 320 };
	std::ifstream file_stream;
	template <typename Type>
	void read(std::ifstream & fp, Type & result, std::size_t size) // Функция чтения блока байтов
	{
		fp.read(reinterpret_cast<char *>(&result), size);
	}
	unsigned char bit_extract(const unsigned int byte, const unsigned int mask); // Функция извлечения байтов
	void image_resize(std::string img_path); // Функция изменения размеров изображения
	void image_convert_to_bmp(std::string & img_path); // Функция конвертации изображения в bmp формат
};
#endif // BMP_H_INCLUDED