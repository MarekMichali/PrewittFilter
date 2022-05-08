#include "Bitmap.h"
#include <fstream>


Bitmap::Bitmap(int _width, int _height) :width(_width), height(_height) {}

Bitmap::~Bitmap(){}


int Bitmap::read(const char* path) {
	if (sourceImage) {
		delete[] sourceImage;
	}
	std::ifstream image;
	image.open(path, std::ios::in | std::ios::binary);

	if (!image.is_open()) {
		return -1;
	}

	const int headerSize = 14;
	const int informationHeaderSize = 40;

	unsigned char fileHeader[headerSize];
	image.read(reinterpret_cast<char*>(fileHeader), headerSize);

	if (fileHeader[0] != 'B' || fileHeader[1] != 'M') {
		image.close();
		return -1;
	}

	unsigned char informationHeader[informationHeaderSize];
	image.read(reinterpret_cast<char*>(informationHeader), informationHeaderSize);
	if (informationHeader[14] != 24) {
		image.close();
		return -2;
	}
	int fileSize = fileHeader[2] + (fileHeader[3] << 8) + (fileHeader[4] << 16) + (fileHeader[5] << 24);
	width = informationHeader[4] + (informationHeader[5] << 8) + (informationHeader[6] << 16) + (informationHeader[7] << 24);
	height = informationHeader[8] + (informationHeader[9] << 8) + (informationHeader[10] << 16) + (informationHeader[11] << 24);
	const int padding = ((4 - (width * 3) % 4) % 4);
	int size = (width * 3 + padding) * height;
	sourceImage = new unsigned char[size];
	image.read((char*)sourceImage, size);
	image.close();
	return 1;

}

void Bitmap::save(const char* path) {
	std::ofstream image;
	image.open(path, std::ios::out | std::ios::binary);

	if (!image.is_open()) {
		return;
	}

	unsigned char bmpPad[3] = { 0,0,0 };
	const int padding = ((4 - (width * 3) % 4) % 4);

	const int headerSize = 14;
	const int informationHeaderSize = 40;
	const int fileSize = headerSize + informationHeaderSize + width * height * 3 + padding * height;

	unsigned char fileHeader[headerSize];

	fileHeader[0] = 'B';
	fileHeader[1] = 'M';
	fileHeader[2] = fileSize;
	fileHeader[3] = fileSize >> 8;
	fileHeader[4] = fileSize >> 16;
	fileHeader[5] = fileSize >> 24;
	fileHeader[6] = 0;
	fileHeader[7] = 0;
	fileHeader[8] = 0;
	fileHeader[9] = 0;
	fileHeader[10] = headerSize + informationHeaderSize;
	fileHeader[11] = 0;
	fileHeader[12] = 0;
	fileHeader[13] = 0;

	unsigned char informationHeader[informationHeaderSize];

	informationHeader[0] = informationHeaderSize;
	informationHeader[1] = 0;
	informationHeader[2] = 0;
	informationHeader[3] = 0;
	informationHeader[4] = width;
	informationHeader[5] = width >> 8;
	informationHeader[6] = width >> 16;
	informationHeader[7] = width >> 24;
	informationHeader[8] = height;
	informationHeader[9] = height >> 8;
	informationHeader[10] = height >> 16;
	informationHeader[11] = height >> 24;
	informationHeader[12] = 1;
	informationHeader[13] = 0;
	informationHeader[14] = 24;
	for (int i = 15; i < 40; i++) {
		informationHeader[i] = 0;
	}

	image.write(reinterpret_cast<char*>(fileHeader), headerSize);
	image.write(reinterpret_cast<char*>(informationHeader), informationHeaderSize);
	image.write(reinterpret_cast<char*>(sourceImage), (width * 3 + padding) * height);
	delete[] sourceImage;
    image.close();
}

int Bitmap::getWidth() {
	return width;
}

int Bitmap::getHeight() {
	return height;
}

unsigned char* Bitmap::getSourceImageAddress() {
	return sourceImage;
}

