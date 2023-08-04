#include <NosLib/Console.hpp>

#include <iostream>
#include <conio.h>

#include <iostream>
#include <windows.h>
#include <mmsystem.h>

#pragma comment(lib, "winmm.lib")

int main()
{
	NosLib::Console::InitializeModifiers::EnableUnicode();
    NosLib::Console::InitializeModifiers::EnableANSI();
	
	PlaySound(TEXT("terror.wav"), NULL, SND_FILENAME);
	
	wprintf(L"Press any button to continue"); _getch();
    return 0;
}