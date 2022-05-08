#pragma once

/**
* Przechowuje informacje dla jednego watku, jaki obszar bitmapy ma przetworzyc i gdzie zapisac wynik
*/

struct Pack {
    int startRow; /// od ktorego wiersza ma rozpoczac przetwarzanie
    int endRow; /// do ktorego wiersza ma przetwarzac
    unsigned char* target; ///tablica do ktorej nalezy zapisywac przetwarzany kawalek bitmapy
};