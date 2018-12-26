#include "specialpoints.h"

SpecialPoints::SpecialPoints(std::vector<std::vector<PIXEL>> pixels) : pixels(pixels) {}

SpecialPoints::~SpecialPoints() {}

void SpecialPoints::print_special_points()
{
	for (const auto & it : special_points)
	{
		std::cout << std::dec
			<< "R: " << +it.first.rgbRed
			<< " G: " << +it.first.rgbGreen
			<< " B: " << +it.first.rgbBlue
			<< " Res: " << +it.first.rgbReserved
			<< " I: " << it.first.intensity
			<< " (x, y): (" << it.first.x << ", " << it.first.y << ")"
			<< std::endl;
	}
}

void SpecialPoints::get_processed_image(std::string file_name)
{
	std::ofstream os(file_name, std::ios::binary);

	unsigned char signature[2] = { 'B', 'M' };
	unsigned int fileSize = 14 + 40 + width * height * 4;
	unsigned int reserved = 0;
	unsigned int offset = 14 + 40;
	unsigned int headerSize = 40;
	unsigned int dimensions[2] = { width, height };
	unsigned short colorPlanes = 1;
	unsigned short bpp = 32;
	unsigned int compression = 0;
	unsigned int imgSize = width * height * 4;
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
	unsigned int k = { 0 };
	for (unsigned int i = 0; i < dimensions[1]; ++i)
	{
		for (unsigned int j = 0; j < dimensions[0]; ++j)
		{
			x = 0;
			if (k < special_points.size() && special_points[k].first.x == pixels[i][j].x && special_points[k].first.y == pixels[i][j].y) // особые точки выделяются красным
			{
				r = { 255 };
				g = { 0 };
				b = { 0 };
				k++;
			}
			else
			{
				r = pixels[i][j].rgbRed;
				g = pixels[i][j].rgbGreen;
				b = pixels[i][j].rgbBlue;
			}
			os.write(reinterpret_cast<char*>(&b), sizeof(b));
			os.write(reinterpret_cast<char*>(&g), sizeof(g));
			os.write(reinterpret_cast<char*>(&r), sizeof(r));
			os.write(reinterpret_cast<char*>(&x), sizeof(x));
		}
	}
	os.close();
}

void SpecialPoints::make_circle(const unsigned int & y, const unsigned int & x, const unsigned int & radius, bool fast_alg)
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
	if (fast_alg)
	{
		for (unsigned int i = { 0 }; i < 2 * (radius + 1); ++i) /*8*/
		{
			circle.push_back(circle[i]);
		}
	}
}

void SpecialPoints::harris_selection()
{
	for (; special_points.size() > max_count; harris_threshold *= 2)
	{
		std::vector<int> indexes; // Индексы точек, которые удаляются из особых
		for (std::vector<std::pair<PIXEL, double>>::iterator it = special_points.begin(); it != special_points.end(); ++it)
		{
			int off_x = { 1 }; // Смещение координаты x - начальной для смещенного окна
			int off_y = { 1 }; // Смещение координаты y - начальной для смещенного окна
			double res = { 0.0 }; // Суммарное значение мер Харриса для окрестностей текущей особой точки
			while (off_x != -1 && off_y != -2)
			{
				if (off_y == -2)
				{
					off_x--;
					off_y = { 1 };
				}
				if (harris_response(off_x, off_y, it, indexes, res) == false) // Получение результата вычисления меры Харриса для смещенного окна
				{
					break;
				}
				off_y = off_y - 1;
				if (off_x == 0 && off_y == 0) off_y = { -1 };
			}
			if (res != 0.0)
			{
				it->second = res;
			}
		}
		int i = { 0 };
		for (const auto & elem : indexes) // Удаление не угловых особых точек
		{
			pixels[special_points[(elem - i)].first.y][special_points[(elem - i)].first.x].special = { false };
			special_points.erase(special_points.begin() + (elem - i));
			++i;
		}
		i = { 0 };
		indexes.clear();
		// Поиск локальных максимумов
		for (std::vector<std::pair<PIXEL, double>>::iterator it = special_points.begin(); it != special_points.end() - 1; ++it)
		{
			for (unsigned int r = { 1 }; r <= radius; ++r) // рассматриваем окрестность особой точки, начиная от радиуса равного 1 до 3
			{
				make_circle(it->first.y, it->first.x, r, false);
				for (const auto & coordinate : circle)
				{
					if (coordinate.first <= 0 || coordinate.second <= 0 || coordinate.first >= height || coordinate.second >= width)
					{
						continue;
					}
					if (pixels[coordinate.first][coordinate.second].special == true) // Если на окружности оказалась особая точка
					{
						std::vector<std::pair<PIXEL, double>>::iterator it_neighbor = it; // Начиная от текущей особой точки ищем ту, которая оказалась на окружности
						for (; it_neighbor->first.y != coordinate.first && it_neighbor->first.x != coordinate.second;)
						{
							if (coordinate.first > it->first.y) // Если точка на окружности выше центральной точки, то идем вверх
								++it_neighbor;
							else if (coordinate.first < it->first.y) // Иначе идем вниз
								--it_neighbor;
						}
						if (it->second <= it_neighbor->second) // Если мера Харриса для текущей (центральной) особой точки больше, чем у той, что на окружности, то удаляем вторую
						{
							if (std::find(indexes.begin(), indexes.end(), it - special_points.begin()) == indexes.end())
								indexes.push_back(std::distance(special_points.begin(), it));
						}
						else // Иначе удаляем текущую
						{
							if (std::find(indexes.begin(), indexes.end(), it_neighbor - special_points.begin()) == indexes.end())
								indexes.push_back(std::distance(special_points.begin(), it_neighbor));
						}
					}
				}
			}
		}
		// Так как выбор индексов шел не снизу вверх, а в завсисимости от расположения особой точки на окружности, 
		// надо предварительно отсортировать вектор индексов удаляемых точек
		std::sort(indexes.begin(), indexes.end());
		for (const auto & elem : indexes)
		{
			special_points.erase(special_points.begin() + (elem - i));
			++i;
		}
		i = { 0 };
		indexes.clear();
	}
	harris_threshold = { 4096 };
}

double SpecialPoints::w(const unsigned int & x_0, const unsigned int & y_0, const unsigned int & x, const unsigned int & y)
{
	return (double)exp(-((double)((x - x_0) * (x - x_0) + (y - y_0) * (y - y_0))/(2.0 * sigma * sigma))) / (two_pi * sigma * sigma);
}

bool SpecialPoints::harris_response(const int & start_x, const int & start_y, const std::vector<std::pair<PIXEL, double>>::iterator & it, std::vector<int> & indexes, double & res)
{
	double a = { 0.0 };
	double b = { 0.0 };
	double c = { 0.0 };
	if (int(it->first.y) + start_y <= 0 || int(it->first.y) + start_y >= special_points.size() || int(it->first.x) + start_x <= 0 || int(it->first.x) + start_x >= special_points.size())
		return false;
	for (unsigned int y = it->first.y + start_y; y < it->first.y + start_y + 3; ++y)
	{
		for (unsigned int x = it->first.x + start_x; x < it->first.x + start_x + 3; ++x)
		{
			double w_ = w(it->first.x, it->first.y, x, y);
			int I_diff_x = pixels[y][x].intensity - pixels[y][x - 1].intensity;
			int I_diff_y = pixels[y][x].intensity - pixels[y - 1][x].intensity;
			a += w_ * I_diff_x * I_diff_x;
			b += w_ * I_diff_y * I_diff_y;
			c += w_ * I_diff_x * I_diff_y;
		}
	}
	double R = (((a * b) - (c * c)) - harris_k * (a + b) * (a + b));
	if (R < harris_threshold)
	{
		res = { 0.0 };
		indexes.push_back(std::distance(special_points.begin(), it));
		return false;
	}
	res += R;
	return true;
}

void SpecialPoints::fast()
{
	for (unsigned int y = { radius }; y < height - radius; ++y)
	{
		for (unsigned int x = { radius }; x < width - radius; ++x)
		{
			// Предварительная проверка четырех пикселей под номерами : 1, 5, 9, 13
			unsigned int count = { 0 }; // Количество пикселей светлее или темнее центрального
			int intensity_difference = { 0 }; // Разница яркостей
			intensity_difference = int(pixels[y][x].intensity) - int(pixels[y + radius][x].intensity);
			if (intensity_difference > fast_threshold || intensity_difference < -fast_threshold)
				count++;
			intensity_difference = int(pixels[y][x].intensity) - int(pixels[y][x + radius].intensity);
			if (intensity_difference > fast_threshold || intensity_difference < -fast_threshold)
				count++;
			intensity_difference = int(pixels[y][x].intensity) - int(pixels[y - radius][x].intensity);
			if (intensity_difference > fast_threshold || intensity_difference < -fast_threshold)
				count++;
			intensity_difference = int(pixels[y][x].intensity) - int(pixels[y][x - radius].intensity);
			if (intensity_difference > fast_threshold || intensity_difference < -fast_threshold)
				count++;
			if (count >= 3) // Проверка всех 16 пикселей
			{
				make_circle(y, x, radius, true);
				unsigned int length = { 0 }; // Длина подряд идущих пикселей из окружности, удовлетворяющих условию |Ix - Ic| > t
				for (std::deque<std::pair<unsigned int, unsigned int>>::iterator it = circle.begin(); it != circle.end(); it++)
				{
					intensity_difference = int(pixels[y][x].intensity) - int(pixels[it->first][it->second].intensity);
					if (intensity_difference > fast_threshold || intensity_difference < -fast_threshold)
						length++;
					else
						length = { 0 };
					if (length == 9) // Если есть 9 подряд идущих пикселей, удовлетворяющих условию |Ix - Ic| > t
					{
						special_points.push_back(std::pair<PIXEL, double>(pixels[y][x], 0.0));
						pixels[y][x].special = { true };
						break;
					}
				}
			}
		}
	}
	// Отбор N особых точек с помощью вычисления меры Харриса
	harris_selection();
}
