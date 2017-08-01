#include "config.h"
#include "team.h"
#include "os.h"
#include <sstream>

Config::Config(){
	LoadDefaults();
	Load();
	Save();
}

Config & Config::GetInstance(void){
	static Config instance;
	return instance;
}

void Config::Save(void){
	CDDataDir();
	SDL_RWops * file = SDL_RWFromFile((GetDataDir() + "config.cfg").c_str(), "w");
	if(file){
		char temp[256];
		WriteString(file, "fullscreen", fullscreen ? "1" : "0");
		WriteString(file, "scalefilter", scalefilter ? "1" : "0");
		WriteString(file, "teamcolors", teamcolors ? "1" : "0");
		WriteString(file, "music", music ? "1" : "0");
		sprintf(temp, "%d", musicvolume); WriteString(file, "musicvolume", temp);
		sprintf(temp, "%d", defaultagency); WriteString(file, "defaultagency", temp);
		WriteString(file, "defaultgamename", defaultgamename);
		sprintf(temp, "%d", defaulttechchoices[0]); WriteString(file, "defaulttechchoices0", temp);
		sprintf(temp, "%d", defaulttechchoices[1]); WriteString(file, "defaulttechchoices1", temp);
		sprintf(temp, "%d", defaulttechchoices[2]); WriteString(file, "defaulttechchoices2", temp);
		sprintf(temp, "%d", defaulttechchoices[3]); WriteString(file, "defaulttechchoices3", temp);
		sprintf(temp, "%d", defaulttechchoices[4]); WriteString(file, "defaulttechchoices4", temp);
		WriteKey(file, "keymoveup", keymoveupbinding, keymoveupoperator);
		WriteKey(file, "keymovedown", keymovedownbinding, keymovedownoperator);
		WriteKey(file, "keymoveleft", keymoveleftbinding, keymoveleftoperator);
		WriteKey(file, "keymoveright", keymoverightbinding, keymoverightoperator);
		WriteKey(file, "keylookupleft", keylookupleftbinding, keylookupleftoperator);
		WriteKey(file, "keylookupright", keylookuprightbinding, keylookuprightoperator);
		WriteKey(file, "keylookdownleft", keylookdownleftbinding, keylookdownleftoperator);
		WriteKey(file, "keylookdownright", keylookdownrightbinding, keylookdownrightoperator);
		WriteKey(file, "keynextinv", keynextinvbinding, keynextinvoperator);
		WriteKey(file, "keynextcam", keynextcambinding, keynextcamoperator);
		WriteKey(file, "keyprevcam", keyprevcambinding, keyprevcamoperator);
		WriteKey(file, "keydetonate", keydetonatebinding, keydetonateoperator);
		WriteKey(file, "keyjump", keyjumpbinding, keyjumpoperator);
		WriteKey(file, "keyjetpack", keyjetpackbinding, keyjetpackoperator);
		WriteKey(file, "keyactivate", keyactivatebinding, keyactivateoperator);
		WriteKey(file, "keyuse", keyusebinding, keyuseoperator);
		WriteKey(file, "keyfire", keyfirebinding, keyfireoperator);
		WriteKey(file, "keychat", keychatbinding, keychatoperator);
		WriteKey(file, "keydisguise", keydisguisebinding, keydisguiseoperator);
		WriteKey(file, "keynextweapon", keynextweaponbinding, keynextweaponoperator);
		SDL_RWclose(file);
	}
}

bool Config::Load(void){
	CDDataDir();
	SDL_RWops * file = SDL_RWFromFile((GetDataDir() + "config.cfg").c_str(), "r");
	if(file){
		char line[256];
		while(RWgets(file, line, sizeof(line))){
			char * variable = strtok(line, "=");
			char * data = strtok(NULL, "=");
			if(variable && data){
				char vardata[64];
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "fullscreen")){ if(atoi(data) == 0){ fullscreen = false; }else{ fullscreen = true; } }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "scalefilter")){ if(atoi(data) == 0){ scalefilter = false; }else{ scalefilter = true; } }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "teamcolors")){ if(atoi(data) == 0){ teamcolors = false; }else{ teamcolors = true; } }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "music")){ if(atoi(data) == 0){ music = false; }else{ music = true; } }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "musicvolume")){ musicvolume = atoi(data); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "defaultagency")){ defaultagency = atoi(data); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "defaultgamename")){ ReadString(data, defaultgamename, sizeof(defaultgamename)); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "defaulttechchoices0")){ defaulttechchoices[0] = atoi(data); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "defaulttechchoices1")){ defaulttechchoices[1] = atoi(data); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "defaulttechchoices2")){ defaulttechchoices[2] = atoi(data); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "defaulttechchoices3")){ defaulttechchoices[3] = atoi(data); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "defaulttechchoices4")){ defaulttechchoices[4] = atoi(data); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keymoveup")){ ReadKey(data, &keymoveupbinding, &keymoveupoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keymovedown")){ ReadKey(data, &keymovedownbinding, &keymovedownoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keymoveleft")){ ReadKey(data, &keymoveleftbinding, &keymoveleftoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keymoveright")){ ReadKey(data, &keymoverightbinding, &keymoverightoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keylookupleft")){ ReadKey(data, &keylookupleftbinding, &keylookupleftoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keylookupright")){ ReadKey(data, &keylookuprightbinding, &keylookuprightoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keylookdownleft")){ ReadKey(data, &keylookdownleftbinding, &keylookdownleftoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keylookdownright")){ ReadKey(data, &keylookdownrightbinding, &keylookdownrightoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keynextinv")){ ReadKey(data, &keynextinvbinding, &keynextinvoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keynextcam")){ ReadKey(data, &keynextcambinding, &keynextcamoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keyprevcam")){ ReadKey(data, &keyprevcambinding, &keyprevcamoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keydetonate")){ ReadKey(data, &keydetonatebinding, &keydetonateoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keyjump")){ ReadKey(data, &keyjumpbinding, &keyjumpoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keyjetpack")){ ReadKey(data, &keyjetpackbinding, &keyjetpackoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keyactivate")){ ReadKey(data, &keyactivatebinding, &keyactivateoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keyuse")){ ReadKey(data, &keyusebinding, &keyuseoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keyfire")){ ReadKey(data, &keyfirebinding, &keyfireoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keychat")){ ReadKey(data, &keychatbinding, &keychatoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keydisguise")){ ReadKey(data, &keydisguisebinding, &keydisguiseoperator); }
				ReadString(variable, vardata, sizeof(vardata)); if(CompareString(vardata, "keynextweapon")){ ReadKey(data, &keynextweaponbinding, &keynextweaponoperator); }
			}
		}
		SDL_RWclose(file);
		return true;
	}
	return false;
}

void Config::LoadDefaults(void){
	fullscreen = true;
	scalefilter = true;
	teamcolors = false;
	music = true;
	musicvolume = 48;
	defaultagency = Team::NOXIS;
	strcpy(defaultgamename, "New Game");
	defaulttechchoices[0] = World::BUY_LASER | World::BUY_ROCKET;
	defaulttechchoices[1] = World::BUY_LASER | World::BUY_ROCKET;
	defaulttechchoices[2] = World::BUY_LASER | World::BUY_ROCKET;
	defaulttechchoices[3] = World::BUY_LASER | World::BUY_ROCKET;
	defaulttechchoices[4] = World::BUY_LASER | World::BUY_ROCKET;
#ifdef OUYA
	keymoveupbinding[0] = SDL_SCANCODE_UP; keymoveupbinding[1] = SDL_SCANCODE_UNKNOWN; keymoveupoperator = OR;
	keymovedownbinding[0] = SDL_SCANCODE_DOWN; keymovedownbinding[1] = SDL_SCANCODE_UNKNOWN; keymovedownoperator = OR;
	keymoveleftbinding[0] = SDL_SCANCODE_LEFT; keymoveleftbinding[1] = SDL_SCANCODE_UNKNOWN; keymoveleftoperator = OR;
	keymoverightbinding[0] = SDL_SCANCODE_RIGHT; keymoverightbinding[1] = SDL_SCANCODE_UNKNOWN; keymoverightoperator = OR;
	keyupbinding[0] = SDL_SCANCODE_UP; keyupbinding[1] = SDL_SCANCODE_UNKNOWN; keyupoperator = OR;
	keydownbinding[0] = SDL_SCANCODE_DOWN; keydownbinding[1] = SDL_SCANCODE_UNKNOWN; keydownoperator = OR;
	keyleftbinding[0] = SDL_SCANCODE_LEFT; keyleftbinding[1] = SDL_SCANCODE_UNKNOWN; keyleftoperator = OR;
	keyrightbinding[0] = SDL_SCANCODE_RIGHT; keyrightbinding[1] = SDL_SCANCODE_UNKNOWN; keyrightoperator = OR;
	keylookupleftbinding[0] = SDL_SCANCODE_UP; keylookupleftbinding[1] = SDL_SCANCODE_LEFT; keylookupleftoperator = AND;
	keylookuprightbinding[0] = SDL_SCANCODE_UP; keylookuprightbinding[1] = SDL_SCANCODE_RIGHT; keylookuprightoperator = AND;
	keylookdownleftbinding[0] = SDL_SCANCODE_DOWN; keylookdownleftbinding[1] = SDL_SCANCODE_LEFT; keylookdownleftoperator = AND;
	keylookdownrightbinding[0] = SDL_SCANCODE_DOWN; keylookdownrightbinding[1] = SDL_SCANCODE_RIGHT; keylookdownrightoperator = AND;
	keynextinvbinding[0] = SDL_SCANCODE_LALT; keynextinvbinding[1] = SDL_SCANCODE_UNKNOWN; keynextinvoperator = OR;
	keynextcambinding[0] = SDL_SCANCODE_KP_6; keynextcambinding[1] = SDL_SCANCODE_KP_8; keynextcamoperator = OR;
	keyprevcambinding[0] = SDL_SCANCODE_KP_4; keyprevcambinding[1] = SDL_SCANCODE_KP_2; keyprevcamoperator = OR;
	keydetonatebinding[0] = SDL_SCANCODE_ESCAPE; keydetonatebinding[1] = (SDL_Scancode)107; keydetonateoperator = OR;
	keyjumpbinding[0] = SDL_SCANCODE_RETURN; keyjumpbinding[1] = SDL_SCANCODE_UNKNOWN; keyjumpoperator = OR;
	keyjetpackbinding[0] = (SDL_Scancode)99; keyjetpackbinding[1] = SDL_SCANCODE_UNKNOWN; keyjetpackoperator = OR;
	keyactivatebinding[0] = (SDL_Scancode)100; keyactivatebinding[1] = SDL_SCANCODE_UNKNOWN; keyactivateoperator = OR;
	keyusebinding[0] = SDL_SCANCODE_RALT; keyusebinding[1] = SDL_SCANCODE_UNKNOWN; keyuseoperator = OR;
	keyfirebinding[0] = (SDL_Scancode)103; keyfirebinding[1] = SDL_SCANCODE_UNKNOWN; keyfireoperator = OR;
	keychatbinding[0] = SDL_SCANCODE_UNKNOWN; keychatbinding[1] = SDL_SCANCODE_UNKNOWN; keychatoperator = OR;
	keydisguisebinding[0] = (SDL_Scancode)106; keydisguisebinding[1] = SDL_SCANCODE_UNKNOWN; keydisguiseoperator = OR;
	keynextweaponbinding[0] = (SDL_Scancode)102; keynextweaponbinding[1] = SDL_SCANCODE_UNKNOWN; keynextweaponoperator = OR;
#else
	keymoveupbinding[0] = SDL_SCANCODE_UP; keymoveupbinding[1] = SDL_SCANCODE_UNKNOWN; keymoveupoperator = OR;
	keymovedownbinding[0] = SDL_SCANCODE_DOWN; keymovedownbinding[1] = SDL_SCANCODE_UNKNOWN; keymovedownoperator = OR;
	keymoveleftbinding[0] = SDL_SCANCODE_LEFT; keymoveleftbinding[1] = SDL_SCANCODE_UNKNOWN; keymoveleftoperator = OR;
	keymoverightbinding[0] = SDL_SCANCODE_RIGHT; keymoverightbinding[1] = SDL_SCANCODE_UNKNOWN; keymoverightoperator = OR;
	keyupbinding[0] = SDL_SCANCODE_UP; keyupbinding[1] = SDL_SCANCODE_UNKNOWN; keyupoperator = OR;
	keydownbinding[0] = SDL_SCANCODE_DOWN; keydownbinding[1] = SDL_SCANCODE_UNKNOWN; keydownoperator = OR;
	keyleftbinding[0] = SDL_SCANCODE_LEFT; keyleftbinding[1] = SDL_SCANCODE_UNKNOWN; keyleftoperator = OR;
	keyrightbinding[0] = SDL_SCANCODE_RIGHT; keyrightbinding[1] = SDL_SCANCODE_UNKNOWN; keyrightoperator = OR;
	keylookupleftbinding[0] = SDL_SCANCODE_UP; keylookupleftbinding[1] = SDL_SCANCODE_LEFT; keylookupleftoperator = AND;
	keylookuprightbinding[0] = SDL_SCANCODE_UP; keylookuprightbinding[1] = SDL_SCANCODE_RIGHT; keylookuprightoperator = AND;
	keylookdownleftbinding[0] = SDL_SCANCODE_DOWN; keylookdownleftbinding[1] = SDL_SCANCODE_LEFT; keylookdownleftoperator = AND;
	keylookdownrightbinding[0] = SDL_SCANCODE_DOWN; keylookdownrightbinding[1] = SDL_SCANCODE_RIGHT; keylookdownrightoperator = AND;
	keynextinvbinding[0] = SDL_SCANCODE_R; keynextinvbinding[1] = SDL_SCANCODE_UNKNOWN; keynextinvoperator = OR;
	keynextcambinding[0] = SDL_SCANCODE_S; keynextcambinding[1] = SDL_SCANCODE_UNKNOWN; keynextcamoperator = OR;
	keyprevcambinding[0] = SDL_SCANCODE_A; keyprevcambinding[1] = SDL_SCANCODE_UNKNOWN; keyprevcamoperator = OR;
	keydetonatebinding[0] = SDL_SCANCODE_D; keydetonatebinding[1] = SDL_SCANCODE_UNKNOWN; keydetonateoperator = OR;
	keyjumpbinding[0] = SDL_SCANCODE_TAB; keyjumpbinding[1] = SDL_SCANCODE_UNKNOWN; keyjumpoperator = OR;
	keyjetpackbinding[0] = SDL_SCANCODE_Q; keyjetpackbinding[1] = SDL_SCANCODE_UNKNOWN; keyjetpackoperator = OR;
	keyactivatebinding[0] = SDL_SCANCODE_SPACE; keyactivatebinding[1] = SDL_SCANCODE_UNKNOWN; keyactivateoperator = OR;
	keyusebinding[0] = SDL_SCANCODE_W; keyusebinding[1] = SDL_SCANCODE_UNKNOWN; keyuseoperator = OR;
	keyfirebinding[0] = SDL_SCANCODE_E; keyfirebinding[1] = SDL_SCANCODE_UNKNOWN; keyfireoperator = OR;
	keychatbinding[0] = SDL_SCANCODE_T; keychatbinding[1] = SDL_SCANCODE_UNKNOWN; keychatoperator = OR;
	keydisguisebinding[0] = SDL_SCANCODE_C; keydisguisebinding[1] = SDL_SCANCODE_UNKNOWN; keydisguiseoperator = OR;
	keynextweaponbinding[0] = SDL_SCANCODE_UNKNOWN; keynextweaponbinding[1] = SDL_SCANCODE_UNKNOWN; keynextweaponoperator = OR;
#endif
}

bool Config::KeyIsPressed(const Uint8 * keyboardstate, SDL_Scancode keybindings[2], bool keyoperator){
	if(keyoperator == OR || (keybindings[0] == SDL_SCANCODE_UNKNOWN || keybindings[1] == SDL_SCANCODE_UNKNOWN)){
		return keyboardstate[keybindings[0]] || keyboardstate[keybindings[1]] ? true : false;
	}else{
		return keyboardstate[keybindings[0]] && keyboardstate[keybindings[1]] ? true : false;
	}
}

bool Config::CompareString(const char * str1, const char * str2){
	if(strcmp(str1, str2) == 0){
		return true;
	}
	return false;
}

void Config::WriteKey(SDL_RWops * file, const char * variable, SDL_Scancode keybindings[2], bool keyoperator){
	char line[256];
	sprintf(line, "%s = %d %d %d\r\n", variable, keybindings[0], keyoperator, keybindings[1]);
	SDL_RWwrite(file, line, strlen(line), 1);
}

void Config::ReadKey(char * data, SDL_Scancode (*keybindings)[2], bool * keyoperator){
	std::istringstream stream(data);
	unsigned int key1 = 0;
	unsigned int keyop = false;
	unsigned int key2 = 0;
	while(stream.peek() == ' ' || stream.peek() == '\t'){ stream.ignore(); }
	stream >> key1;
	while(stream.peek() == ' ' || stream.peek() == '\t'){ stream.ignore(); }
	stream >> keyop;
	while(stream.peek() == ' ' || stream.peek() == '\t'){ stream.ignore(); }
	stream >> key2;
	(*keybindings)[0] = (SDL_Scancode)key1;
	if(keyop){
		*keyoperator = true;
	}else{
		*keyoperator = false;
	}
	(*keybindings)[1] = (SDL_Scancode)key2;
}

void Config::WriteString(SDL_RWops * file, const char * variable, const char * string){
	char line[256];
	sprintf(line, "%s = %s\r\n", variable, string);
	SDL_RWwrite(file, line, strlen(line), 1);
}

void Config::ReadString(const char * data, char * variable, int length){
	memset(variable, 0, length);
	bool datafound = false;
	for(int i = 0, j = 0; i < strlen(data); i++){
		if(!datafound && (data[i] == ' ' || data[i] == '\t' || data[i] == '\r')){
			
		}else{
			datafound = true;
			if(j < length){
				variable[j++] = data[i];
			}
		}
	}
	datafound = false;
	for(int i = strlen(data), j = strlen(variable); i > 0; i--){
		if(!datafound && (data[i] == ' ' || data[i] == '\t' || data[i] == '\r' || data[i] == 0)){
			variable[j--] = 0;
		}else{
			datafound = true;
			if(j >= 0){
				variable[j--] = data[i];
			}
		}
	}
}

char * Config::RWgets(SDL_RWops * file, char * buffer, int count){
	int i;
	buffer[count - 1] = '\0';
	for(i = 0; i < count - 1; i++){
		if(SDL_RWread(file, buffer + i, 1, 1) != 1){
			if(i == 0){
				return NULL;
			}
			break;
		}
		if(buffer[i] == '\n'){
			break;
		}
	}
	buffer[i] = '\0';
	return buffer;
}