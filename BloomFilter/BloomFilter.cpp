#include <iostream>
#include <fstream>
#include <math.h>
#include <unordered_map>

class bitmap {
private:
	//размерность чанка
	size_t dimetry = 32u;
	int* bitmap_array;
	size_t bitmap_size;

public:
	bitmap(size_t size) {
		bitmap_size = size;
		size_t array_size = bitmap_size / dimetry;
		if (bitmap_size % dimetry != 0)
			array_size++;
		bitmap_array = new int[array_size]();
	}

	void set(size_t index, bool value) {
		if (index >= bitmap_size) return;
		size_t element_chunk = index / dimetry, element_index = index % dimetry, bit = (1 << element_index);
		if (value)
			bitmap_array[element_chunk] = bitmap_array[element_chunk] | bit;
		else
			bitmap_array[element_chunk] = bitmap_array[element_chunk] & ~bit;
	}

	bool get(size_t index) {
		if (index >= bitmap_size) return false;
		size_t element_chunk = index / dimetry, element_index = index % dimetry;
		return bitmap_array[element_chunk] != 0 && (bitmap_array[element_chunk] & (1 << element_index)) != 0;
	}

	//деструктор; чтобы не было утечки памяти
	~bitmap() {
		delete[] bitmap_array;
	}
};

class bloom_filter {
private:
	size_t hash_1(std::string str) {
		size_t coeff = 54;
		size_t polynomial = 3;
		for (size_t i = 0; i < str.length(); i++)
			polynomial = (coeff * polynomial + (size_t)str[i]);
		return polynomial;
	}

	size_t hash_2(std::string str) {
		size_t coeff = 126;
		size_t polynomial = 4;
		for (size_t i = 0; i < str.length(); i++)
			polynomial = (coeff * polynomial + (size_t)str[i]);
		return polynomial;
	}

	size_t hash_3(std::string str) {
		size_t coeff = 67;
		size_t polynomial = 6;
		for (size_t i = 0; i < str.length(); i++)
			polynomial = (coeff * polynomial + (size_t)str[i]);
		return polynomial;
	}

	bitmap* bitmap_;
	size_t size_of_filter;

public:
	//fpr - погрешность
	bloom_filter(size_t size, double fpr) {
		size_of_filter = size;
		bitmap_ = new bitmap((size_t)round(-(size * log(fpr)) / (pow(log(2), 2))));
	}

	bool get(std::string str) {
		size_t val_1 = hash_1(str) % size_of_filter;
		size_t val_2 = hash_2(str) % size_of_filter;
		size_t val_3 = hash_3(str) % size_of_filter;
		return ((bitmap_->get(val_1)) && (bitmap_->get(val_2)) && (bitmap_->get(val_3)));
	}

	void set(std::string str) {
		size_t val_1 = hash_1(str) % size_of_filter;
		size_t val_2 = hash_2(str) % size_of_filter;
		size_t val_3 = hash_3(str) % size_of_filter;
		bitmap_->set(val_1, true);
		bitmap_->set(val_2, true);
		bitmap_->set(val_3, true);
	}

	//деструктор
	~bloom_filter() {
		delete bitmap_;
	}
};

std::ifstream open_input_file(std::string path_input) {
	std::ifstream file(path_input);
	try {
		file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	}
	catch (std::exception ex) {
		std::throw_with_nested(std::invalid_argument("Ошибка в исходном файле"));
	}
	return file;
};

std::ofstream open_output_file(std::string path_output) {
	std::ofstream file(path_output);
	if (!file.is_open()) {
		throw std::invalid_argument("Ошибка в выходном файле");
	}
	return file;
};

int main(int argc, char** argv)
{
	std::string path_input = argv[1];
	std::string path_output = argv[2];
	std::unordered_map<std::string, bloom_filter*> users_dictionary;

	try {
		if (argc != 3) throw std::length_error("Неверные данные: должно быть 2 входных аргумента");
		std::ifstream filestream_input = open_input_file(path_input);
		std::ofstream filestream_output = open_output_file(path_output);
		double fpr = 0.01;
		std::string input_str;
		size_t bitmap_size = 0u;
		while (filestream_input.good() && !filestream_input.eof()) {
			filestream_input >> input_str;
			if (input_str == "videos") {
				filestream_input >> bitmap_size;
				filestream_output << "Ok\n";
			}
			else if (input_str == "watch") {
				filestream_input >> input_str;
				bloom_filter* user;
				if (users_dictionary[input_str] == NULL)
					users_dictionary[input_str] = new bloom_filter(bitmap_size, fpr);
				user = users_dictionary[input_str];
				filestream_input >> input_str;
				user->set(input_str);
				filestream_output << "Ok\n";
			}
			else if (input_str == "check") {
				filestream_input >> input_str;
				auto user = users_dictionary[input_str];
				filestream_input >> input_str;
				if ((user != NULL && user->get(input_str)))
					filestream_output << "Probably\n";
				else  filestream_output << "No\n";
			}
		}
	}
	catch (std::exception e) {
		std::cout << e.what();
		return 1;
	}
}