#pragma once
class Bitmap
{
public:
    int width; /// szeroksoc bitmapy
    int height; /// wysoksoc bitmapy
    unsigned char* sourceImage; //wskaznik na tablice przechowujaca piksele bitmapy
public:
    /**
    * Wczytuje bitmape wybrana przez uzytkownika
    *
    * @param path sciezka do bitmapy
    * @return wartosc int orkeslajaca czy wczytanie sie powiodlo
    */
    int read(const char* path);
    /**
    * Zapisuje przetworzona bitmape
    *
    * @param path sciezka do zapisu
    */
    void save(const char* path);
    /**
    * Zwraca szerokosc bitmapy
    * 
    * @return szerokosc
    */
    int getWidth();
    /**
    * Zwraca wysokosc bitmapy
    *
    * @return wysokosc
    */
    int getHeight();
    /**
    * Zwraca adres bitmapy
    *
    * @return adres bitmapy
    */
    unsigned char* getSourceImageAddress();
    Bitmap(int width, int height);
    ~Bitmap();
};

