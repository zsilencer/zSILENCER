#include "shared.h"
#include "game.h"
#ifdef __APPLE__
#include "CoreFoundation/CoreFoundation.h"
#endif

#ifdef __ANDROID__
#include <jni.h>

static Game * gameglobal = 0;
static SDL_Event pushedevent;
JNIEnv * env;
JavaVM * jvm;

extern "C" void Java_com_zSILENCER_game_zSILENCER_OuyaControllerKeyEvent(JNIEnv * env, jclass cls, jint player, jint type, jint keycode){
    printf("native ouya key down %d\n", keycode);
	if(type == 1){
		pushedevent.type = SDL_KEYDOWN;
	}else{
		pushedevent.type = SDL_KEYUP;
	}
	int keycode2 = keycode;
	switch(keycode){
		case 19: keycode2 = SDL_SCANCODE_UP; break;
		case 20: keycode2 = SDL_SCANCODE_DOWN; break;
		case 21: keycode2 = SDL_SCANCODE_LEFT; break;
		case 22: keycode2 = SDL_SCANCODE_RIGHT; break;
		case 82: keycode2 = SDL_SCANCODE_HOME; break; // Menu
		case 96: keycode2 = SDL_SCANCODE_RETURN; break; // O
		case 97: keycode2 = SDL_SCANCODE_ESCAPE; break; // A
	}
	pushedevent.key.keysym.scancode = (SDL_Scancode)keycode2;
	SDL_PushEvent(&pushedevent);
}

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
#ifdef __ANDROID__
		exit(-1);
#endif
		return -1;
    }
    CFRelease(resourcesURL);
	
    chdir(path);
#endif
	
	Game game;
#ifdef __ANDROID__
	gameglobal = &game;
#endif
	if(!game.Load(cmdline)){
#ifdef __ANDROID__
		exit(-1);
#endif
		return -1;
	}

	int x, y;
	SDL_GetMouseState(&x, &y);
	srand(x + y + SDL_GetTicks());
	while(1){
		if(!game.HandleSDLEvents()){
#ifdef __ANDROID__
			exit(0);
#endif
			return 0;
		}
		if(!game.Loop()){
#ifdef __ANDROID__
			exit(0);
#endif
			return 0;
		}
	}
#ifdef __ANDROID__
	exit(0);
#endif
	return 0;
}