#include "os.h"

std::string GetResDir(void){
#ifdef __linux
	std::string d[2] = {"/usr/local/share/zsilencer", "/usr/share/zsilencer"};
	for(size_t i = 0; i < 2; i++){
		struct stat dirinfo;
		if(stat(d[i].c_str(), &dirinfo) == 0 && S_ISDIR(dirinfo.st_mode)){
			return d[i] + "/";
		}
	}
	return "";
#else
	return "";
#endif
}

std::string GetDataDir(void){
#ifdef __linux
	std::string d = getpwuid(getuid())->pw_dir;
	d += "/.config/zsilencer/";
	CreateDirectory(d.c_str());
	return d;
#else
	return "";
#endif
}

mode_t getumask(){
#ifdef WIN32
    mode_t mask = _umask(0);
	_umask(mask);
#else
	mode_t mask = umask(0);
	umask(mask);
#endif
    return 0777 & ~mask;
}

void CreateDirectory(const char * path){
    mode_t mask = getumask();
    char buffer[260];
    strcpy(buffer, path);

    for(char *p = buffer + 1; *p != '\0'; p++){
        if(*p == '/'){
            *p = '\0';
#ifdef WIN32
			if(_mkdir(buffer, mask) != 0){
#else
            if(mkdir(buffer, mask) != 0){
#endif
                if(errno != EEXIST){
                    return;
                }
            }
            *p = '/';
        }
    }

#ifdef WIN32
	if(_mkdir(buffer, mask) != 0){
#else
    if(mkdir(buffer, mask) != 0){
#endif
        if(errno != EEXIST){
            return;
        }
    }
}
