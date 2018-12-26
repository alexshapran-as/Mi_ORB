#include "bmp.h"

BMP::BMP(std::string file_name)
{
	// Открытие файла-изображения
	file_stream.open(file_name, std::ifstream::binary);
	if (!file_stream)
	{
		std::exit(EXIT_FAILURE);
	}
}

BMP::~BMP()
{
	file_stream.close();
}

unsigned char BMP::bit_extract(const unsigned int byte, const unsigned int mask)
{
	if (mask == 0) 
	{
		return 0;
	}

	// Определение количества нулевых бит справа от маски
	int mask_buffer = mask;
	int	mask_padding = 0;

	while (!(mask_buffer & 1)) 
	{
		mask_buffer >>= 1;
		mask_padding++;
	}

	// Применение маски и смещение
	return (byte & mask) >> mask_padding;
}

void BMP::get_file_header()
{
	read(file_stream, file_header.bfType, sizeof(file_header.bfType));
	read(file_stream, file_header.bfSize, sizeof(file_header.bfSize));
	read(file_stream, file_header.bfReserved1, sizeof(file_header.bfReserved1));
	read(file_stream, file_header.bfReserved2, sizeof(file_header.bfReserved2));
	read(file_stream, file_header.bfOffBits, sizeof(file_header.bfOffBits));

	// Проверка на наличие заголовка BM
	if (file_header.bfType != 0x4D42)
	{
		throw false; // Не BMP файл
	}
}

void BMP::get_file_info_header()
{
	read(file_stream, file_info_header.biSize, sizeof(file_info_header.biSize));
	// Информация о файле
	if (file_info_header.biSize >= 12)
	{
		read(file_stream, file_info_header.biWidth, sizeof(file_info_header.biWidth));
		read(file_stream, file_info_header.biHeight, sizeof(file_info_header.biHeight));
		read(file_stream, file_info_header.biPlanes, sizeof(file_info_header.biPlanes));
		read(file_stream, file_info_header.biBitCount, sizeof(file_info_header.biBitCount));
	}

	// Получение информации о битности
	int colors_count = file_info_header.biBitCount >> 3;
	if (colors_count < 3)
	{
		colors_count = 3;
	}

	int bits_on_color = file_info_header.biBitCount / colors_count;
	int mask_value = (1 << bits_on_color) - 1;

	// bmp v1
	if (file_info_header.biSize >= 40)
	{
		read(file_stream, file_info_header.biCompression, sizeof(file_info_header.biCompression));
		read(file_stream, file_info_header.biSizeImage, sizeof(file_info_header.biSizeImage));
		read(file_stream, file_info_header.biXPelsPerMeter, sizeof(file_info_header.biXPelsPerMeter));
		read(file_stream, file_info_header.biYPelsPerMeter, sizeof(file_info_header.biYPelsPerMeter));
		read(file_stream, file_info_header.biClrUsed, sizeof(file_info_header.biClrUsed));
		read(file_stream, file_info_header.biClrImportant, sizeof(file_info_header.biClrImportant));
	}

	// bmp v2
	file_info_header.biRedMask = 0;
	file_info_header.biGreenMask = 0;
	file_info_header.biBlueMask = 0;

	if (file_info_header.biSize >= 52)
	{
		read(file_stream, file_info_header.biRedMask, sizeof(file_info_header.biRedMask));
		read(file_stream, file_info_header.biGreenMask, sizeof(file_info_header.biGreenMask));
		read(file_stream, file_info_header.biBlueMask, sizeof(file_info_header.biBlueMask));
	}

	// Проверка на наличие маски. Если маска не задана, то ставится маска по умолчанию
	if (file_info_header.biRedMask == 0 || file_info_header.biGreenMask == 0 || file_info_header.biBlueMask == 0)
	{
		file_info_header.biRedMask = mask_value << (bits_on_color * 2);
		file_info_header.biGreenMask = mask_value << bits_on_color;
		file_info_header.biBlueMask = mask_value;
	}

	// bmp v3
	if (file_info_header.biSize >= 56)
	{
		read(file_stream, file_info_header.biAlphaMask, sizeof(file_info_header.biAlphaMask));
	}
	else
	{
		file_info_header.biAlphaMask = mask_value << (bits_on_color * 3);
	}

	// bmp v4
	if (file_info_header.biSize >= 108)
	{
		read(file_stream, file_info_header.biCSType, sizeof(file_info_header.biCSType));
		read(file_stream, file_info_header.biEndpoints, sizeof(file_info_header.biEndpoints));
		read(file_stream, file_info_header.biGammaRed, sizeof(file_info_header.biGammaRed));
		read(file_stream, file_info_header.biGammaGreen, sizeof(file_info_header.biGammaGreen));
		read(file_stream, file_info_header.biGammaBlue, sizeof(file_info_header.biGammaBlue));
	}

	// bmp v5
	if (file_info_header.biSize >= 124)
	{
		read(file_stream, file_info_header.biIntent, sizeof(file_info_header.biIntent));
		read(file_stream, file_info_header.biProfileData, sizeof(file_info_header.biProfileData));
		read(file_stream, file_info_header.biProfileSize, sizeof(file_info_header.biProfileSize));
		read(file_stream, file_info_header.biReserved, sizeof(file_info_header.biReserved));
	}

	// Проверка на поддержку данной версии формата
	if (file_info_header.biSize != 12 && file_info_header.biSize != 40 && file_info_header.biSize != 52 &&
		file_info_header.biSize != 56 && file_info_header.biSize != 108 && file_info_header.biSize != 124)
	{
		throw 1;
	}
	if (file_info_header.biBitCount != 16 && file_info_header.biBitCount != 24 && file_info_header.biBitCount != 32)
	{
		throw 2;
	}
	if (file_info_header.biCompression != 0 && file_info_header.biCompression != 3)
	{
		throw 3;
	}
}

void BMP::get_pixels_info()
{
	pixels.resize(height);
	for (std::vector<PIXEL> & it : pixels)
	{
		it.resize(width);
	}

	// Определение размера отступа в конце каждой строки
	int line_padding = ((width * (file_info_header.biBitCount / 8)) % 4) & 3;

	// Чтение значений RGB для каждого пикселя
	unsigned int bufer;
	unsigned int y = { 0 };
	for (std::vector<PIXEL> & it : pixels)
	{
		unsigned int x = { 0 };
		for (PIXEL & elem : it)
		{
			read(file_stream, bufer, file_info_header.biBitCount / 8);

			elem.rgbRed = bit_extract(bufer, file_info_header.biRedMask);
			elem.rgbGreen = bit_extract(bufer, file_info_header.biGreenMask);
			elem.rgbBlue = bit_extract(bufer, file_info_header.biBlueMask);
			elem.rgbReserved = bit_extract(bufer, file_info_header.biAlphaMask);
			elem.intensity = (unsigned int)(0.299*(unsigned int)elem.rgbRed + 0.587*(unsigned int)elem.rgbGreen + 0.114*(unsigned int)elem.rgbBlue);
			elem.x = x;
			elem.y = y;
			elem.special = { false };
			x++;
		}
		file_stream.seekg(line_padding, std::ios_base::cur);
		y++;
	}
}

void BMP::print_pixels_info()
{
	std::cout << "Width: " << pixels[0].size() << " Height: " << pixels.size() << std::endl;
	for (std::vector<PIXEL> & it : pixels)
	{
		for (PIXEL & elem : it)
		{
			std::cout << std::dec
				<< "R: " << +elem.rgbRed
				<< " G: " << +elem.rgbGreen
				<< " B: " << +elem.rgbBlue
				<< " Res: " << +elem.rgbReserved
				<< " I: " << elem.intensity
				<< " (x, y): (" << elem.x << ", " << elem.y << ")"
				<< std::endl;
		}
	}
}

void BMP::image_resize(std::string img_path)
{
	cv::Mat image = cv::imread(img_path);   // Считывание изображения
	if (!image.data) // Проверка на правильность считывания (непустые данные)
	{
		return;
	}
	cv::resize(image, image, cv::Size(width, height)); // Изменение размеров исходного изображения
	// Изменение цветного изображения в черно-белое (grayscale)
	for (unsigned int i = 0; i < height; ++i)
	{
		for (unsigned int j = 0; j < width; ++j)
		{
			image.at<cv::Vec3b>(i, j)[0] = image.at<cv::Vec3b>(i, j)[1] = image.at<cv::Vec3b>(i, j)[2] = (0.2*image.at<cv::Vec3b>(i, j)[2] + 0.7*image.at<cv::Vec3b>(i, j)[1] + 0.07*image.at<cv::Vec3b>(i, j)[0]);
		}
	}	
	cv::imwrite(img_path, image); // Перезапись исходного изображения с новыми размерами
}

void BMP::image_convert_to_bmp(std::string & img_path)
{
	cv::Mat image = cv::imread(img_path);   // Считывание изображения
	if (!image.data) // Проверка на правильность считывания (непустые данные)
	{
		return;
	}

	std::ofstream os(img_path, std::ios::binary);
	unsigned char signature[2] = { 'B', 'M' };
	unsigned int fileSize = 14 + 40 + image.cols * image.rows * 4;
	unsigned int reserved = 0;
	unsigned int offset = 14 + 40;
	unsigned int headerSize = 40;
	unsigned int dimensions[2] = { image.cols, image.rows };
	unsigned short colorPlanes = 1;
	unsigned short bpp = 32;
	unsigned int compression = 0;
	unsigned int imgSize = image.cols * image.rows * 4;
	unsigned int resolution[2] = { 2795, 2795 };
	unsigned int pltColors = 0;
	unsigned int impColors = 0;

	os.write(reinterpret_cast<char*>(signature), sizeof(signature));
	os.write(reinterpret_cast<char*>(&fileSize), sizeof(fileSize));
	os.write(reinterpret_cast<char*>(&reserved), sizeof(reserved));
	os.write(reinterpret_cast<char*>(&offset), sizeof(offset));
	os.write(reinterpret_cast<char*>(&headerSize), sizeof(headerSize));
	os.write(reinterpret_cast<char*>(dimensions), sizeof(dimensions));
	os.write(reinterpret_cast<char*>(&colorPlanes), sizeof(colorPlanes));
	os.write(reinterpret_cast<char*>(&bpp), sizeof(bpp));
	os.write(reinterpret_cast<char*>(&compression), sizeof(compression));
	os.write(reinterpret_cast<char*>(&imgSize), sizeof(imgSize));
	os.write(reinterpret_cast<char*>(resolution), sizeof(resolution));
	os.write(reinterpret_cast<char*>(&pltColors), sizeof(pltColors));
	os.write(reinterpret_cast<char*>(&impColors), sizeof(impColors));

	unsigned char x, r, g, b;
	/*for (unsigned int i = 0; i < dimensions[1]; ++i)
	{
		for (unsigned int j = 0; j < dimensions[0]; ++j)
		{*/

	for (int i = dimensions[1] - 1; i >= 0; --i)
	{
		for (int j = dimensions[0] - 1; j >= 0; --j)
		{
			x = 0;
			b = image.at<cv::Vec3b>(i, j)[0];
			g = image.at<cv::Vec3b>(i, j)[1];
			r = image.at<cv::Vec3b>(i, j)[2];
			os.write(reinterpret_cast<char*>(&b), sizeof(b));
			os.write(reinterpret_cast<char*>(&g), sizeof(g));
			os.write(reinterpret_cast<char*>(&r), sizeof(r));
			os.write(reinterpret_cast<char*>(&x), sizeof(x));
		}
	}
	os.close();
}

int BMP::image_read(std::string img_path)
{
	std::size_t found = img_path.find_last_of(".");
	if (img_path.substr(found + 1) == "bmp") // Конвертация не нужна, меняем размер
	{
		image_resize(img_path);
	}
	else // Нужна конвертация изображения в bmp
	{
		image_resize(img_path);
		image_convert_to_bmp(img_path);
	}
	try
	{
		this->get_file_header();
		this->get_file_info_header();
		this->get_pixels_info();
	}
	catch (bool)
	{
		return 1; // Не BMP
	}
	catch (int err)
	{
		switch (err)
		{
		case 1:
			return 1; // Неподдерживаемый BMP формат
		case 2:
			return 2; // Неподдерживаемая битность BMP изображения
		case 3:
			return 3; // Неподдерживаемое сжатие
		default:
			break;
		}
	}
}
