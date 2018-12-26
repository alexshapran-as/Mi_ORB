#include "descriptors.h"

Descriptors::Descriptors(std::vector<std::pair<PIXEL, double>> special_points, std::vector<std::vector<PIXEL>> pixels) : special_points(special_points), pixels(pixels) {}

Descriptors::~Descriptors() {}

void Descriptors::brief()
{
	std::vector<bool> zero_vec;
	zero_vec.assign(nd_dimensional, 0);
	for (std::vector<std::pair<PIXEL, double>>::iterator it = special_points.begin(); it != special_points.end(); ++it)
	{
		double tetta = get_tetta(it->first.x, it->first.y);
		std::vector<bool> t; // Тест на патче размера (size x size)
		for (unsigned int i = { 0 }; i < nd_dimensional; ++i) // Проведение 256 сравнений и получение дескриптора для патча
		{
			size = { 10 };
			while (it->first.x + (size * size / 25) >= width || it->first.y + (size * size / 25) >= height || it->first.x - (size * size / 25) < 0 || it->first.y - (size * size / 25) < 0) // Если патч большой
			{
				size /= 2;
			}
			int x_1, y_1, x_2, y_2; // Координаты двух пикселей, которые будут участвовать в сравнении
			int x_1_rotated, y_1_rotated, x_2_rotated, y_2_rotated; // Повернутые координаты
			do
			{
				// https://www.researchgate.net/publication/51798992_BRIEF_Computing_a_local_binary_descriptor_very_fast
				x_1 = (0 + rand() % (size * size / 25));
				y_1 = (0 + rand() % (size * size / 25));
				x_2 = (0 + rand() % (size * size / 25));
				y_2 = (0 + rand() % (size * size / 25));
				x_1_rotated = it->first.x + (int)round((x_1 * cos(tetta)) - (y_1 * sin(tetta)));
				y_1_rotated = it->first.y + (int)round((x_1 * sin(tetta)) + (y_1 * cos(tetta)));
				x_2_rotated = it->first.x + (int)round((x_2 * cos(tetta)) - (y_2 * sin(tetta)));
				y_2_rotated = it->first.y + (int)round((x_2 * sin(tetta)) + (y_2 * cos(tetta)));
			} while (x_1_rotated < 0 || x_2_rotated < 0 || y_1_rotated < 0 || y_2_rotated < 0 ||
					 x_1_rotated >= width || x_2_rotated >= width || y_1_rotated >= height || y_2_rotated >= height);
			std::pair<unsigned int, unsigned int> blurred_pixel_intensity_values = get_blurred_pixel_intensity_values(x_1_rotated, y_1_rotated, x_2_rotated, y_2_rotated);
			if (blurred_pixel_intensity_values.first < blurred_pixel_intensity_values.second)
				t.push_back(1);
			else
				t.push_back(0);
		}
		if (t != zero_vec)
		{
			descriptors.push_back(std::pair<std::pair<PIXEL, double>, std::vector<bool>>(*it, t));
		}
	}
}

double Descriptors::get_tetta(const unsigned int & x, const unsigned int & y)
{
	int m_0_1 = { 0 };
	int m_1_0 = { 0 };
	make_circle(y, x);
	for (const auto & coordinate : circle)
	{
		m_0_1 += coordinate.first * pixels[coordinate.first][coordinate.second].intensity; // sum(y*I(x,y))
		m_1_0 += coordinate.second * pixels[coordinate.first][coordinate.second].intensity; // sum(x*I(x,y))
	}
	return atan((double)m_0_1 / m_1_0);
}

void Descriptors::make_circle(const unsigned int & y, const unsigned int & x)
{
	circle.clear();
	circle.push_back(std::pair<unsigned int, unsigned int>(y + radius, x));
	for (unsigned int i = { 0 }; i < 2 * radius + 1; ++i)
	{
		unsigned int j = { 0 };
		if (i == 1 || i == 5)
			j = { 1 };
		else if (i == 2 || i == 3 || i == 4)
			j = { 2 };
		circle.push_front(std::pair<unsigned int, unsigned int>(y - radius + i, x + 1 + j));
		circle.push_back(std::pair<unsigned int, unsigned int>(y - radius + i, x - 1 - j));
	}
	circle.push_front(std::pair<unsigned int, unsigned int>(y + radius, x));
}

std::pair<unsigned int, unsigned int> Descriptors::get_blurred_pixel_intensity_values(const int & x_1, const int & y_1, const int & x_2, const int & y_2)
{
	unsigned int new_intensity_1 = { 0 };
	unsigned int new_intensity_2 = { 0 };
	unsigned int count = { 0 };
	for (int i = { -2 }; i != 3; ++i)
	{
		for (int j = { -2 }; j != 3; ++j)
		{
			if (x_1 + i >= 0 && x_2 + i >= 0 && x_1 + i < width && x_2 + i < width && y_1 + j >= 0 && y_2 + j >= 0 && y_1 + j < height && y_2 + j < height)
			{
				new_intensity_1 += pixels[y_1 + j][x_1 + i].intensity;
				new_intensity_2 += pixels[y_2 + j][x_2 + i].intensity;
				count++;
			}
		}
	}
	new_intensity_1 = new_intensity_1 / count;
	new_intensity_2 = new_intensity_2 / count;
	if (new_intensity_1 == 0)
		new_intensity_1 = pixels[y_1][x_1].intensity;
	if (new_intensity_2 == 0)
		new_intensity_2 = pixels[y_2][x_2].intensity;
	return std::pair<unsigned int, unsigned int>(new_intensity_1, new_intensity_2);
}

void Descriptors::save_descriptors(std::string file_name, std::string image_name)
{
	std::ofstream fout(file_name, std::ios_base::app);
	if (!fout.is_open())
	{
		return;
	}
	fout << "{" << std::endl << "\"ImageName\": \"" << image_name << "\","<< std::endl << "\"Count\": \"" << descriptors.size() << "\"," << std::endl 
		<< "\"Descriptors\": [" << std::endl;
	for (auto point : descriptors)
	{
		fout << "\"";
		for (auto desc : point.second)
		{
			fout << desc;
		}
		fout << "\"," << std::endl;
	}
	fout << "]" << std::endl  << "}" << std::endl;
	fout.close();
}

