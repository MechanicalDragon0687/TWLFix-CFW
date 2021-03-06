#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <unistd.h>
#include <3ds.h>
#define SECOND(x) (x*1000ULL*1000ULL*1000ULL)
PrintConsole topScreen, bottomScreen;
using namespace std;
void initServices();
void exitServices();
void fucked();


void fucked() {
	cout << "\n\nPress [Start] to exit";	
	while (1) {
		hidScanInput();
		if (hidKeysDown() & KEY_START) { 
			exitServices();
			exit(0);
		}
	}
}


void initServices() {
	gfxInitDefault();
	consoleInit(GFX_TOP, &topScreen);
	consoleInit(GFX_BOTTOM, &bottomScreen);
	consoleSelect(&bottomScreen);
	cout << "Initializing APT services\n";
	if (R_FAILED(aptInit())) {
		cout << "Failed to initialize APT services\n";
		fucked();
	}
	cout << "Initializing AM services\n";
	if (R_FAILED(amInit())) {
		cout << "Failed to initialize AM services\n";
		fucked();
	}
	cout << "Initializing PTM services\n";
	if (R_FAILED(ptmSysmInit())) {
		cout << "Failed to initialize PTM services\n";
		fucked();
	}
}
void exitServices() {
		ptmSysmExit();
		amExit();
		aptExit();
		gfxExit();
}


int main(int argc, char* argv[])
{

	bool isN3ds = false;
	initServices();
	APT_CheckNew3DS(&isN3ds);


	cout << "\nPress [A] to begin or [Start] to Exit!\n\n";
	
	while (1) {
		hidScanInput();
		if (hidKeysDown() & KEY_A) { break; }
		if (hidKeysDown() & KEY_START) { 
			exitServices();
			return 0;
		}
	}


	cout << "Uninstalling System DSiWare:\n";

	vector<u64> Breakables =  {
		0x0004800f484e4841,		// Whitelist
		0x0004800f484e4C41,		// Version Data
		0x0004800542383841,		// DS Internet
		0x00048005484E4441		// DS Dlp
	};

	if (isN3ds) {
		Breakables.push_back(0x0004013820000102); // N3DS TWL Firm
	}else{
		Breakables.push_back(0x0004013800000102);		// o3DS TWL Firm

	}
	for (vector<u64>::iterator title=Breakables.begin();title != Breakables.end(); ++title) {
		cout << "Uninstalling " << setw(16) << setfill('0') << hex << *title << setw(0) << "\t";
		if (R_FAILED(AM_DeleteTitle(MEDIATYPE_NAND, *title))) {
			cout << "Failed\n";
		}else{
			cout << "Success\n";
		}
	}

	cout<<"\nDone!\nReboot and then open System Update.\n\nPress Start to reboot.\n";

	while (aptMainLoop()) {
		hidScanInput();
		if (hidKeysDown() & KEY_START) break; 
		gfxFlushBuffers();
		gfxSwapBuffers();
		gspWaitForVBlank();
	} 
	PTMSYSM_RebootAsync(0);
	exitServices();

	return 0;
}
