#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <unistd.h>
#include <3ds.h>
#define SECOND(x) (x*1000ULL*1000ULL*1000ULL)
#define SYSTEMINFO_TYPE_CFWINFO 0x10000
#define CFWINFO_PARAM_ISRELEASE 0x200
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
	cout << "\nTWLFix v1.2!\n\n";

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
	cout << "Initializing CFG services\n";
	if (R_FAILED(cfguInit())) {
		cout << "Failed to initialize CFG services\n";
		fucked();
	}
}
void exitServices() {
		cfguExit();
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
	u8 region=0;
	if (R_FAILED(CFGU_SecureInfoGetRegion(&region))) {
		region=255;
	}
	bool isRelease = (OS_KernelConfig->env_info & 0b1) != 0;
	if (!isRelease) {
		cout << "\n*****\nSystem might be a devkit\n*****\n\nRunning this on a devkit WILL ONLY BREAK THINGS!\n";
	}
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
	
	vector<std::pair<std::string,u64>> Breakables =  {
		std::make_pair("Whitelist",0x0004800f484e4841),		// Whitelist
		std::make_pair("Version Data",0x0004800f484e4C41),		// Version Data
		std::make_pair("DS Internet",0x0004800542383841),		// DS Internet
	};
	if (isN3ds) {
		Breakables.push_back(std::make_pair("TWL Firm (n3DS)",0x0004013820000102));		// twlfirm n3ds

	}else{
		Breakables.push_back(std::make_pair("TWL Firm (o3DS)",0x0004013800000102));	// twlfirm o3ds
	}
	if (region==4 || region==255) {
		Breakables.push_back(std::make_pair("DS Download Play (CHN)",0x00048005484E4443));		// DS Dlp
	}else if (region==5 || region==255) {
		Breakables.push_back(std::make_pair("DS Download Play (KOR)",0x00048005484E444B));		// DS Dlp
	}else if(region != 4 && region != 5){
		Breakables.push_back(std::make_pair("DS Download Play",0x00048005484E4441));		// DS Dlp
	}
	for (vector<std::pair<std::string,u64>>::iterator title=Breakables.begin();title != Breakables.end(); ++title) {
		cout << "Uninstalling " << (*title).first << "\t";
		if (R_FAILED(AM_DeleteTitle(MEDIATYPE_NAND, (*title).second))) {
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
