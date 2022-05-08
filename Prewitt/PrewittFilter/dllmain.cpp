// dllmain.cpp : Definiuje punkt wejścia dla aplikacji DLL.
#include "pch.h"
#include <math.h>  
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}


#define EOF (-1)

#ifdef __cplusplus   
extern "C" {        
#endif

	__declspec(dllexport) void PrewittThread(int width, int height, int startRow, int endRow, unsigned char* sourceImage, unsigned char* targetImage) {
		const int padding = ((4 - (width * 3) % 4) % 4);
		int size = (width * 3 + padding) * (height);
		int index = 0;
		double xValue = 0.0;
	    double yValue = 0.0;
		double color = 0.0;
		//index startowy dla otrzymanych danych
		index = startRow * (width * 3 + padding);
		if ((index >= ((width * 3 + padding) * height))) {
			return;
		}
		for (int j = startRow; j < endRow; j++) {
			for (int i = 0; i < width; i++) {
				if ((i == 0) || (i == width - 1) || (j == 0) || (j == height - 1)){
					xValue = 1;
					yValue = 1;
				}
				else {
					xValue = 0;
					yValue = 0;

					xValue = xValue + sourceImage[padding + index - 3 - width - width - width];
					yValue = yValue + sourceImage[padding + index - 3 - width - width - width];

					yValue = yValue + sourceImage[padding + index - width - width - width];
					yValue = yValue + sourceImage[padding + index + 3 - width - width - width];

					xValue = xValue + sourceImage[index - 3];
					xValue = xValue + sourceImage[width + width + width + padding + index - 3];
							
					yValue = yValue - sourceImage[width + width + width + padding + index];

					xValue = xValue - sourceImage[width + width + width + padding + index + 3];
					yValue = yValue - sourceImage[width + width + width + padding + index + 3];

					xValue = xValue - sourceImage[padding + index + 3 - width - width - width];
					xValue = xValue - sourceImage[index + 3];
					yValue = yValue - sourceImage[width + width + width + padding + index - 3];

				}	
				color = sqrt(xValue * xValue + yValue * yValue);
				targetImage[index] = static_cast<unsigned char>(color);
				targetImage[index + 1] = static_cast<unsigned char>(color);
				targetImage[index + 2] = static_cast<unsigned char>(color);
				index = index + 3;
			}
			index = index + padding;
		}	
	}

#ifdef __cplusplus
}
#endif