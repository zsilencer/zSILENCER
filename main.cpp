#include "shared.h"
#include "game.h"
#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#endif

#ifdef __APPLE__
int SDL_main(int argc, char * argv[]){
#elif defined(POSIX)
int main(int argc, char * argv[]){
#endif
	
#ifdef POSIX
	char cmdlinestr[1024];
	cmdlinestr[0] = 0;
	for(int i = 1; i < argc; i++){
		strcat(cmdlinestr, argv[i]);
		if(i < argc){
			strcat(cmdlinestr, " ");
		}
	}
	char * cmdline = cmdlinestr;
#else
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow){
	char * cmdline = lpCmdLine;
#endif
    	
#ifndef POSIX
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
	
#ifdef __APPLE__
	CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if(!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX)){
        // error!
    }
    CFRelease(resourcesURL);
	
    chdir(path);
#endif
	
	Game game;
	if(!game.Load(cmdline)){
		return false;
	}
	int x, y;
	SDL_GetMouseState(&x, &y);
	srand(x + y + SDL_GetTicks());
	while(1){
		if(!game.HandleSDLEvents()){
			return true;
		}
		if(!game.Loop()){
			return true;
		}
	}
	
	return true;
}