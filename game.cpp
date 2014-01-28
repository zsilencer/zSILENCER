#include "game.h"
#include <math.h>
#include "overlay.h"
#include "interface.h"
#include "textbox.h"
#include "textinput.h"
#include "button.h"
#include "toggle.h"
#include "state.h"
#include "selectbox.h"
#include "scrollbar.h"
#include "team.h"
#include "player.h"
#include "playerai.h"
#include "terminal.h"
#include "config.h"
#include "cocoawrapper.h"

Game::Game() : renderer(world), screenbuffer(640, 480){
	world.SetVersion("00021");
	frames = 0;
	fps = 0;
	state = MAINMENU;
	stateisnew = true;
	fade_i = 0;
	lobbyinterface = 0;
	gamecreateinterface = 0;
	gamejoininterface = 0;
	gametechinterface = 0;
	gameselectinterface = 0;
	modalinterface = 0;
	sharedstate = 0;
	joininggame = false;
	keynames[0] = "Move Up";
	keynames[1] = "Move Down";
	keynames[2] = "Move Left";
	keynames[3] = "Move Right";
	keynames[4] = "Aim Up/Left";
	keynames[5] = "Aim Up/Right";
	keynames[6] = "Aim Down/Left";
	keynames[7] = "Aim Down/Right";
	keynames[8] = "Jump";
	keynames[9] = "Jetpack";
	keynames[10] = "Activate/Hack";
	keynames[11] = "Use Inventory";
	keynames[12] = "Fire";
	keynames[13] = "Chat";
	keynames[14] = "Next Inventory";
	keynames[15] = "Next Camera";
	keynames[16] = "Previous Camera";
	keynames[17] = "Detonate";
	keynames[18] = "Disguise";
	keynames[19] = "Next Weapon";
	memset(keystate, 0, sizeof(keystate));
	singleplayermessage = 0;
	updatetitle = true;
	oldselectedagency = -1;
	agencychanged = true;
	currentinterface = 0;
	lastchannel[0] = 0;
	minimized = false;
	modaldialoghasok = false;
	window = 0;
	windowrenderer = 0;
	streamingtexture = 0;
	streamingtexturepixelformat = 0;
	glcontext = 0;
	fbo = 0;
	gltextures[0] = 0;
	gltextures[1] = 0;
	lasttick = 0;
	usingopengl = true;
	sdlscreenbuffer = 0;
	oldambiencelevel = 0;
	nextstateprocessed = false;
#ifdef OUYA
	quitscancode = SDL_SCANCODE_HOME;
#else
	quitscancode = SDL_SCANCODE_ESCAPE;
#endif
}

Game::~Game(){
	if(glcontext){
		SDL_GL_DeleteContext(glcontext);
	}
	if(sdlscreenbuffer){
		SDL_FreeSurface(sdlscreenbuffer);
	}
	if(usingopengl){
		SDL_GL_UnloadLibrary();
	}
	if(windowrenderer){
		SDL_DestroyRenderer(windowrenderer);
	}
	if(streamingtexture){
		SDL_DestroyTexture(streamingtexture);
	}
	if(streamingtexturepixelformat){
		SDL_FreeFormat(streamingtexturepixelformat);
	}
	if(window){
		SDL_DestroyWindow(window);
	}
	world.resources.UnloadSounds();
	Audio::GetInstance().Close();
	Mix_Quit();
	SDL_Quit();
}

bool Game::Load(char * cmdline){
	if((cmdline = strtok(cmdline, " "))){
		do{
			if(strncmp(cmdline, "-s", 2) == 0){
				char * lobbyaddress = strtok(NULL, " ");
				char * lobbyport = strtok(NULL, " ");
				char * gameid = strtok(NULL, " ");
				char * accountid = strtok(NULL, " ");
				if(gameid && accountid && lobbyaddress && lobbyport){
					world.Listen();
					world.lobby.Connect(lobbyaddress, atoi(lobbyport));
					do{
						world.lobby.DoNetwork();
						if(world.lobby.state == Lobby::RESOLVED){
							world.lobby.Connect(lobbyaddress, atoi(lobbyport));
						}
						if(world.lobby.state == Lobby::DISCONNECTED || world.lobby.state == Lobby::CONNECTIONFAILED || world.lobby.state == Lobby::RESOLVEFAILED){
							return false;
						}
						SDL_Delay(1);
					}while(world.lobby.state != Lobby::CONNECTED);
					world.gameplaystate = World::INLOBBY;
					/*User * user;
					do{
						user = world.lobby.GetUserInfo(2);
						world.lobby.DoNetwork();
					}while(user->retrieving);
					printf("name: %s, techslots: %d\n", user->name, user->agency[0].techslots);*/
					world.dedicatedserver.Start(lobbyaddress, atoi(lobbyport), atoi(gameid), atoi(accountid));
					char filename[256];
					sprintf(filename, "replays/%d.zsr", atoi(gameid));
					world.replay.BeginRecording(filename);
					if(world.replay.IsRecording()){
						world.replay.WriteHeader(world);
					}
					State * newstateobject = static_cast<State *>(world.CreateObject(ObjectTypes::STATE));
					sharedstate = newstateobject->id;
					newstateobject->state = 0;
					state = NONE;
				}
			}
		}while((cmdline = strtok(0, " ")));
	}
	Config::GetInstance().Load();
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) == -1){
		printf("Could not initialize SDL %s\n", SDL_GetError());
		return false;
	}
	int mixinitted;
	if((mixinitted = Mix_Init(MIX_INIT_MP3 | MIX_INIT_MOD | MIX_INIT_MODPLUG)) == -1){
		printf("Could not initialize SDL_mixer %s\n", Mix_GetError());
		return false;
	}
	if(!(mixinitted & MIX_INIT_MOD) && !(mixinitted & MIX_INIT_MODPLUG)){
		printf("Could not initialize MOD support %s\n", Mix_GetError());
	}
	if(!(mixinitted & MIX_INIT_MP3)){
		printf("Could not initialize MP3 support %s\n", Mix_GetError());
	}
	if(!Audio::GetInstance().Init()){
		printf("Could not initialize audio\n");
	}
	Audio::GetInstance().SetMusicVolume(64);
	printf("Loading palette...\n");
	if(!renderer.palette.SetPalette(0)){
		return false;
	}
	SDL_AddTimer(1000, TimerCallback, this);
	SDL_EventState(SDL_TEXTINPUT, SDL_TRUE); //SDL_EnableUNICODE(true);
	//SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
	//screen = SDL_SetVideoMode(640, 480, 8, SDL_DOUBLEBUF | SDL_SWSURFACE);
	window = SDL_CreateWindow("zSILENCER", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screenbuffer.w, screenbuffer.h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | (Config::GetInstance().fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
	if(!SetupOpenGL()){
		printf("Unable to setup OpenGL shaders, using SDL Renderer\n");
		usingopengl = false;
		CreateRenderer();
		sdlscreenbuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, screenbuffer.w, screenbuffer.h, 8, 0, 0, 0, 0);
		CreateStreamingTexture();
	}
	SetColors(renderer.palette.GetColors());
	//SDL_Flip(screen);
	printf("Loading resources...\n");
	if(!world.resources.Load(*this, world.dedicatedserver.active)){
		printf("Could not load resources\n");
		return false;
	}
	printf("Resources loaded\n");
	lasttick = SDL_GetTicks();
	return true;
}

bool Game::SetupOpenGL(void){
	return false;
	/*if(SDL_GL_LoadLibrary(0) == -1){
		return false;
	}
	glcontext = SDL_GL_CreateContext(window);
	if(!glcontext){
		return false;
	}
	glViewport(0, 0, screenbuffer.w, screenbuffer.h);
	//glGenFramebuffers(1, &fbo);
	if(!glCreateProgram){
		return false;
	}
	GLuint program = glCreateProgram();
	if(!program){
		return false;
	}
	
	GLuint vertshader = glCreateShader(GL_VERTEX_SHADER);
	if(!vertshader){
		return false;
	}
	const GLchar * vertshadercode = "#version 110\n"
	"varying vec2 texture_coordinate;"
	"void main(){"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
	"	texture_coordinate = vec2(gl_MultiTexCoord0);"
	"}";
	GLint shadercodelength = strlen(vertshadercode);
	if(!glShaderSource){
		return false;
	}
	glShaderSource(vertshader, 1, &vertshadercode, &shadercodelength);
	if(!glCompileShader){
		return false;
	}
	glCompileShader(vertshader);
	char buffer[1024];
	GLsizei length;
	if(!glGetShaderiv){
		return false;
	}
	glGetShaderiv(vertshader, GL_INFO_LOG_LENGTH, &length);
	if(length > 0){
		if(!glGetShaderInfoLog){
			return false;
		}
		glGetShaderInfoLog(vertshader, sizeof(buffer), &length, buffer);
		if(length > 0){
			printf("vert ShaderInfo: %s\n", buffer);
			return false;
		}
	}
	if(!glAttachShader){
		return false;
	}
	glAttachShader(program, vertshader);
	
	GLuint fragshader = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar * fragshadercode = "#version 110\n"
	"uniform sampler2D Palette;"
	"uniform sampler2D IndexedColorTexture;"
	"varying vec2 texture_coordinate;"
	"void main(){"
	"	vec4 index = texture2D(IndexedColorTexture, texture_coordinate);"
	"	vec4 texel = texture2D(Palette, vec2(index.r, 0));"
	"	gl_FragColor = texel;"
	"}";
	shadercodelength = strlen(fragshadercode);
	glShaderSource(fragshader, 1, &fragshadercode, &shadercodelength);
	glCompileShader(fragshader);
	glGetShaderiv(fragshader, GL_INFO_LOG_LENGTH, &length);
	if(length > 0){
		glGetShaderInfoLog(fragshader, sizeof(buffer), &length, buffer);
		if(length > 0){
			printf("frag ShaderInfo: %s\n", buffer);
			return false;
		}
	}
	glAttachShader(program, fragshader);
	if(!glLinkProgram){
		return false;
	}
	glLinkProgram(program);
	if(!glGetProgramiv){
		return false;
	}
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
	if(length > 0){
		if(!glGetProgramInfoLog){
			return false;
		}
		glGetProgramInfoLog(program, sizeof(buffer), &length, buffer);
		if(length > 0){
			printf("ProgramInfo: %s\n", buffer);
			return false;
		}
	}
	if(!glUseProgram){
		return false;
	}
	glUseProgram(program);
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(2, gltextures);
	
	glBindTexture(GL_TEXTURE_2D, gltextures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, renderer.palette.GetColors());
	glBindTexture(GL_TEXTURE_2D, gltextures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, 1, screenbuffer.w, screenbuffer.h, 0, GL_RED, GL_UNSIGNED_BYTE, screenbuffer.pixels);
	
	int palettesampler = glGetUniformLocation(program, "Palette");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gltextures[0]);
	if(!glUniform1i){
		return false;
	}
	glUniform1i(palettesampler, 0);
	
	int indexedtexturesampler = glGetUniformLocation(program, "IndexedColorTexture");
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, gltextures[1]);
	glUniform1i(indexedtexturesampler, 1);
	return true;*/
}

void Game::CreateRenderer(void){
	if(windowrenderer){
		SDL_DestroyRenderer(windowrenderer);
		windowrenderer = 0;
	}
	windowrenderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
}

void Game::CreateStreamingTexture(void){
	const char * scalefilter = "nearest";
	if(Config::GetInstance().scalefilter){
		scalefilter = "linear";
	}
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, scalefilter);
	if(streamingtexture){
		SDL_DestroyTexture(streamingtexture);
		streamingtexture = 0;
	}
	if(streamingtexturepixelformat){
		SDL_FreeFormat(streamingtexturepixelformat);
		streamingtexturepixelformat = 0;
	}
	SDL_RendererInfo rendererinfo;
	SDL_GetRendererInfo(windowrenderer, &rendererinfo);
	Uint32 pixelformat = 0;
	for(int i = 0; i < rendererinfo.num_texture_formats; i++){
		Uint32 format = rendererinfo.texture_formats[i];
		if(!SDL_ISPIXELFORMAT_FOURCC(format) && !SDL_ISPIXELFORMAT_INDEXED(format) && (!pixelformat || SDL_BYTESPERPIXEL(format) < SDL_BYTESPERPIXEL(pixelformat))){
			pixelformat = format;
		}
	}
	if(pixelformat){
		streamingtexture = SDL_CreateTexture(windowrenderer, pixelformat, SDL_TEXTUREACCESS_STREAMING, screenbuffer.w, screenbuffer.h);
		Uint32 format;
		SDL_QueryTexture(streamingtexture, &format, 0, 0, 0);
		streamingtexturepixelformat = SDL_AllocFormat(format);
	}
}

void Game::Present(void){
	if(usingopengl){
/*
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindTexture(GL_TEXTURE_2D, gltextures[1]);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screenbuffer.w, screenbuffer.h, GL_RED, GL_UNSIGNED_BYTE, screenbuffer.pixels);
		glLoadIdentity();
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f( -1.0f, -1.0f, 0.0f);
		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(1.0f,-1.0f, 0.0f);
		glTexCoord2f(1.0f, 0.0f);
		glVertex3f( 1.0f,1.0f, 0.0f);
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f( -1.0f,1.0f, 0.0f);
		glEnd();
		SDL_GL_SwapWindow(window);
*/
	}else{
		if(streamingtexture){
			void * pixels;
			int pitch;
			if(SDL_LockTexture(streamingtexture, 0, &pixels, &pitch) == 0){
				if(pitch == screenbuffer.w * 1){
					Uint8 * src = screenbuffer.pixels;
					Uint8 * dst = (Uint8 *)pixels;
					for(int y = screenbuffer.h; y > 0; y--){
						for(int x = screenbuffer.w; x > 0; x--){
							*(dst++) = *(Uint8 *)(&streamingtexturepalette[*src++]);
						}
					}
				}else
				if(pitch == screenbuffer.w * 2){
					Uint8 * src = screenbuffer.pixels;
					Uint16 * dst = (Uint16 *)pixels;
					for(int y = screenbuffer.h; y > 0; y--){
						for(int x = screenbuffer.w; x > 0; x--){
							*(dst++) = *(Uint16 *)(&streamingtexturepalette[*src++]);
						}
					}
				}else
				if(pitch == screenbuffer.w * 4){
					Uint8 * src = screenbuffer.pixels;
					Uint32 * dst = (Uint32 *)pixels;
					for(int y = screenbuffer.h; y > 0; y--){
						for(int x = screenbuffer.w; x > 0; x--){
							*(dst++) = *(Uint32 *)(&streamingtexturepalette[*src++]);
						}
					}
				}
				SDL_UnlockTexture(streamingtexture);
				SDL_RenderCopy(windowrenderer, streamingtexture, 0, 0);
				SDL_RenderPresent(windowrenderer);
			}
		}else{
			void * oldpixels = sdlscreenbuffer->pixels;
			sdlscreenbuffer->pixels = screenbuffer.pixels;
			SDL_Texture * texture = SDL_CreateTextureFromSurface(windowrenderer, sdlscreenbuffer);
			sdlscreenbuffer->pixels = oldpixels;
			SDL_RenderCopy(windowrenderer, texture, 0, 0);
			SDL_DestroyTexture(texture);
			SDL_RenderPresent(windowrenderer);
		}
	}
}

void Game::LoadProgressCallback(int progress, int totalprogressitems){
	HandleSDLEvents();
	if(SDL_GetTicks() - lasttick >= 100){
		int width = 500;
		int widthp = (float(progress) / totalprogressitems) * width;
		//const char * text = "Loading...";
		//renderer.DrawText(&screenbuffer, 320 - ((strlen(text) * 12) / 2), 200, text, 135, 12);
		renderer.DrawFilledRectangle(&screenbuffer, (640 - (width)) / 2, (480 - 20) / 2, (640 + (widthp)) / 2, (480 + 20) / 2, 123);
		Present();
		lasttick = SDL_GetTicks();
	}
}

void Game::SetColors(SDL_Color * colors){
	if(usingopengl){
		/*glBindTexture(GL_TEXTURE_2D, gltextures[0]);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glPixelStorei(GL_PACK_ALIGNMENT, 4);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 1, GL_RGBA, GL_UNSIGNED_BYTE, colors);*/
	}else{
		SDL_SetPaletteColors(sdlscreenbuffer->format->palette, colors, 0, 256);
		if(streamingtexture){
			for(int i = 0; i < 256; i++){
				streamingtexturepalette[i] = SDL_MapRGB(streamingtexturepixelformat, colors[i].r, colors[i].g, colors[i].b);
			}
		}
	}
}

Uint32 Game::TimerCallback(Uint32 interval, void * param){
	Game * game = static_cast<Game *>(param);
	game->updatetitle = true;
	game->fps = game->frames;
	return 1000;
}

bool Game::Loop(void){
	unsigned int wait = 42; // 24 fps
	if(updatetitle){
		char title[128];
		sprintf(title, "zSILENCER - %d FPS  Latency: %d ms  B/s: D:%d U:%d", fps, world.GetPingTime(), world.totalbytesread, world.totalbytessent);
		SDL_SetWindowTitle(window, title);
		updatetitle = false;
		frames = 1;
		world.totalbytesread = 0;
		world.totalbytessent = 0;
	}
	/*while(SDL_GetTicks() - lasttick <= wait){
		//SDL_Delay(1);
		world.DoNetwork();
	}*/
	world.DoNetwork();
	Uint32 tickcheck = SDL_GetTicks();
	while(lasttick <= tickcheck && tickcheck - lasttick > wait){
		//printf("%d\n", tickcheck - lasttick);
		world.systemcameraactive[0] = false;
		world.systemcameraactive[1] = false;
		//world.DoNetwork();
		UpdateInputState(world.localinput);
		world.SendInput();
		if(!Tick()){
			return false;
		}
		if(!world.replay.IsPlaying() || (world.replay.IsPlaying() && world.gameplaystate == World::INGAME)){
			world.Tick();
		}
		renderer.Tick();
		if(world.gameplaystate == World::INGAME){
			Uint8 newambiencelevel = renderer.GetAmbienceLevel();
			if(newambiencelevel != oldambiencelevel || fade_i <= 15){
				SDL_Color * colors = renderer.palette.GetColors();
				if(fade_i <= 15){
					colors = renderer.palette.GetTempPalette();
				}
				SDL_Color * ambiencepalette = renderer.palette.CopyWithBrightness(colors, newambiencelevel, 2, 114);
				SetColors(ambiencepalette);
				renderer.palette.CalculateLighted(newambiencelevel);
				oldambiencelevel = newambiencelevel;
			}
		}
		fade_i++;
		if(fade_i >= 16){
			fade_i = 16;
		}
		lasttick += wait;
	}
	//world.DoNetwork();
	if(!world.dedicatedserver.active){
		screenbuffer.Clear(0);
		//world.DoNetwork();
		renderer.Draw(&screenbuffer, 1 - (float(tickcheck - lasttick) / wait));
		/*char fpstext[16];
		sprintf(fpstext, "%d", fps);
		renderer.DrawText(&screenbuffer, 10, 30, fpstext, 133, 7);*/
		if(minimized){
			SDL_Delay(wait);
		}
		//world.DoNetwork();
		//Uint32 drawtick = SDL_GetTicks();
		Present();
		//Uint32 afterdrawtick = SDL_GetTicks();
		/*if(1 || afterdrawtick - drawtick > wait){
			printf("frame took %d ms to present\n", afterdrawtick - drawtick);
		}*/
	}else{
		SDL_Delay(1);
	}
	frames++;
	return true;
}

bool Game::Tick(void){
	ProcessInGameInterfaces();
	if(world.dedicatedserver.active){
		if(world.dedicatedserver.nopeerstime >= 10 * 24){
			world.dedicatedserver.SendHeartBeat(world, 2);
			return false;
		}
		if(sharedstate){
			State * sharedstateobject = static_cast<State *>(world.GetObjectFromId(sharedstate));
			if(sharedstateobject && sharedstateobject->state == 0 && world.peercount >= 1 && world.AllPeersReady(world.localpeerid) && world.AllPeersLoadedGameInfo()){
				sharedstateobject->state = 1;
				GoToState(INGAME);
			}
		}
	}
	if(!sharedstate && !world.IsAuthority()){
		//printf("no shared state!");
		for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
			Object * object = *it;
			if(object->type == ObjectTypes::STATE){
				sharedstate = object->id;
				break;
			}
		}
	}
	if(sharedstate && !world.IsAuthority()){
		State * sharedstateobject = static_cast<State *>(world.GetObjectFromId(sharedstate));
		if(sharedstateobject && sharedstateobject->state != sharedstateobject->oldstate){
			switch(sharedstateobject->state){
				case 1:
					GoToState(INGAME);
				break;
				case 2:
					if(state != INGAME){
						GoToState(INGAME);
						//state = INGAME;
						//stateisnew = true;
					}
				break;
			}
			sharedstateobject->oldstate = sharedstateobject->state;
		}
	}
	if(world.gameplaystate == World::INGAME && (state == INGAME || state == SINGLEPLAYERGAME || state == TESTGAME)){
		UpdateAmbienceChannels();
		SDL_ShowCursor(SDL_DISABLE);
	}else{
		SDL_ShowCursor(SDL_ENABLE);
	}
	switch(state){
		case FADEOUT:{
			world.intutorialmode = false;
			SDL_Color * fadedpalette = renderer.palette.CopyWithBrightness(renderer.palette.GetColors(), (15 - fade_i) * 8);
			SetColors(fadedpalette);
			if(fade_i >= 16){
				state = nextstate;
				fade_i = 0;
				stateisnew = true;
			}
		}break;
		case MAINMENU:{
			if(stateisnew){
				world.Disconnect();
				world.gameplaystate = World::NONE;
				world.lobby.Disconnect();
				UnloadGame();
				world.GetAuthorityPeer()->controlledlist.clear();
				world.DestroyAllObjects();
				currentinterface = CreateMainMenuInterface()->id;
				world.GetAuthorityPeer()->controlledlist.push_back(currentinterface);
				renderer.camera.SetPosition(320, 240);
				renderer.palette.SetPalette(1);
				screenbuffer.Clear(0);
				SetColors(renderer.palette.GetColors());
				stateisnew = false;
			}else{
				if(FadedIn()){
					Audio::GetInstance().PlayMusic(world.resources.menumusic);
				}
				Interface * iface = (Interface *)world.GetObjectFromId(currentinterface);
				if(iface){
					if(!ProcessMainMenuInterface(iface)){
						return false;
					}
				}
			}
		}break;
		case LOBBYCONNECT:{
			if(stateisnew){
				world.GetAuthorityPeer()->controlledlist.clear();
				world.DestroyAllObjects();
				world.lobby.ClearGames();
				currentinterface = CreateLobbyConnectInterface()->id;
				world.GetAuthorityPeer()->controlledlist.push_back(currentinterface);
				renderer.palette.SetPalette(2);
				world.lobby.state = Lobby::WAITING;
				motdprinted = false;
				screenbuffer.Clear(0);
				SetColors(renderer.palette.GetColors());
				stateisnew = false;
			}else{
				if(FadedIn()){
					Audio::GetInstance().PlayMusic(world.resources.menumusic);
					Interface * iface = (Interface *)world.GetObjectFromId(currentinterface);
					if(iface){
						ProcessLobbyConnectInterface(iface);
					}
				}
			}
		}break;
		case LOBBY:{
			if(stateisnew){
				world.gameplaystate = World::INLOBBY;
				agencychanged = true;
				UnloadGame();
				world.Disconnect();
				renderer.camera.SetPosition(320, 240);
				lobbyinterface = 0;
				characterinterface = 0;
				chatinterface = 0;
				gameselectinterface = 0;
				gamecreateinterface = 0;
				gamejoininterface = 0;
				gametechinterface = 0;
				gamesummaryinterface = 0;
				modalinterface = 0;
				passwordinterface = 0;
				world.choosingtech = false;
				world.lobby.channelchanged = true;
				lobbyinterface = CreateLobbyInterface()->id;
				currentinterface = lobbyinterface;
				world.GetAuthorityPeer()->controlledlist.push_back(currentinterface);
				renderer.palette.SetPalette(2);
				screenbuffer.Clear(0);
				SetColors(renderer.palette.GetColors());
				stateisnew = false;
			}else{
				if(FadedIn()){
					Audio::GetInstance().PlayMusic(world.resources.menumusic);
				}
				if(world.lobby.state == Lobby::DISCONNECTED){
					GoToState(LOBBYCONNECT);
					break;
				}
				if(lobbyinterface){
					Interface * iface = static_cast<Interface *>(world.GetObjectFromId(lobbyinterface));
					if(iface){
						ProcessLobbyInterface(iface);
					}
				}
				if(gamecreateinterface){
					if(world.lobby.creategamestatus == 1){
						world.lobby.creategamestatus = 0;
						Peer * authoritypeer = world.GetAuthorityPeer();
						LobbyGame * lobbygame = world.lobby.GetGameById(world.lobby.createdgameid);
						if(lobbygame){
							Serializer data;
							lobbygame->Serialize(Serializer::WRITE, data);
							world.gameinfo.Serialize(Serializer::READ, data);
							authoritypeer->ip = ntohl(inet_addr(lobbygame->hostname));
							//authoritypeer->ip = ntohl(inet_addr("127.0.0.1")); // temporary
							authoritypeer->port = lobbygame->port;
							sharedstate = 0;
							currentlobbygameid = lobbygame->id;
							world.Connect(GetSelectedAgency(), world.lobby.accountid, lobbygame->password);
							joininggame = true;
						}
						/*Team * team = (Team *)world.CreateObject(ObjectTypes::TEAM);
						team->AddPeer(world.GetAuthorityPeer()->id);
						team->agency = GetSelectedAgency();*/
					}else
					if(world.lobby.creategamestatus != 1 && world.lobby.creategamestatus != 100 && world.lobby.creategamestatus != 0){ // failed and not creating
						world.lobby.creategamestatus = 0;
						CreateModalDialog("Could not create game");
					}
				}
				if(gameselectinterface || gamecreateinterface){
					if(joininggame){
						if(world.state == World::CONNECTED){
							joininggame = false;
						}
						if(world.state == World::IDLE){
							joininggame = false;
							CreateModalDialog("Unable to join game");
						}
					}
					if(modalinterface && !modaldialoghasok){
						Interface * modaliface = static_cast<Interface *>(world.GetObjectFromId(modalinterface));
						if(modaliface){
							for(std::vector<Uint16>::iterator it = modaliface->objects.begin(); it != modaliface->objects.end(); it++){
								Object * object = world.GetObjectFromId(*it);
								if(object && object->type == ObjectTypes::OVERLAY){
									Overlay * overlay = static_cast<Overlay *>(object);
									if(overlay->text){
										strcpy(overlay->text, "Creating game");
										int dots = (world.tickcount / 4) % 6;
										if(dots > 3){
											dots = 6 - dots;
										}
										for(int i = 0; i < dots; i++){
											strcat(overlay->text, ".");
										}
									}
								}
							}
						}
					}
					if(!modaldialoghasok && world.lobby.creategamestatus != 100 && (world.state == World::CONNECTED || world.state == World::IDLE)){
						DestroyModalDialog();
						creategameclicked = false;
					}
					if(world.state == World::CONNECTED && lobbyinterface){
						Peer * peer = world.peerlist[world.localpeerid];
						if(peer){
							if(gameselectinterface){
								Interface * lobbyiface = static_cast<Interface *>(world.GetObjectFromId(lobbyinterface));
								if(lobbyiface){
									Interface * gameselectiface = static_cast<Interface *>(world.GetObjectFromId(gameselectinterface));
									if(gameselectiface){
										gameselectiface->DestroyInterface(world, lobbyiface);
									}
								}
								gameselectinterface = 0;
							}
							if(gamecreateinterface){
								Interface * lobbyiface = static_cast<Interface *>(world.GetObjectFromId(lobbyinterface));
								if(lobbyiface){
									Interface * gamecreateiface = static_cast<Interface *>(world.GetObjectFromId(gamecreateinterface));
									if(gamecreateiface){
										gamecreateiface->DestroyInterface(world, lobbyiface);
									}
								}
								gamecreateinterface = 0;
							}
							world.SetTech(Config::GetInstance().defaulttechchoices[GetSelectedAgency()]);
							gamejoininterface = CreateGameJoinInterface()->id;
							LobbyGame * lobbygame = world.lobby.GetGameById(currentlobbygameid);
							if(lobbygame){
								char temp[256];
								GetGameChannelName(*lobbygame, temp);
								strcpy(lastchannel, world.lobby.channel);
								world.lobby.JoinChannel(temp);
							}
							Interface * lobbyiface = static_cast<Interface *>(world.GetObjectFromId(lobbyinterface));
							if(lobbyiface){
								lobbyiface->AddObject(gamejoininterface);
							}
						}
					}
				}
				
				if(world.state != World::CONNECTED && !modalinterface){
					if(gamejoininterface || gametechinterface){
						CreateModalDialog("Disconnected from game");
					}
				}
			}
		}break;
		case INGAME:{
			if(/*!world.map.loaded && */stateisnew){
				for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
					Object * object = *it;
					switch(object->type){
						case ObjectTypes::TEAM:{
							Team * team = static_cast<Team *>(object);
							team->DestroyOverlays(world);
						}break;
						case ObjectTypes::INTERFACE:{
							Interface * iface = static_cast<Interface *>(object);
							iface->DestroyInterface(world, iface);
						}break;
					}
				}
				screenbuffer.Clear(0);
				char mapname[7 + 256];
				sprintf(mapname, "level/%s", world.gameinfo.mapname);
				if(!LoadMap(mapname)){
					world.Disconnect();
					if(world.lobby.state == Lobby::AUTHENTICATED){
						world.lobby.JoinChannel(lastchannel);
						GoToState(LOBBY);
					}else{
						GoToState(MAINMENU);
					}
					break;
				}
				State * sharedstateobject = static_cast<State *>(world.GetObjectFromId(sharedstate));
				if(sharedstateobject){
					sharedstateobject->state = 2;
				}
				if(world.replay.IsRecording()){
					world.replay.WriteStart();
				}
				ShowDeployMessage();
				Audio::GetInstance().StopMusic();
				world.gameplaystate = World::INGAME;
				for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
					Object * object = *it;
					switch(object->type){
						case ObjectTypes::TEAM:{
							Team * team = static_cast<Team *>(object);
							for(int i = 0; i < team->numpeers; i++){
								Peer * peer = world.peerlist[team->peers[i]];
								if(peer){
									world.ingameusers.push_back(peer->accountid);
									User * user = world.lobby.GetUserInfo(peer->accountid);
									if(user){
										user->statsagency = team->agency;
										user->teamnumber = team->number;
									}
									Player * player = (Player *)world.CreateObject(ObjectTypes::PLAYER);
									if(player){
										world.map.RandomPlayerStartLocation(world, player->x, player->y);
										player->oldx = player->x;
										player->oldy = player->y;
										player->teamid = team->id;
										Uint8 teamcolor = team->GetColor();
										player->suitcolor = teamcolor;//(((teamcolor >> 4) - i) << 4) + (teamcolor & 0xF);
										peer->controlledlist.clear();
										peer->controlledlist.push_back(player->id);
									}
								}
							}
						}break;
					}
				}
				world.SendPeerList();
				currentinterface = 0;
				renderer.palette.SetPalette(0);
				renderer.palette.SetParallaxColors(world.map.parallax);
				screenbuffer.Clear(0);
				SetColors(renderer.palette.GetColors());
				LoadRandomGameMusic();
				stateisnew = false;
			}else{
				if(FadedIn()){
					Audio::GetInstance().PlayMusic(world.resources.gamemusic);
				}
				if(world.replay.IsPlaying()){
					if(world.localinput.keymoveleft){
						world.localpeerid--;
					}
					if(world.localinput.keymoveright){
						world.localpeerid++;
					}
					if(!world.peerlist[world.localpeerid] || world.localpeerid == world.authoritypeer){
						for(int i = 0; i < world.maxpeers; i++){
							if(world.peerlist[i] && i != world.authoritypeer){
								world.localpeerid = i;
								break;
							}
						}
					}
					if(!world.replay.ReadToNextTick(world)){
						world.replay.EndPlaying();
						GoToState(MAINMENU);
					}
				}
				if(!deploymessageshown && world.messagetype == 1 && world.message_i == 63){
					world.ShowMessage((char *)"Location : Base Arsia Mons, Surface Temperature : -7C", 96, 1);
					deploymessageshown = true;
				}
				if(CheckForQuit()){
					world.Disconnect();
					if(world.lobby.state == Lobby::AUTHENTICATED){
						GoToState(LOBBY);
						world.lobby.JoinChannel(lastchannel);
					}else{
						if(world.replay.IsPlaying()){
							world.replay.EndPlaying();
						}
						GoToState(MAINMENU);
					}
				}
				if(CheckForEndOfGame()){
					if(world.lobby.state == Lobby::AUTHENTICATED){
						GoToState(MISSIONSUMMARY);
					}else{
						if(world.replay.IsPlaying()){
							world.replay.EndPlaying();
						}
						GoToState(MAINMENU);
					}
				}
				if(CheckForConnectionLost()){
					if(world.lobby.state == Lobby::AUTHENTICATED){
						GoToState(LOBBY);
						world.lobby.JoinChannel(lastchannel);
					}else{
						GoToState(MAINMENU);
					}
				}
			}
		}break;
		case MISSIONSUMMARY:{
			if(stateisnew){
				UnloadGame();
				gamesummaryinfoloaded = false;
				world.Disconnect();
				renderer.camera.SetPosition(320, 240);
				User * user = world.lobby.GetUserInfo(world.lobby.accountid);
				gamesummaryinterface = CreateGameSummaryInterface(user->statscopy, user->statsagency)->id;
				currentinterface = gamesummaryinterface;
				renderer.palette.SetPalette(1);
				screenbuffer.Clear(0);
				SetColors(renderer.palette.GetColors());
				stateisnew = false;
			}else{
				if(FadedIn()){
					Audio::GetInstance().PlayMusic(world.resources.menumusic);
				}
				Interface * gamesummaryiface = static_cast<Interface *>(world.GetObjectFromId(gamesummaryinterface));
				if(gamesummaryiface){
					ProcessGameSummaryInterface(gamesummaryiface);
				}
			}
		}break;
		case SINGLEPLAYERGAME:{
			if(stateisnew){
				world.GetAuthorityPeer()->controlledlist.clear();
				world.DestroyAllObjects();
				world.gameplaystate = World::INGAME;
				world.intutorialmode = true;
				currentinterface = 0;
				world.GetAuthorityPeer()->techchoices = World::BUY_LASER | World::BUY_ROCKET;
				//world.Listen(23456);
				Team * team = (Team *)world.CreateObject(ObjectTypes::TEAM);
				team->AddPeer(world.GetAuthorityPeer()->id);
				team->agency = Team::NOXIS;
				team->color = ((8 << 4) + 13);
				Player * player = (Player *)world.CreateObject(ObjectTypes::PLAYER);
				player->suitcolor = team->color;
				player->laserammo = 0;
				player->credits = 500;
				player->RemoveInventoryItem(Player::INV_BASEDOOR);
				ShowDeployMessage();
				world.GetAuthorityPeer()->controlledlist.push_back(player->id);
				world.gameinfo.securitylevel = LobbyGame::SECNONE;
				if(!LoadMap("level/ALLY10c.sil")){
					GoToState(MAINMENU);
				}
				Audio::GetInstance().StopMusic();
				world.map.RandomPlayerStartLocation(world, player->x, player->y);
				player->oldx = player->x;
				player->oldy = player->y;
				renderer.palette.SetPalette(0);
				renderer.palette.SetParallaxColors(world.map.parallax);
				screenbuffer.Clear(0);
				SetColors(renderer.palette.GetColors());
				singleplayermessage = 0;
				stateisnew = false;
				LoadRandomGameMusic();
			}
			if(FadedIn()){
				Audio::GetInstance().PlayMusic(world.resources.gamemusic);
			}
			Player * player = world.GetPeerPlayer(world.localpeerid);
			if(player){
				switch(singleplayermessage){
					case 0:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "Move your agent left and right\nBy tapping %s and %s.", GetKeyName(Config::GetInstance().keymoveleftbinding[0]), GetKeyName(Config::GetInstance().keymoverightbinding[0]));
							world.ShowMessage(text, 128);
						}
						if(player->state == Player::RUNNING){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 1:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "Make your agent jump by striking %s.", GetKeyName(Config::GetInstance().keyjumpbinding[0]));
							world.ShowMessage(text, 128);
						}
						if(player->state == Player::JUMPING){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 2:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "If you hold the %s key down, it will\nactivate your agent's jet-pack.", GetKeyName(Config::GetInstance().keyjetpackbinding[0]));
							world.ShowMessage(text, 128);
						}
						if(player->state == Player::JETPACK){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 3:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "Make your agent kneel by holding %s.", GetKeyName(Config::GetInstance().keymovedownbinding[0]));
							world.ShowMessage(text, 128);
						}
						if(player->state == Player::CROUCHED){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 4:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "Make your agent roll by kneeling,\nthen striking %s or %s.", GetKeyName(Config::GetInstance().keymoveleftbinding[0]), GetKeyName(Config::GetInstance().keymoverightbinding[0]));
							world.ShowMessage(text, 128);
						}
						if(player->state == Player::ROLLING){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 5:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "To disguise as a civilian, press the %s key.", GetKeyName(Config::GetInstance().keydisguisebinding[0]));
							world.ShowMessage(text, 128);
						}
						if(player->IsDisguised()){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 6:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "To return to normal, press the %s key again.", GetKeyName(Config::GetInstance().keydisguisebinding[0]));
							world.ShowMessage(text, 128);
						}
						if(!player->IsDisguised()){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 7:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "The %s key fires your current weapon,\nthe Blaster.", GetKeyName(Config::GetInstance().keyfirebinding[0]));
							world.ShowMessage(text, 128);
						}
						if(player->state == Player::STANDINGSHOOT || player->state == Player::CROUCHEDSHOOT || player->state == Player::FALLINGSHOOT || player->state == Player::JETPACKSHOOT || player->state == Player::LADDERSHOOT){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 8:{
						if(!world.message_i){
							char text[256];
#ifdef OUYA
							sprintf(text, "To change weapons, press %s", GetKeyName(Config::GetInstance().keynextweaponbinding[0]));
#else
							sprintf(text, "To change weapons, press the 1, 2, 3, or 4 keys");
#endif
							world.ShowMessage(text, 128);
							
						}
						if(player->laserammo == 0){
							player->laserammo = 15;
						}
						if(player->currentweapon != 0){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 9:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "Good job, agent.\n\nYou have been given a base-building device.");
							world.ShowMessage(text, 128);
							player->AddInventoryItem(Player::INV_BASEDOOR);
							singleplayermessage++;
						}
					}break;
					case 10:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "Hit %s to build your base.", GetKeyName(Config::GetInstance().keyusebinding[0]));
							world.ShowMessage(text, 128);
						}
						Team * team = player->GetTeam(world);
						if(team && team->basedoorid){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 11:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "To enter your base, hit %s when your\nSilencer is at the base entrance.", GetKeyName(Config::GetInstance().keyactivatebinding[0]));
							world.ShowMessage(text, 128);
						}
						if(player->InBase(world)){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 12:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "You are now inside your agent's secret base.\nWalk right to the flashing green computer screen\nand hit %s to activate it.", GetKeyName(Config::GetInstance().keyactivatebinding[0]));
							world.ShowMessage(text, 255);
						}
						if(!player->InBase(world)){
							singleplayermessage = 11;
							world.message_i = 0;
						}
						if(player->buyinterfaceid){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 13:{
						if(!world.message_i){
							char text[256];
#ifdef OUYA
							const char * button = "O";
#else
							const char * button = "Enter";
#endif
							sprintf(text, "Use the Up and Down keys to select Rocket ammo\nand press %s to purchase.", button);
							world.ShowMessage(text, 255);
						}
						if(!player->InBase(world)){
							singleplayermessage = 11;
							world.message_i = 0;
						}
						if(player->credits < 250){
							player->credits = 250;
						}
						if(player->rocketammo > 0){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 14:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "Good, now hit %s or %s to exit the menu.", GetKeyName(Config::GetInstance().keymoveleftbinding[0]), GetKeyName(Config::GetInstance().keymoverightbinding[0]));
							world.ShowMessage(text, 128);
						}
						if(player->rocketammo > 0){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 15:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "To leave your base, walk Left through\nthe door you entered.");
							world.ShowMessage(text, 128);
						}
						if(!player->InBase(world)){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 16:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "In the playfield, you need to hack into\ndata terminals to collect information.");
							world.ShowMessage(text, 192);
						}
						if(world.message_i >= 192 - 1){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 17:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "Walk around until you see a\nflashing green data port.\nStanding in front of the data port, hit %s\nto initiate hacking.", GetKeyName(Config::GetInstance().keyactivatebinding[0]));
							world.ShowMessage(text, 255);
						}
						if(player->state == Player::HACKING && player->files >= 100){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 18:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "Return with the information to your base door,\nhitting %s to enter the base.", GetKeyName(Config::GetInstance().keyactivatebinding[0]));
							world.ShowMessage(text, 128);
						}
						if(player->InBase(world)){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 19:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "Walk to the Right, through the agency receiver\nto deliver the information to your agency.");
							world.ShowMessage(text, 128);
						}
						if(!player->InBase(world)){
							singleplayermessage = 18;
							world.message_i = 0;
						}
						if(!player->files){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 20:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "Good job, agent.\nYou're ready for the final training exercise.");
							world.ShowMessage(text, 128);
						}
						if(world.message_i >= 128 - 1){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 21:{
						if(!world.message_i){
							world.highlightsecrets = true;
							char text[256];
							sprintf(text, "This indicator shows your progress towards\ndiscovering the location of a secret.\nKeep collecting files\nuntil all stages are lit.");
							world.ShowMessage(text, 255);
						}
						Team * team = player->GetTeam(world);
						if(team && team->beamingterminalid){
							world.highlightsecrets = false;
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 22:{
						if(!world.message_i){
							world.highlightminimap = true;
							char text[256];
							sprintf(text, "The narrowing circle on your radar shows\nyour agency acquiring a lock on the secret.\nWhen the lock is completed, the\nsecret can be picked up by your team.");
							world.ShowMessage(text, 255);
						}
						Team * team = player->GetTeam(world);
						if(team && team->beamingterminalid){
							Terminal * terminal = static_cast<Terminal *>(world.GetObjectFromId(team->beamingterminalid));
							if(terminal){
								if(terminal->beamingtime > 45){
									terminal->beamingtime = 45;
								}
								if(terminal->state == Terminal::SECRETREADY){
									world.highlightminimap = false;
									singleplayermessage++;
									world.message_i = 0;
								}
							}
						}
					}break;
					case 23:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "Pick up the secret at the location shown\non your radar map");
							world.ShowMessage(text, 128);
						}
						if(player->hassecret){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 24:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "Now, you must return the secret to your base.\nIf this were a real government secret,\nyou would have limited time before\nthe government traced your location.");
							world.ShowMessage(text, 255);
						}
						if(player->InBase(world)){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 25:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "To stash the secret data, it must be brought\nto the memory bank at the\nfar right of your base.");
							world.ShowMessage(text, 128);
						}
						Team * team = player->GetTeam(world);
						if(team && team->secrets > 0){
							singleplayermessage++;
							world.message_i = 0;
						}
					}break;
					case 26:{
						if(!world.message_i){
							char text[256];
							sprintf(text, "Good job, agent.\n\nYou're ready to begin real agency missions.");
							world.ShowMessage(text, 255);
						}
						if(world.message_i == 12){
							player->state = Player::UNDEPLOYING;
							player->state_i = 0;
						}
						if(world.message_i >= 128 - 1){
							GoToState(MAINMENU);
						}
					}break;
				}
			}
			if(CheckForQuit() || CheckForEndOfGame()){
				GoToState(MAINMENU);
			}
		}break;
		case OPTIONS:{
			if(stateisnew){
				world.DestroyAllObjects();
				currentinterface = CreateOptionsInterface()->id;
				stateisnew = false;
			}
			Interface * iface = (Interface *)world.GetObjectFromId(currentinterface);
			if(iface){
				for(std::vector<Uint16>::iterator it = iface->objects.begin(); it != iface->objects.end(); it++){
					Object * object = world.GetObjectFromId(*it);
					if(object->type == ObjectTypes::BUTTON){
						Button * button = static_cast<Button *>(object);
						if(button && button->clicked){
							switch(button->uid){
								case 0:{
									GoToState(MAINMENU);
								}break;
								case 1:{
									GoToState(OPTIONSCONTROLS);
								}break;
								case 2:{
									GoToState(OPTIONSDISPLAY);
								}break;
								case 3:{
									GoToState(OPTIONSAUDIO);
								}break;
							}
							button->clicked = false;
						}
					}
				}
			}
		}break;
		case OPTIONSCONTROLS:{
			if(stateisnew){
				world.DestroyAllObjects();
				currentinterface = CreateOptionsControlsInterface()->id;
				stateisnew = false;
			}
			Interface * iface = (Interface *)world.GetObjectFromId(currentinterface);
			if(iface){
				ScrollBar * scrollbar = (ScrollBar *)world.GetObjectFromId(iface->scrollbar);
				if(!iface->disabled){
					for(int i = 0; i < 6; i++){
						SDL_Scancode * key1 = 0;
						SDL_Scancode * key2 = 0;
						bool * keyop = 0;
						IndexToConfigKey(i + scrollbar->scrollposition, &key1, &key2, &keyop);
						sprintf(keynameoverlay[i]->text, "%s:", keynames[i + scrollbar->scrollposition]);
						if(key1 && key2){
							strcpy(c1button[i]->text, GetKeyName(*key1));
							strcpy(c2button[i]->text, GetKeyName(*key2));
						}
					}
				}
				iface->buttonenter = 200;
				for(std::vector<Uint16>::iterator it = iface->objects.begin(); it != iface->objects.end(); it++){
					Object * object = world.GetObjectFromId(*it);
					if(object->type == ObjectTypes::BUTTON){
						Button * button = static_cast<Button *>(object);
						if(button){
							if(button->state == Button::ACTIVE || button->state == Button::ACTIVATING){
								if(button->uid >= 0 && button->uid < 200){
									iface->buttonenter = button->id;
								}
							}
							if(button->uid >= 150 && button->uid < 200){
								int index = button->uid - 150 + scrollbar->scrollposition;
								SDL_Scancode * key1 = 0;
								SDL_Scancode * key2 = 0;
								bool * keyop = 0;
								IndexToConfigKey(index, &key1, &key2, &keyop);
								if(keyop){
									if(*keyop == Config::OR){
										strcpy(button->text, "OR");
									}else{
										strcpy(button->text, "AND");
									}
								}
							}
							if(button->uid >= 0 && button->uid < 150){
								const int timeout = 72;
								if(iface->disabled && button->state == Button::ACTIVE && (iface->lastsym != SDL_SCANCODE_UNKNOWN || world.tickcount - optionscontrolstick > timeout)){
									int index = button->uid + scrollbar->scrollposition;
									if(button->uid >= 150){
										index -= 150;
									}
									if(button->uid >= 100){
										index -= 100;
									}
									SDL_Scancode sym = iface->lastsym;
									if(world.tickcount - optionscontrolstick > timeout){
										sym = SDL_SCANCODE_UNKNOWN;
									}
#ifndef OUYA
									if(sym == SDL_SCANCODE_ESCAPE){
										sym = SDL_SCANCODE_UNKNOWN;
									}
#endif
									strcpy(button->text, GetKeyName(sym));
									SDL_Scancode * key1 = 0;
									SDL_Scancode * key2 = 0;
									bool * keyop = 0;
									IndexToConfigKey(index, &key1, &key2, &keyop);
									if(key1 && key2 && keyop){
										if(button->uid < 100){
											*key1 = sym;
										}else{
											*key2 = sym;
										}
									}
									iface->disabled = false;
								}
							}
							if(button->clicked){
								if(button->uid >= 0 && button->uid < 150){
									strcpy(button->text, "-");
									iface->disabled = true;
									optionscontrolstick = world.tickcount;
									iface->lastsym = SDL_SCANCODE_UNKNOWN;
								}
								if(button->uid >= 150 && button->uid < 200){
									int index = button->uid - 150 + scrollbar->scrollposition;
									SDL_Scancode * key1 = 0;
									SDL_Scancode * key2 = 0;
									bool * keyop = 0;
									IndexToConfigKey(index, &key1, &key2, &keyop);
									if(keyop){
										if(*keyop == Config::OR){
											*keyop = Config::AND;
										}else{
											*keyop = Config::OR;
										}
									}
								}
								switch(button->uid){
									case 200:{
										Config::GetInstance().Save();
										GoToState(OPTIONS);
									}break;
									case 201:{
										Config::GetInstance().Load();
										GoToState(OPTIONS);
									}break;
								}
								button->clicked = false;
							}
						}
					}
				}
			}
		}break;
		case OPTIONSDISPLAY:{
			if(stateisnew){
				world.DestroyAllObjects();
				currentinterface = CreateOptionsDisplayInterface()->id;
				stateisnew = false;
			}
			Interface * iface = (Interface *)world.GetObjectFromId(currentinterface);
			if(iface){
				iface->buttonenter = 200;
				for(std::vector<Uint16>::iterator it = iface->objects.begin(); it != iface->objects.end(); it++){
					Object * object = world.GetObjectFromId(*it);
					if(object->type == ObjectTypes::OVERLAY){
						Overlay * overlay = static_cast<Overlay *>(object);
						if(overlay){
							switch(overlay->uid){
								case 20:{
									if(Config::GetInstance().fullscreen){
										overlay->res_index = 12;
									}else{
										overlay->res_index = 13;
									}
								}break;
								case 21:{
									if(Config::GetInstance().scalefilter){
										overlay->res_index = 12;
									}else{
										overlay->res_index = 13;
									}
								}break;
								case 40:{
									if(Config::GetInstance().fullscreen){
										overlay->res_index = 15;
									}else{
										overlay->res_index = 14;
									}
								}break;
								case 41:{
									if(Config::GetInstance().scalefilter){
										overlay->res_index = 15;
									}else{
										overlay->res_index = 14;
									}
								}break;
							}
						}
					}else
					if(object->type == ObjectTypes::BUTTON){
						Button * button = static_cast<Button *>(object);
						if(button){
							if(button->state == Button::ACTIVE || button->state == Button::ACTIVATING){
								if(button->uid >= 0 && button->uid < 200){
									iface->buttonenter = button->id;
								}
							}
							if(button->clicked){
								switch(button->uid){
									case 0:{ // fullscreen
										Config::GetInstance().fullscreen = Config::GetInstance().fullscreen ? false : true;
										if(Config::GetInstance().fullscreen){
											SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
										}else{
											SDL_SetWindowFullscreen(window, 0);
										}
									}break;
									case 1:{ // smooth scaling
										Config::GetInstance().scalefilter = Config::GetInstance().scalefilter ? false : true;
										CreateStreamingTexture();
									}break;
									case 200:{
										Config::GetInstance().Save();
										GoToState(OPTIONS);
									}break;
									case 201:{
										Config::GetInstance().Load();
										CreateStreamingTexture();
										if(Config::GetInstance().fullscreen){
											SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
										}else{
											SDL_SetWindowFullscreen(window, 0);
										}
										GoToState(OPTIONS);
									}break;
								}
								button->clicked = false;
							}
						}
					}
				}
			}
		}break;
		case OPTIONSAUDIO:{
			if(stateisnew){
				world.DestroyAllObjects();
				currentinterface = CreateOptionsAudioInterface()->id;
				stateisnew = false;
			}
			Interface * iface = (Interface *)world.GetObjectFromId(currentinterface);
			if(iface){
				iface->buttonenter = 200;
				for(std::vector<Uint16>::iterator it = iface->objects.begin(); it != iface->objects.end(); it++){
					Object * object = world.GetObjectFromId(*it);
					if(object->type == ObjectTypes::OVERLAY){
						Overlay * overlay = static_cast<Overlay *>(object);
						if(overlay){
							switch(overlay->uid){
								case 20:{
									if(Config::GetInstance().music){
										overlay->res_index = 12;
									}else{
										overlay->res_index = 13;
									}
								}break;
								case 40:{
									if(Config::GetInstance().music){
										overlay->res_index = 15;
									}else{
										overlay->res_index = 14;
									}
								}break;
							}
						}
					}else
						if(object->type == ObjectTypes::BUTTON){
							Button * button = static_cast<Button *>(object);
							if(button){
								if(button->state == Button::ACTIVE || button->state == Button::ACTIVATING){
									if(button->uid >= 0 && button->uid < 200){
										iface->buttonenter = button->id;
									}
								}
								if(button->clicked){
									switch(button->uid){
										case 0:{ // music
											Config::GetInstance().music = Config::GetInstance().music ? false : true;
											if(Config::GetInstance().music){
												Audio::GetInstance().ResumeMusic();
											}else{
												Audio::GetInstance().PauseMusic();
											}
										}break;
										case 200:{
											Config::GetInstance().Save();
											GoToState(OPTIONS);
										}break;
										case 201:{
											Config::GetInstance().Load();
											if(Config::GetInstance().music){
												Audio::GetInstance().ResumeMusic();
											}else{
												Audio::GetInstance().PauseMusic();
											}
											GoToState(OPTIONS);
										}break;
									}
									button->clicked = false;
								}
							}
						}
				}
			}
		}break;
		case HOSTGAME:{
			if(stateisnew){
				strcpy(world.gameinfo.mapname, "STAR72.SIL");
				world.Listen(12456);
				world.DestroyAllObjects();
				Audio::GetInstance().StopMusic();
				world.gameplaystate = World::INLOBBY;
				currentinterface = 0;
				State * newsharedstateobject = static_cast<State *>(world.CreateObject(ObjectTypes::STATE));
				sharedstate = newsharedstateobject->id;
				newsharedstateobject->state = 0;
				world.replay.BeginRecording("testrecording.zsr");
				if(world.replay.IsRecording()){
					world.replay.WriteHeader(world);
					world.replay.WriteGameInfo(world.gameinfo);
				}
				stateisnew = false;
			}
			/*if(world.tickcount % 48 == 0){
				world.SendPeerList();
			}*/
			if(!world.map.loaded && world.peercount >= 2){
				screenbuffer.Clear(0);
				if(world.replay.IsRecording()){
					world.replay.WriteStart();
				}
				char mapname[256];
				sprintf(mapname, "level/%s", world.gameinfo.mapname);
				LoadMap(mapname);
				renderer.palette.SetPalette(0);
				renderer.palette.SetParallaxColors(world.map.parallax);
				SetColors(renderer.palette.GetColors());
				State * sharedstateobject = static_cast<State *>(world.GetObjectFromId(sharedstate));
				if(sharedstateobject){
					sharedstateobject->state = 2;
				}
				//world.GetAuthorityPeer()->controlledlist.clear();
				world.gameplaystate = World::INGAME;
				for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
					Object * object = *it;
					switch(object->type){
						case ObjectTypes::TEAM:{
							Team * team = static_cast<Team *>(object);
							if(team){
								for(int i = 0; i < team->numpeers; i++){
									Player * player = (Player *)world.CreateObject(ObjectTypes::PLAYER);
									if(player){
										world.map.RandomPlayerStartLocation(world, player->x, player->y);
										player->oldx = player->x;
										player->oldy = player->y;
										player->teamid = team->id;
										//player->AddInventoryItem(Player::INV_VIRUS);
										//player->credits = 2000;
										Uint8 teamcolor = team->GetColor();
										player->suitcolor = (((teamcolor >> 4) - i) << 4) + (teamcolor & 0xF);
										//world.peerlist[team->peers[i]]->techchoices = 0xFFFFFFFF;
										world.peerlist[team->peers[i]]->controlledlist.clear();
										world.peerlist[team->peers[i]]->controlledlist.push_back(player->id);
									}
								}
							}
						}break;
					}
				}
				world.SendPeerList();
			}
		}break;
		case JOINGAME:{
			if(stateisnew){
				sharedstate = 0;
				Peer * authoritypeer = world.GetAuthorityPeer();
				authoritypeer->ip = ntohl(inet_addr("127.0.0.1"));
				authoritypeer->port = 12456;
				world.Connect(rand() % 5, rand());
				Audio::GetInstance().StopMusic();
				currentinterface = 0;
				world.DestroyAllObjects();
				stateisnew = false;
			}else{
				State * sharedstateobject = static_cast<State *>(world.GetObjectFromId(sharedstate));
				if(sharedstateobject && sharedstateobject->state == 2){
					world.gameplaystate = World::INGAME;
				}
			}
		}break;
		case TESTGAME:{
			if(stateisnew){
				world.GetAuthorityPeer()->controlledlist.clear();
				world.DestroyAllObjects();
				world.gameplaystate = World::INGAME;
				currentinterface = 0;
				Audio::GetInstance().StopMusic();
				world.GetAuthorityPeer()->techchoices = 0xffffffff;//World::BUY_LASER | World::BUY_ROCKET;
				Team * team = (Team *)world.CreateObject(ObjectTypes::TEAM);
				team->AddPeer(world.GetAuthorityPeer()->id);
				team->agency = Team::NOXIS;
				//team->color = ((8 << 4) + 13);
				Player * player = (Player *)world.CreateObject(ObjectTypes::PLAYER);
				player->suitcolor = team->GetColor();
				player->laserammo = 0;
				player->credits = 0xffff;
				player->oldx = player->x;
				player->oldy = player->y;
				world.GetAuthorityPeer()->controlledlist.push_back(player->id);
				int botnum = 0;
				for(int i = 0; i < 40; i++){
					Uint8 agency;
					do{
						agency = rand() % 5;
					}while(agency == Team::BLACKROSE);
					Peer * botpeer = world.AddBot(agency);
					if(botpeer){
						botpeer->accountid = 0xFFFFFFFF - botnum;
						Team * botteam = world.GetPeerTeam(botpeer->id);
						Player * botplayer = (Player *)world.CreateObject(ObjectTypes::PLAYER);
						botplayer->suitcolor = botteam->GetColor();
						botplayer->laserammo = 0;
						botplayer->credits = 500;
						botplayer->ai = new PlayerAI(*botplayer);
						botpeer->controlledlist.push_back(botplayer->id);
						world.map.RandomPlayerStartLocation(world, botplayer->x, botplayer->y);
						botplayer->oldx = botplayer->x;
						botplayer->oldy = botplayer->y;
						botnum++;
					}
				}
				world.gameinfo.securitylevel = LobbyGame::SECHIGH;
				LoadMap("level/ALLY10c.sil");
				for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
					if((*it)->type == ObjectTypes::PLAYER){
						Player * player = static_cast<Player *>(*it);
						world.map.RandomPlayerStartLocation(world, player->x, player->y);
					}
				}
				ShowDeployMessage();
				renderer.palette.SetPalette(0);
				renderer.palette.SetParallaxColors(world.map.parallax);
				screenbuffer.Clear(0);
				SetColors(renderer.palette.GetColors());
				singleplayermessage = 0;
				stateisnew = false;
			}else{
				/*Player * localplayer = world.GetPeerPlayer(world.authoritypeer);
				if(localplayer){
					if(localplayer->state == Player::STANDINGSHOOT){
						for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
							if((*it)->type == ObjectTypes::PLAYER){
								Player * player = static_cast<Player *>(*it);
								if(player->ai){
									//player->ai->SetTarget(world, localplayer->x, localplayer->y);
									player->hassecret = true;
									break;
								}
							}
						}
					}
				}*/
				if(CheckForQuit() || CheckForEndOfGame()){
					GoToState(MAINMENU);
				}
			}
		}break;
		case REPLAYGAME:{
			if(stateisnew){
				world.DestroyAllObjects();
				world.gameplaystate = World::INLOBBY;
				world.replay.BeginPlaying("testrecording.zsr");
				if((world.replay.IsPlaying() && !world.replay.ReadHeader(world)) || !world.replay.IsPlaying()){
					printf("Replay error\n");
					world.replay.EndPlaying();
					GoToState(MAINMENU);
				}else{
					stateisnew = false;
				}
			}else{
				while(world.replay.ReadToNextTick(world)){
					if(world.replay.GameStarted()){
						GoToState(INGAME);
						break;
					}
					world.Tick();
				}
				if(!world.replay.GameStarted()){
					world.replay.EndPlaying();
					GoToState(MAINMENU);
				}
			}
		}break;
	}
	if(fade_i < 16 && state != FADEOUT){
		// Fade IN the palette
		SDL_Color * fadedpalette = renderer.palette.CopyWithBrightness(renderer.palette.GetColors(), (fade_i) * 8);
		if(fade_i == 15){
			SetColors(renderer.palette.GetColors());
		}else{
			SetColors(fadedpalette);
		}
	}
	if(!nextstateprocessed){
		nextstateprocessed = true;
		return Tick();
	}
	return true;
}

void Game::UpdateInputState(Input & input){
	int mousex;
	int mousey;
	Uint8 mousestate = SDL_GetMouseState(&mousex, &mousey);
	input.keymoveup = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keymoveupbinding, Config::GetInstance().keymoveupoperator);
	input.keymovedown = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keymovedownbinding, Config::GetInstance().keymovedownoperator);
	input.keymoveleft = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keymoveleftbinding, Config::GetInstance().keymoveleftoperator);
	input.keymoveright = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keymoverightbinding, Config::GetInstance().keymoverightoperator);
	input.keyup = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keyupbinding, Config::GetInstance().keyupoperator);
	input.keydown = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keydownbinding, Config::GetInstance().keydownoperator);
	input.keyleft = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keyleftbinding, Config::GetInstance().keyleftoperator);
	input.keyright = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keyrightbinding, Config::GetInstance().keyrightoperator);
	input.keylookupleft = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keylookupleftbinding, Config::GetInstance().keylookupleftoperator);
	input.keylookupright = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keylookuprightbinding, Config::GetInstance().keylookuprightoperator);
	input.keylookdownleft = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keylookdownleftbinding, Config::GetInstance().keylookdownleftoperator);
	input.keylookdownright = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keylookdownrightbinding, Config::GetInstance().keylookdownrightoperator);
	input.keynextinv = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keynextinvbinding, Config::GetInstance().keynextinvoperator);
	input.keynextcam = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keynextcambinding, Config::GetInstance().keynextcamoperator);
	input.keyprevcam = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keyprevcambinding, Config::GetInstance().keyprevcamoperator);
	input.keydetonate = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keydetonatebinding, Config::GetInstance().keydetonateoperator);
	input.keyjump = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keyjumpbinding, Config::GetInstance().keyjumpoperator);
	input.keyjetpack = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keyjetpackbinding, Config::GetInstance().keyjetpackoperator);
	input.keyactivate = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keyactivatebinding, Config::GetInstance().keyactivateoperator);
	input.keyuse = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keyusebinding, Config::GetInstance().keyuseoperator);
	input.keyfire = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keyfirebinding, Config::GetInstance().keyfireoperator);
	input.keychat = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keychatbinding, Config::GetInstance().keychatoperator);
	input.keydisguise = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keydisguisebinding, Config::GetInstance().keydisguiseoperator);
	input.keyweapon[0] = keystate[SDL_SCANCODE_1] ? true : false;
	input.keyweapon[1] = keystate[SDL_SCANCODE_2] ? true : false;
	input.keyweapon[2] = keystate[SDL_SCANCODE_3] ? true : false;
	input.keyweapon[3] = keystate[SDL_SCANCODE_4] ? true : false;
	input.keynextweapon = Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keynextweaponbinding, Config::GetInstance().keynextweaponoperator);
	input.mousex = (Uint16)mousex;
	input.mousey = (Uint16)mousey;
	input.mousedown = SDL_BUTTON_LEFT & mousestate ? true : false;
	
	Player * localplayer = world.GetPeerPlayer(world.localpeerid);
	if(localplayer){
		if(input.keynextweapon && !localplayer->input.keynextweapon){
			switch(localplayer->currentweapon){
				case 0:
					if(localplayer->laserammo > 0){
						input.keyweapon[1] = true;
					}else
					if(localplayer->rocketammo > 0){
						input.keyweapon[2] = true;
					}else
					if(localplayer->flamerammo > 0){
						input.keyweapon[3] = true;
					}
				break;
				case 1:
					if(localplayer->rocketammo > 0){
						input.keyweapon[2] = true;
					}else
					if(localplayer->flamerammo > 0){
						input.keyweapon[3] = true;
					}else{
						input.keyweapon[0] = true;
					}
				break;
				case 2:
					if(localplayer->flamerammo > 0){
						input.keyweapon[3] = true;
					}else{
						input.keyweapon[0] = true;
					}
				break;
				case 3:
					input.keyweapon[0] = true;
				break;
			}
		}
		if(localplayer->chatinterfaceid){
			Input zeroinput;
			input = zeroinput;
		}
		if(localplayer->buyinterfaceid || localplayer->techinterfaceid){
			Input zeroinput;
			zeroinput.keyactivate = input.keyactivate;
			zeroinput.keymoveleft = input.keymoveleft;
			zeroinput.keymoveright = input.keymoveright;
			input = zeroinput;
		}
	}
}

bool Game::LoadMap(const char * name){
	if(!world.map.Load(name, world)){
		return false;
	}
	CreateAmbienceChannels();
	renderer.palette.SetParallaxColors(world.map.parallax);
	return true;
}

void Game::UnloadGame(void){
	Audio::GetInstance().StopAll(200);
	for(int i = 0; i < sizeof(bgchannel) / sizeof(int); i++){
		bgchannel[i] = -1;
	}
	world.SwitchToLocalAuthorityMode();
	if(world.map.loaded){ // if the map was loaded, then the music was played
		Audio::GetInstance().StopMusic();
	}
	world.map.Unload();
	world.message_i = 0;
	world.winningteamid = 0;
	world.DestroyAllObjects();
	world.chatlines.clear();
	world.messagetype = 0;
	world.highlightminimap = false;
	world.highlightsecrets = false;
	world.quitstate = 0;
	world.ingameusers.clear();
}

bool Game::CheckForQuit(void){
	if(keystate[SDL_SCANCODE_RETURN]){
		if(world.quitstate == 1 || world.quitstate == 2){
			world.quitstate = 0;
			return true;
		}
	}
	return false;
}

bool Game::CheckForEndOfGame(void){
	if(world.winningteamid){
		if(world.message_i == 24 * 3){
			if(world.IsAuthority()){
				if(world.replay.IsRecording()){
					world.replay.EndRecording();
				}
				for(std::vector<Uint32>::iterator it = world.ingameusers.begin(); it != world.ingameusers.end(); it++){
					Uint32 accountid = *it;
					User * user = world.lobby.GetUserInfo(accountid);
					if(user){
						Uint8 won = 0;
						for(int i = 0; i < world.maxpeers; i++){
							Peer * peer = world.peerlist[i];
							if(peer){
								if(peer->accountid == user->accountid){
									Team * team = world.GetPeerTeam(peer->id);
									user->statscopy = peer->stats;
									user->statsagency = team->agency;
									user->teamnumber = team->number;
									world.SendStats(*peer);
									if(team && team->id == world.winningteamid){
										won = 1;
									}
								}
							}
						}
						world.lobby.RegisterStats(*user, won, world.gameinfo.id);
					}
				}
			}
		}
		if(world.message_i >= 240){
			return true;
		}
	}
	return false;
}

bool Game::CheckForConnectionLost(void){
	if(world.replay.IsPlaying()){
		return false;
	}
	if(world.state == World::IDLE && world.message_i >= 48){
		return true;
	}
	return false;
}

void Game::ProcessInGameInterfaces(void){
	Player * localplayer = world.GetPeerPlayer(world.localpeerid);
	if(localplayer){
		if(localplayer->buyinterfaceid || localplayer->techinterfaceid){
			currentinterface = localplayer->buyinterfaceid;
			if(!currentinterface){
				currentinterface = localplayer->techinterfaceid;
			}
		}else{
			oldselecteditem = 0;
		}
		if(localplayer->chatinterfaceid){
			currentinterface = localplayer->chatinterfaceid;
		}
		if(!localplayer->chatinterfaceid && !localplayer->buyinterfaceid && !localplayer->techinterfaceid){
			currentinterface = 0;
		}
		Interface * iface = (Interface *)world.GetObjectFromId(currentinterface);
		if(iface){
			if(iface->id == localplayer->chatinterfaceid){
				TextInput * textinput = (TextInput *)iface->GetObjectWithUid(world, 1);
				if(textinput){
					if(textinput->tabpressed){
						localplayer->chatwithteam = !localplayer->chatwithteam;
						textinput->tabpressed = false;
					}
					if(textinput->enterpressed){
						if(strlen(textinput->text) > 0){
							world.SendChat(localplayer->chatwithteam, textinput->text);
						}
						iface->DestroyInterface(world, iface);
						localplayer->chatinterfaceid = 0;
					}
					if(keystate[quitscancode]){
						iface->DestroyInterface(world, iface);
						localplayer->chatinterfaceid = 0;
					}
				}
			}else
			if(iface->id == localplayer->buyinterfaceid || iface->id == localplayer->techinterfaceid){
				bool buying = false;
				if(iface->id == localplayer->buyinterfaceid){
					buying = true;
				}
				SelectBox * selectbox = (SelectBox *)iface->GetObjectWithUid(world, 1);
				if(selectbox){
					if(selectbox->selecteditem != oldselecteditem){
						Audio::GetInstance().Play(world.resources.soundbank["grndown.wav"], 64);
						oldselecteditem = selectbox->selecteditem;
						if(buying){
							localplayer->buyifacelastitem = selectbox->selecteditem;
						}
					}
					if(selectbox->selecteditem >= selectbox->scrolled + 5){
						selectbox->scrolled = selectbox->selecteditem - 4;
					}
					if(selectbox->selecteditem < selectbox->scrolled){
						selectbox->scrolled = selectbox->selecteditem;
					}
					if(buying){
						localplayer->buyifacelastscrolled = selectbox->scrolled;
					}
					if(selectbox->enterpressed){
						BuyableItem * buyableitem = 0;
						for(std::vector<BuyableItem *>::iterator it = world.buyableitems.begin(); it != world.buyableitems.end(); it++){
							if((*it)->id == selectbox->IndexToId(selectbox->selecteditem)){
								buyableitem = (*it);
								break;
							}
						}
						if(buyableitem){
							if(buying){
								localplayer->BuyItem(world, buyableitem->id);
							}else{
								if(localplayer->InOwnBase(world)){
									localplayer->RepairItem(world, buyableitem->id);
								}else{
									localplayer->VirusItem(world, buyableitem->id);
								}
							}
						}
						selectbox->enterpressed = false;
					}
				}
			}
		}
	}
}

void Game::ShowDeployMessage(void){
	world.ShowMessage((char *)"STANDBY FOR TEAM DEPLOYMENT", 64, 1);
	deploymessageshown = false;
}

void Game::JoinGame(LobbyGame & lobbygame, char * password){
	strcpy(world.mapname, lobbygame.mapname);
	Peer * peer = world.GetAuthorityPeer();
	peer->ip = ntohl(inet_addr(lobbygame.hostname));
	//peer->ip = ntohl(inet_addr("127.0.0.1")); // temporary
	peer->port = lobbygame.port;
	sharedstate = 0;
	world.mode = World::REPLICA;
	world.Connect(GetSelectedAgency(), world.lobby.accountid, password);
	joininggame = true;
}

void Game::GoToState(Uint8 newstate){
	nextstate = newstate;
	state = FADEOUT;
	fade_i = 0;
	stateisnew = true;
	nextstateprocessed = false;
}

Interface * Game::CreateMainMenuInterface(void){
	Overlay * background = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	background->res_bank = 6;
	background->res_index = 0;
	Overlay * logo = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	logo->res_bank = 208;
	Overlay * version = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	version->text = new char[256];
	sprintf(version->text, "zSILENCER v%s", world.version);
	version->textbank = 133;
	version->textwidth = 11;
	version->x = 10;
	version->y = 480 - 10 - 7;
	Button * startbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	startbutton->y = -134;
	startbutton->x = 40;
	startbutton->uid = 0;
	strcpy(startbutton->text, "Tutorial");
	Button * lobbybutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	lobbybutton->y = -67;
	lobbybutton->x = 80;
	lobbybutton->uid = 1;
	strcpy(lobbybutton->text, "Connect To Lobby");
	Button * optionsbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	strcpy(optionsbutton->text, "Options");
	optionsbutton->x = 40;
	optionsbutton->uid = 2;
	Button * exitbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	strcpy(exitbutton->text, "Exit");
	exitbutton->x = 0;
	exitbutton->y = 67;
	exitbutton->uid = 3;
	
	Interface * iface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
	iface->AddObject(startbutton->id);
	iface->AddObject(lobbybutton->id);
	if(0){
		Button * hostbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
		hostbutton->y = -270;
		hostbutton->x = -240;
		hostbutton->uid = 4;
		strcpy(hostbutton->text, "Host Game");
		
		Button * joinbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
		joinbutton->y = -230;
		joinbutton->x = -240;
		joinbutton->uid = 5;
		strcpy(joinbutton->text, "Join Game");
		
		Button * replaybutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
		replaybutton->y = -270;
		replaybutton->x = -40;
		replaybutton->uid = 7;
		strcpy(replaybutton->text, "Test Replay");
		
		Button * testbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
		testbutton->y = -201;
		testbutton->x = 0;
		testbutton->uid = 6;
		strcpy(testbutton->text, "Test");
		
		iface->AddObject(hostbutton->id);
		iface->AddObject(joinbutton->id);
		iface->AddObject(replaybutton->id);
		iface->AddObject(testbutton->id);
		iface->AddTabObject(testbutton->id);
	}
	iface->AddObject(optionsbutton->id);
	iface->AddObject(exitbutton->id);
	iface->AddTabObject(startbutton->id);
	iface->AddTabObject(lobbybutton->id);
	iface->AddTabObject(optionsbutton->id);
	iface->AddTabObject(exitbutton->id);
	iface->activeobject = 0;
	iface->buttonescape = exitbutton->id;
	return iface;
}

Interface * Game::CreateOptionsInterface(void){
	Overlay * background = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	background->res_bank = 6;
	background->res_index = 0;
	Button * controlsbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	controlsbutton->y = -142;
	controlsbutton->x = -89;
	controlsbutton->uid = 1;
	strcpy(controlsbutton->text, "Controls");
	Button * displaybutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	displaybutton->y = -90;
	displaybutton->x = -89;
	displaybutton->uid = 2;
	strcpy(displaybutton->text, "Display");
	Button * audiobutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	audiobutton->y = -38;
	audiobutton->x = -89;
	audiobutton->uid = 3;
	strcpy(audiobutton->text, "Audio");
	Button * gobackbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	gobackbutton->y = 15;
	gobackbutton->x = -89;
	gobackbutton->uid = 0;
	strcpy(gobackbutton->text, "Go Back");
	Interface * iface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
	iface->AddObject(controlsbutton->id);
	iface->AddObject(displaybutton->id);
	iface->AddObject(audiobutton->id);
	iface->AddObject(gobackbutton->id);
	iface->AddTabObject(controlsbutton->id);
	iface->AddTabObject(displaybutton->id);
	iface->AddTabObject(audiobutton->id);
	iface->AddTabObject(gobackbutton->id);
	iface->activeobject = 0;
	iface->buttonescape = gobackbutton->id;
	return iface;
}

Interface * Game::CreateOptionsControlsInterface(void){
	Overlay * background = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	background->res_bank = 6;
	background->res_index = 0;
	Overlay * background2 = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	background2->res_bank = 7;
	background2->res_index = 7;
	Overlay * title = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	title->text = new char[256];
	strcpy(title->text, "Configure Controls");
	title->textbank = 135;
	title->textwidth = 12;
	title->x = 320 - ((strlen(title->text) * 12) / 2);
	title->y = 14;
	Interface * iface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
	for(int i = 0; i < 6; i++){
		Overlay * name = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
		keynameoverlay[i] = name;
		name->text = new char[256];
		strcpy(name->text, "");
		name->textbank = 134;
		name->textwidth = 10;
		name->x = 80;
		name->y = 95 + (i * 53);
		
		Button * c1button = (Button *)world.CreateObject(ObjectTypes::BUTTON);
		Game::c1button[i] = c1button;
		c1button->SetType(Button::B112x33);
		c1button->y = 0 + (i * 53);
		c1button->x = -30;
		c1button->uid = i;
		strcpy(c1button->text, "");
		iface->AddObject(c1button->id);
		iface->AddTabObject(c1button->id);
		
		Button * buttonop = (Button *)world.CreateObject(ObjectTypes::BUTTON);
		Game::cobutton[i] = buttonop;
		buttonop->SetType(Button::BNONE);
		buttonop->y = 95 + (i * 53);
		buttonop->x = 383;
		buttonop->uid = 150 + i;
		buttonop->width = 40;
		buttonop->height = 30;
		buttonop->textbank = 134;
		buttonop->textwidth = 9;
		strcpy(buttonop->text, "");
		iface->AddObject(buttonop->id);
		
		Button * c2button = (Button *)world.CreateObject(ObjectTypes::BUTTON);
		Game::c2button[i] = c2button;
		c2button->SetType(Button::B112x33);
		c2button->y = 0 + (i * 53);
		c2button->x = 120;
		c2button->uid = 100 + i;
		strcpy(c2button->text, "");
		iface->AddObject(c2button->id);
		iface->AddTabObject(c2button->id);
	}
	iface->objectupscroll = c1button[0]->id;
	iface->objectdownscroll = c2button[5]->id;
	
	ScrollBar * scrollbar = (ScrollBar *)world.CreateObject(ObjectTypes::SCROLLBAR);
	scrollbar->res_index = 9;
	scrollbar->scrollpixels = 53;
	scrollbar->scrollposition = 0;
	scrollbar->scrollmax = numkeys - 6;
	iface->AddObject(scrollbar->id);
	iface->scrollbar = scrollbar->id;
	
	Button * savebutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	savebutton->y = 117;
	savebutton->x = -200;
	savebutton->uid = 200;
	strcpy(savebutton->text, "Save");
	iface->AddObject(savebutton->id);
	iface->AddTabObject(savebutton->id);
	
	Button * cancelbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	cancelbutton->y = 117;
	cancelbutton->x = 20;
	cancelbutton->uid = 201;
	strcpy(cancelbutton->text, "Cancel");
	iface->AddObject(cancelbutton->id);
	iface->AddTabObject(cancelbutton->id);
	iface->activeobject = 0;
	iface->buttonenter = savebutton->id;
	iface->buttonescape = cancelbutton->id;
	return iface;
}

Interface * Game::CreateOptionsDisplayInterface(void){
	Overlay * background = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	background->res_bank = 6;
	background->res_index = 0;
	Overlay * title = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	title->text = new char[256];
	strcpy(title->text, "Display Options");
	title->textbank = 135;
	title->textwidth = 12;
	title->x = 320 - ((strlen(title->text) * 12) / 2);
	title->y = 14;
	Interface * iface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
	
	const char * names[] = {"Fullscreen", "Smooth Scaling"};
	for(int i = 0; i < 2; i++){
		Button * c1button = (Button *)world.CreateObject(ObjectTypes::BUTTON);
		c1button->SetType(Button::B220x33);
		c1button->y = 50 + (i * 53);
		c1button->x = 100;
		c1button->uid = 0 + i;
		strcpy(c1button->text, names[i]);
		iface->AddObject(c1button->id);
		iface->AddTabObject(c1button->id);
		
		Overlay * off = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
		off->y = 137 + (i * 53);
		off->x = 420;
		off->res_bank = 6;
		off->res_index = 12;
		off->uid = 20 + i;
		iface->AddObject(off->id);
		
		Overlay * on = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
		on->y = 137 + (i * 53);
		on->x = 450;
		on->res_bank = 6;
		on->res_index = 14;
		on->uid = 40 + i;
		iface->AddObject(on->id);
	}
	
	Button * savebutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	savebutton->y = 117;
	savebutton->x = -200;
	savebutton->uid = 200;
	strcpy(savebutton->text, "Save");
	iface->AddObject(savebutton->id);
	iface->AddTabObject(savebutton->id);
	
	Button * cancelbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	cancelbutton->y = 117;
	cancelbutton->x = 20;
	cancelbutton->uid = 201;
	strcpy(cancelbutton->text, "Cancel");
	iface->AddObject(cancelbutton->id);
	iface->AddTabObject(cancelbutton->id);
	iface->activeobject = 0;
	iface->buttonenter = savebutton->id;
	iface->buttonescape = cancelbutton->id;
	return iface;
}

Interface * Game::CreateOptionsAudioInterface(void){
	Overlay * background = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	background->res_bank = 6;
	background->res_index = 0;
	Overlay * title = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	title->text = new char[256];
	strcpy(title->text, "Audio Options");
	title->textbank = 135;
	title->textwidth = 12;
	title->x = 320 - ((strlen(title->text) * 12) / 2);
	title->y = 14;
	Interface * iface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
	
	const char * names[] = {"Music"};
	for(int i = 0; i < 1; i++){
		Button * c1button = (Button *)world.CreateObject(ObjectTypes::BUTTON);
		c1button->SetType(Button::B220x33);
		c1button->y = 50 + (i * 53);
		c1button->x = 100;
		c1button->uid = 0 + i;
		strcpy(c1button->text, names[i]);
		iface->AddObject(c1button->id);
		iface->AddTabObject(c1button->id);
		
		Overlay * off = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
		off->y = 137 + (i * 53);
		off->x = 420;
		off->res_bank = 6;
		off->res_index = 12;
		off->uid = 20 + i;
		iface->AddObject(off->id);
		
		Overlay * on = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
		on->y = 137 + (i * 53);
		on->x = 450;
		on->res_bank = 6;
		on->res_index = 14;
		on->uid = 40 + i;
		iface->AddObject(on->id);
	}
	
	Button * savebutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	savebutton->y = 117;
	savebutton->x = -200;
	savebutton->uid = 200;
	strcpy(savebutton->text, "Save");
	iface->AddObject(savebutton->id);
	iface->AddTabObject(savebutton->id);
	
	Button * cancelbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	cancelbutton->y = 117;
	cancelbutton->x = 20;
	cancelbutton->uid = 201;
	strcpy(cancelbutton->text, "Cancel");
	iface->AddObject(cancelbutton->id);
	iface->AddTabObject(cancelbutton->id);
	iface->activeobject = 0;
	iface->buttonenter = savebutton->id;
	iface->buttonescape = cancelbutton->id;
	return iface;
}

Interface * Game::CreateLobbyConnectInterface(void){
	Overlay * background = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	background->res_bank = 7;
	background->res_index = 2;
	Button * loginbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	loginbutton->y = 339;
	loginbutton->x = 264;
	loginbutton->SetType(Button::B52x21);
	loginbutton->uid = 0;
	strcpy(loginbutton->text, "Login");
	Button * cancelbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	cancelbutton->y = 339;
	cancelbutton->x = 321;
	cancelbutton->SetType(Button::B52x21);
	cancelbutton->uid = 1;
	strcpy(cancelbutton->text, "Cancel");
	TextBox * textbox = (TextBox *)world.CreateObject(ObjectTypes::TEXTBOX);
	textbox->x = 185;
	textbox->y = 101;
	textbox->width = 250;
	textbox->height = 170;
	textbox->res_bank = 133;
	textbox->lineheight = 11;
	textbox->fontwidth = 6;
	Overlay * usernametext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	usernametext->text = new char[9];
	strcpy(usernametext->text, "Username");
	usernametext->textbank = 134;
	usernametext->textwidth = 9;
	usernametext->x = 190;
	usernametext->y = 291;
	Overlay * passwordtext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	passwordtext->text = new char[9];
	strcpy(passwordtext->text, "Password");
	passwordtext->textbank = 134;
	passwordtext->textwidth = 9;
	passwordtext->x = 190;
	passwordtext->y = 318;
	TextInput * usernameinput = (TextInput *)world.CreateObject(ObjectTypes::TEXTINPUT);
	usernameinput->x = 275;
	usernameinput->y = 293;
	usernameinput->width = 180;
	usernameinput->height = 14;
	usernameinput->res_bank = 133;
	usernameinput->fontwidth = 6;
	usernameinput->maxchars = 16;
	usernameinput->maxwidth = 16;
	usernameinput->uid = 1;
	TextInput * passwordinput = (TextInput *)world.CreateObject(ObjectTypes::TEXTINPUT);
	passwordinput->x = 275;
	passwordinput->y = 320;
	passwordinput->width = 180;
	passwordinput->height = 14;
	passwordinput->res_bank = 133;
	passwordinput->fontwidth = 6;
	passwordinput->maxchars = 28;
	passwordinput->maxwidth = 28;
	passwordinput->password = true;
	passwordinput->uid = 2;
	Interface * iface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
#ifdef OUYA
	Overlay * helptext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	helptext->text = new char[256];
	const char * helptextstring = "Use the trackpad to click on input boxes";
	strcpy(helptext->text, helptextstring);
	helptext->textbank = 134;
	helptext->textwidth = 9;
	helptext->x = 320 - ((strlen(helptextstring) * helptext->textwidth) / 2);
	helptext->y = 400;
#endif
	
	iface->AddObject(textbox->id);
	iface->AddObject(usernameinput->id);
	iface->AddObject(passwordinput->id);
	iface->AddObject(loginbutton->id);
	iface->AddObject(cancelbutton->id);
	iface->AddTabObject(usernameinput->id);
	iface->AddTabObject(passwordinput->id);
	iface->AddTabObject(loginbutton->id);
	iface->AddTabObject(cancelbutton->id);
	iface->activeobject = usernameinput->id;
	iface->ActiveChanged(world, iface, false);
	iface->buttonenter = loginbutton->id;
	iface->buttonescape = cancelbutton->id;
	return iface;
}

Interface * Game::CreateLobbyInterface(void){
	chatlinesprinted = 0;
	Overlay * background = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	background->res_bank = 7;
	background->res_index = 1;
	Overlay * toptext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	toptext->text = new char[64];
	strcpy(toptext->text, "zSILENCER");
	toptext->textbank = 135;
	toptext->textwidth = 11;
	toptext->effectcolor = 152;
	toptext->x = 15;
	toptext->y = 32;
	Overlay * vertext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	vertext->text = new char[64];
	sprintf(vertext->text, "v.%s", world.version);
	vertext->textbank = 133;
	vertext->textwidth = 6;
	vertext->effectcolor = 189;
	vertext->x = 115;
	vertext->y = 39;
	Button * exitbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	exitbutton->y = 29;
	exitbutton->x = 473;
	exitbutton->SetType(Button::B156x21);
	exitbutton->uid = 10;
	strcpy(exitbutton->text, "Go Back");
	
	characterinterface = CreateCharacterInterface()->id;
	gameselectinterface = CreateGameSelectInterface()->id;
	chatinterface = CreateChatInterface()->id;
	
	Interface * iface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
	iface->AddObject(background->id);
	iface->AddObject(toptext->id);
	iface->AddObject(vertext->id);
	iface->AddObject(exitbutton->id);
	iface->AddObject(chatinterface);
	iface->AddObject(characterinterface);
	iface->AddObject(gameselectinterface);
	iface->buttonescape = exitbutton->id;
	iface->activeobject = chatinterface;
	iface->ActiveChanged(world, iface, false);
	return iface;
}

Interface * Game::CreateCharacterInterface(void){
	Interface * characterinterface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
	characterinterface->x = 10;
	characterinterface->y = 64;
	characterinterface->width = 217;
	characterinterface->height = 120;
	Overlay * usertext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	usertext->text = new char[64];
	strcpy(usertext->text, localusername);
	usertext->textbank = 134;
	usertext->textwidth = 8;
	usertext->effectcolor = 200;
	usertext->x = 20;
	usertext->y = 71;
	Overlay * leveltext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	leveltext->text = new char[64];
	leveltext->uid = 2;
	leveltext->textbank = 133;
	leveltext->textwidth = 7;
	leveltext->effectcolor = 129;
	leveltext->effectbrightness = 128 + 32;
	leveltext->textcolorramp = true;
	leveltext->x = 17;
	leveltext->y = 130;
	Overlay * winstext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	winstext->text = new char[64];
	winstext->uid = 3;
	winstext->textbank = 133;
	winstext->textwidth = 7;
	winstext->effectcolor = 129;
	winstext->effectbrightness = 128 + 32;
	winstext->textcolorramp = true;
	winstext->x = 17;
	winstext->y = 143;
	Overlay * lossestext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	lossestext->text = new char[64];
	lossestext->uid = 4;
	lossestext->textbank = 133;
	lossestext->textwidth = 7;
	lossestext->effectcolor = 129;
	lossestext->effectbrightness = 128 + 32;
	lossestext->textcolorramp = true;
	lossestext->x = 17;
	lossestext->y = 156;
	Overlay * etctext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	etctext->text = new char[64];
	etctext->uid = 5;
	etctext->textbank = 133;
	etctext->textwidth = 7;
	etctext->effectcolor = 129;
	etctext->effectbrightness = 128 + 32;
	etctext->textcolorramp = true;
	etctext->x = 17;
	etctext->y = 169;
	int xmargin = 42;
	Toggle * noxisbutton = (Toggle *)world.CreateObject(ObjectTypes::TOGGLE);
	noxisbutton->y = 90;
	noxisbutton->x = 20 + (0 * xmargin);
	noxisbutton->res_bank = 181;
	noxisbutton->res_index = 0;
	noxisbutton->uid = 1;
	noxisbutton->set = 1;
	if(Config::GetInstance().defaultagency == Team::NOXIS){
		noxisbutton->selected = true;
	}
	Toggle * lazarusbutton = (Toggle *)world.CreateObject(ObjectTypes::TOGGLE);
	lazarusbutton->y = 90;
	lazarusbutton->x = 20 + (1 * xmargin);
	lazarusbutton->res_bank = 181;
	lazarusbutton->res_index = 1;
	lazarusbutton->uid = 2;
	lazarusbutton->set = 1;
	if(Config::GetInstance().defaultagency == Team::LAZARUS){
		lazarusbutton->selected = true;
	}
	Toggle * caliberbutton = (Toggle *)world.CreateObject(ObjectTypes::TOGGLE);
	caliberbutton->y = 90;
	caliberbutton->x = 20 + (2 * xmargin);
	caliberbutton->res_bank = 181;
	caliberbutton->res_index = 2;
	caliberbutton->uid = 3;
	caliberbutton->set = 1;
	if(Config::GetInstance().defaultagency == Team::CALIBER){
		caliberbutton->selected = true;
	}
	Toggle * staticbutton = (Toggle *)world.CreateObject(ObjectTypes::TOGGLE);
	staticbutton->y = 90;
	staticbutton->x = 20 + (3 * xmargin);
	staticbutton->res_bank = 181;
	staticbutton->res_index = 3;
	staticbutton->uid = 4;
	staticbutton->set = 1;
	if(Config::GetInstance().defaultagency == Team::STATIC){
		staticbutton->selected = true;
	}
	Toggle * blackrosebutton = (Toggle *)world.CreateObject(ObjectTypes::TOGGLE);
	blackrosebutton->y = 90;
	blackrosebutton->x = 20 + (4 * xmargin);
	blackrosebutton->res_bank = 181;
	blackrosebutton->res_index = 4;
	blackrosebutton->uid = 5;
	blackrosebutton->set = 1;
	if(Config::GetInstance().defaultagency == Team::BLACKROSE){
		blackrosebutton->selected = true;
	}
	characterinterface->AddObject(usertext->id);
	characterinterface->AddObject(leveltext->id);
	characterinterface->AddObject(winstext->id);
	characterinterface->AddObject(lossestext->id);
	characterinterface->AddObject(etctext->id);
	characterinterface->AddObject(noxisbutton->id);
	characterinterface->AddObject(lazarusbutton->id);
	characterinterface->AddObject(caliberbutton->id);
	characterinterface->AddObject(staticbutton->id);
	characterinterface->AddObject(blackrosebutton->id);
	/*characterinterface->AddTabObject(noxisbutton->id);
	characterinterface->AddTabObject(lazarusbutton->id);
	characterinterface->AddTabObject(caliberbutton->id);
	characterinterface->AddTabObject(staticbutton->id);
	characterinterface->AddTabObject(blackrosebutton->id);*/
	return characterinterface;
}

Interface * Game::CreateGameSelectInterface(void){
	Interface * gameselectinterface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
	gameselectinterface->x = 403;
	gameselectinterface->y = 87;
	gameselectinterface->width = 222;
	gameselectinterface->height = 267;
	Overlay * rightborder = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	rightborder->res_bank = 7;
	rightborder->res_index = 8;
	Overlay * gamestext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	gamestext->text = new char[64];
	strcpy(gamestext->text, "Active Games");
	gamestext->textbank = 134;
	gamestext->textwidth = 8;
	gamestext->x = 405;
	gamestext->y = 70;
	SelectBox * gameselect = (SelectBox *)world.CreateObject(ObjectTypes::SELECTBOX);
	gameselect->x = 407;
	gameselect->y = 89;
	gameselect->width = 214;
	gameselect->height = 265;
	gameselect->lineheight = 14;
	gameselect->uid = 10;
	ScrollBar * gamescrollbar = (ScrollBar *)world.CreateObject(ObjectTypes::SCROLLBAR);
	gamescrollbar->res_index = 9;
	gamescrollbar->scrollpixels = gameselect->lineheight;
	gamescrollbar->scrollposition = gameselect->scrolled;
	Overlay * gamenametext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	gamenametext->text = new char[128];
	gamenametext->text[0] = 0;
	//strcpy(gamenametext->text, "game name");
	gamenametext->textbank = 133;
	gamenametext->textwidth = 6;
	gamenametext->x = 405;
	gamenametext->y = 358;
	gamenametext->uid = 1;
	Overlay * gamemaptext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	gamemaptext->text = new char[128];
	gamemaptext->text[0] = 0;
	//strcpy(gamemaptext->text, "game map");
	gamemaptext->textbank = 133;
	gamemaptext->textwidth = 6;
	gamemaptext->x = 405;
	gamemaptext->y = 370;
	gamemaptext->uid = 2;
	Overlay * gameplayerstext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	gameplayerstext->text = new char[128];
	gameplayerstext->text[0] = 0;
	//strcpy(gameplayerstext->text, "Players: ");
	gameplayerstext->textbank = 133;
	gameplayerstext->textwidth = 6;
	gameplayerstext->x = 405;
	gameplayerstext->y = 382;
	gameplayerstext->uid = 3;
	Overlay * gamecreatortext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	gamecreatortext->text = new char[128];
	gamecreatortext->text[0] = 0;
	//strcpy(gamecreatortext->text, "Creator: ");
	gamecreatortext->textbank = 133;
	gamecreatortext->textwidth = 6;
	gamecreatortext->x = 405;
	gamecreatortext->y = 394;
	gamecreatortext->uid = 4;
	Overlay * gameinfotext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	gameinfotext->text = new char[128];
	gameinfotext->text[0] = 0;
	//strcpy(gameinfotext->text, "Info: ");
	gameinfotext->textbank = 133;
	gameinfotext->textwidth = 6;
	gameinfotext->x = 405;
	gameinfotext->y = 406;
	gameinfotext->uid = 5;
	
	Button * gamejoinbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	gamejoinbutton->y = 430;
	gamejoinbutton->x = 436;
	gamejoinbutton->SetType(Button::B156x21);
	gamejoinbutton->uid = 20;
	strcpy(gamejoinbutton->text, "Join Game");
	Button * gamecreatebutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	gamecreatebutton->y = 68;
	gamecreatebutton->x = 242;
	gamecreatebutton->SetType(Button::B156x21);
	gamecreatebutton->uid = 30;
	strcpy(gamecreatebutton->text, "Create Game");
	gameselectinterface->AddObject(rightborder->id);
	gameselectinterface->AddObject(gamestext->id);
	gameselectinterface->AddObject(gamejoinbutton->id);
	gameselectinterface->AddObject(gamecreatebutton->id);
	gameselectinterface->AddObject(gameselect->id);
	gameselectinterface->AddObject(gamescrollbar->id);
	gameselectinterface->AddObject(gamenametext->id);
	gameselectinterface->AddObject(gamemaptext->id);
	gameselectinterface->AddObject(gameplayerstext->id);
	gameselectinterface->AddObject(gamecreatortext->id);
	gameselectinterface->AddObject(gameinfotext->id);
	gameselectinterface->buttonenter = gamejoinbutton->id;
	gameselectinterface->scrollbar = gamescrollbar->id;
	return gameselectinterface;
}

Interface * Game::CreateChatInterface(void){
	Interface * chatinterface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
	chatinterface->x = 15;
	chatinterface->y = 216;
	chatinterface->width = 368;
	chatinterface->height = 234;
	Overlay * chatborder = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	chatborder->res_bank = 7;
	chatborder->res_index = 11;
	Overlay * chatinputborder = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	chatinputborder->res_bank = 7;
	chatinputborder->res_index = 14;
	Overlay * channeltext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	channeltext->text = new char[128];
	channeltext->uid = 1;
	strcpy(channeltext->text, "");
	channeltext->textbank = 134;
	channeltext->textwidth = 8;
	channeltext->x = 15;
	channeltext->y = 200;
	TextBox * textbox = (TextBox *)world.CreateObject(ObjectTypes::TEXTBOX);
	textbox->x = 19;
	textbox->y = 220;
	textbox->width = 342;
	textbox->height = 207;
	textbox->res_bank = 133;
	textbox->lineheight = 11;
	textbox->fontwidth = 6;
	textbox->bottomtotop = true;
	/*for(int i = 0; i < 0; i++){
		char line[256];
		sprintf(line, "line %d", i);
		textbox->AddLine(line);
	}*/
	TextInput * chatinput = (TextInput *)world.CreateObject(ObjectTypes::TEXTINPUT);
	chatinput->x = 18;
	chatinput->y = 437;
	chatinput->width = 360;
	chatinput->height = 14;
	chatinput->res_bank = 133;
	chatinput->fontwidth = 6;
	chatinput->maxchars = 200;
	chatinput->maxwidth = 60;
	chatinput->uid = 1;
	ScrollBar * chatscrollbar = (ScrollBar *)world.CreateObject(ObjectTypes::SCROLLBAR);
	chatscrollbar->res_index = 12;
	chatscrollbar->barres_index = 13;
	chatscrollbar->scrollpixels = textbox->lineheight;
	chatscrollbar->scrollposition = textbox->scrolled;
	chatinterface->AddObject(chatborder->id);
	chatinterface->AddObject(chatinputborder->id);
	chatinterface->AddObject(channeltext->id);
	chatinterface->AddObject(textbox->id);
	chatinterface->AddObject(chatinput->id);
	chatinterface->AddObject(chatscrollbar->id);
	chatinterface->AddTabObject(chatinput->id);
	chatinterface->scrollbar = chatscrollbar->id;
	//chatinterface->ActiveChanged(&world, chatinterface, false);
	return chatinterface;
}

Interface * Game::CreateGameCreateInterface(void){
	creategameclicked = false;
	Interface * gamecreateinterface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
	gamecreateinterface->x = 403;
	gamecreateinterface->y = 87;
	gamecreateinterface->width = 222;
	gamecreateinterface->height = 390;
	Overlay * rightborder = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	rightborder->res_bank = 7;
	rightborder->res_index = 8;
	
	Overlay * optionstext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	optionstext->text = new char[64];
	strcpy(optionstext->text, "Game Options");
	optionstext->textbank = 134;
	optionstext->textwidth = 8;
	optionstext->x = 250;
	optionstext->y = 70;
	
	int yoffset = 6;
	
	Overlay * securitytext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	securitytext->text = new char[64];
	strcpy(securitytext->text, "Security:");
	securitytext->textbank = 134;
	securitytext->textwidth = 8;
	securitytext->x = 245;
	securitytext->y = 87 + yoffset;
	Button * buttonsecurity = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	buttonsecurity->SetType(Button::BNONE);
	buttonsecurity->x = 323;
	buttonsecurity->y = 87 + yoffset;
	buttonsecurity->uid = 40;
	buttonsecurity->width = 70;
	buttonsecurity->height = 20;
	buttonsecurity->textbank = 134;
	buttonsecurity->textwidth = 9;
	strcpy(buttonsecurity->text, "Medium");
	
	Overlay * minleveltext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	minleveltext->text = new char[64];
	strcpy(minleveltext->text, "Min Level:");
	minleveltext->textbank = 134;
	minleveltext->textwidth = 8;
	minleveltext->x = 245;
	minleveltext->y = 104 + yoffset;
	
	TextInput * minlevelinput = (TextInput *)world.CreateObject(ObjectTypes::TEXTINPUT);
	minlevelinput->x = 350;
	minlevelinput->y = 104 + yoffset;
	minlevelinput->width = 20;
	minlevelinput->height = 20;
	minlevelinput->res_bank = 134;
	minlevelinput->fontwidth = 8;
	minlevelinput->maxchars = 2;
	minlevelinput->maxwidth = 50;
	minlevelinput->uid = 41;
	minlevelinput->numbersonly = true;
	minlevelinput->SetText("0");
	
	Overlay * maxleveltext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	maxleveltext->text = new char[64];
	strcpy(maxleveltext->text, "Max Level:");
	maxleveltext->textbank = 134;
	maxleveltext->textwidth = 8;
	maxleveltext->x = 245;
	maxleveltext->y = 121 + yoffset;
	
	TextInput * maxlevelinput = (TextInput *)world.CreateObject(ObjectTypes::TEXTINPUT);
	maxlevelinput->x = 350;
	maxlevelinput->y = 121 + yoffset;
	maxlevelinput->width = 20;
	maxlevelinput->height = 20;
	maxlevelinput->res_bank = 134;
	maxlevelinput->fontwidth = 8;
	maxlevelinput->maxchars = 2;
	maxlevelinput->maxwidth = 50;
	maxlevelinput->uid = 42;
	maxlevelinput->numbersonly = true;
	maxlevelinput->SetText("99");
	
	Overlay * maxplayerstext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	maxplayerstext->text = new char[64];
	strcpy(maxplayerstext->text, "Max Players:");
	maxplayerstext->textbank = 134;
	maxplayerstext->textwidth = 8;
	maxplayerstext->x = 245;
	maxplayerstext->y = 138 + yoffset;
	
	TextInput * maxplayersinput = (TextInput *)world.CreateObject(ObjectTypes::TEXTINPUT);
	maxplayersinput->x = 350;
	maxplayersinput->y = 138 + yoffset;
	maxplayersinput->width = 20;
	maxplayersinput->height = 20;
	maxplayersinput->res_bank = 134;
	maxplayersinput->fontwidth = 8;
	maxplayersinput->maxchars = 2;
	maxplayersinput->maxwidth = 50;
	maxplayersinput->uid = 43;
	maxplayersinput->numbersonly = true;
	maxplayersinput->SetText("24");
	
	Overlay * maxteamstext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	maxteamstext->text = new char[64];
	strcpy(maxteamstext->text, "Max Teams:");
	maxteamstext->textbank = 134;
	maxteamstext->textwidth = 8;
	maxteamstext->x = 245;
	maxteamstext->y = 155 + yoffset;
	
	TextInput * maxteamsinput = (TextInput *)world.CreateObject(ObjectTypes::TEXTINPUT);
	maxteamsinput->x = 350;
	maxteamsinput->y = 155 + yoffset;
	maxteamsinput->width = 20;
	maxteamsinput->height = 20;
	maxteamsinput->res_bank = 134;
	maxteamsinput->fontwidth = 8;
	maxteamsinput->maxchars = 2;
	maxteamsinput->maxwidth = 50;
	maxteamsinput->uid = 44;
	maxteamsinput->numbersonly = true;
	maxteamsinput->SetText("6");
	
	Overlay * selectmaptext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	selectmaptext->text = new char[64];
	strcpy(selectmaptext->text, "Select Map");
	selectmaptext->textbank = 134;
	selectmaptext->textwidth = 8;
	selectmaptext->x = 405;
	selectmaptext->y = 70;
	SelectBox * mapselect = (SelectBox *)world.CreateObject(ObjectTypes::SELECTBOX);
	mapselect->x = 407;
	mapselect->y = 89;
	mapselect->width = 214;
	mapselect->height = 265;
	mapselect->lineheight = 14;
	mapselect->uid = 4;
	//mapselect->ListFiles("level");
	const char * maps[] = {"ALLY10c.sil", "CRAN01h.SIL", "EASY05c.SIL", "PIT16d.SIL", "STAR72.SIL", "THET06e.SIL"};
	for(int i = 0; i < sizeof(maps) / sizeof(const char *); i++){
		mapselect->AddItem(maps[i]);
	}
	ScrollBar * mapscrollbar = (ScrollBar *)world.CreateObject(ObjectTypes::SCROLLBAR);
	mapscrollbar->res_index = 9;
	mapscrollbar->scrollpixels = mapselect->lineheight;
	mapscrollbar->scrollposition = mapselect->scrolled;
	Overlay * nametext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	nametext->text = new char[64];
	strcpy(nametext->text, "Game name:");
	nametext->textbank = 134;
	nametext->textwidth = 8;
	nametext->x = 405;
	nametext->y = 360;
	TextInput * nametextinput = (TextInput *)world.CreateObject(ObjectTypes::TEXTINPUT);
	nametextinput->x = 410;
	nametextinput->y = 375;
	nametextinput->width = 210;
	nametextinput->height = 14;
	nametextinput->res_bank = 133;
	nametextinput->fontwidth = 6;
	nametextinput->maxchars = 35;
	nametextinput->maxwidth = 35;
	nametextinput->uid = 5;
	nametextinput->SetText(Config::GetInstance().defaultgamename);
	Overlay * passwordtext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	passwordtext->text = new char[64];
	strcpy(passwordtext->text, "Password (optional):");
	passwordtext->textbank = 134;
	passwordtext->textwidth = 8;
	passwordtext->x = 405;
	passwordtext->y = 390;
	TextInput * passwordtextinput = (TextInput *)world.CreateObject(ObjectTypes::TEXTINPUT);
	passwordtextinput->x = 410;
	passwordtextinput->y = 405;
	passwordtextinput->width = 210;
	passwordtextinput->height = 14;
	passwordtextinput->res_bank = 133;
	passwordtextinput->fontwidth = 6;
	passwordtextinput->maxchars = 20;
	passwordtextinput->maxwidth = 20;
	passwordtextinput->uid = 6;
	passwordtextinput->password = true;
	Button * gamecreatebutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	gamecreatebutton->y = 430;
	gamecreatebutton->x = 436;
	gamecreatebutton->SetType(Button::B156x21);
	gamecreatebutton->uid = 35;
	strcpy(gamecreatebutton->text, "Create");
	gamecreateinterface->AddObject(rightborder->id);
	gamecreateinterface->AddObject(optionstext->id);
	gamecreateinterface->AddObject(securitytext->id);
	gamecreateinterface->AddObject(buttonsecurity->id);
	gamecreateinterface->AddObject(minleveltext->id);
	gamecreateinterface->AddObject(minlevelinput->id);
	gamecreateinterface->AddObject(maxleveltext->id);
	gamecreateinterface->AddObject(maxlevelinput->id);
	gamecreateinterface->AddObject(maxplayerstext->id);
	gamecreateinterface->AddObject(maxplayersinput->id);
	gamecreateinterface->AddObject(maxteamstext->id);
	gamecreateinterface->AddObject(maxteamsinput->id);
	gamecreateinterface->AddObject(selectmaptext->id);
	gamecreateinterface->AddObject(mapselect->id);
	gamecreateinterface->AddObject(mapscrollbar->id);
	gamecreateinterface->AddObject(nametext->id);
	gamecreateinterface->AddObject(nametextinput->id);
	gamecreateinterface->AddObject(passwordtext->id);
	gamecreateinterface->AddObject(passwordtextinput->id);
	gamecreateinterface->AddObject(gamecreatebutton->id);
	//gamecreateinterface->AddTabObject(buttonsecurity->id);
	gamecreateinterface->AddTabObject(nametextinput->id);
	gamecreateinterface->AddTabObject(passwordtextinput->id);
	gamecreateinterface->AddTabObject(gamecreatebutton->id);
	gamecreateinterface->AddTabObject(minlevelinput->id);
	gamecreateinterface->AddTabObject(maxlevelinput->id);
	gamecreateinterface->AddTabObject(maxplayersinput->id);
	gamecreateinterface->AddTabObject(maxteamsinput->id);
	gamecreateinterface->buttonenter = gamecreatebutton->id;
	gamecreateinterface->activeobject = nametextinput->id;
	return gamecreateinterface;
}

Interface * Game::CreateGameJoinInterface(void){
	Interface * gamejoininterface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
	gamejoininterface->x = 403;
	gamejoininterface->y = 87;
	gamejoininterface->width = 222;
	gamejoininterface->height = 267;
	Button * gamestartbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	gamestartbutton->x = 242;
	gamestartbutton->y = 160;
	gamestartbutton->SetType(Button::B156x21);
	gamestartbutton->uid = 25;
	//if(ishost){
	//	strcpy(gamestartbutton->text, "Start Game");
	//}else{
		strcpy(gamestartbutton->text, "Ready");
	//}
	Button * techbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	techbutton->x = 242;
	techbutton->y = 68;
	techbutton->SetType(Button::B156x21);
	techbutton->uid = 27;
	strcpy(techbutton->text, "Choose Tech");
	Button * changeteambutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	changeteambutton->x = 242;
	changeteambutton->y = 100;
	changeteambutton->SetType(Button::B156x21);
	changeteambutton->uid = 26;
	strcpy(changeteambutton->text, "Change Team");
	gamejoininterface->AddObject(techbutton->id);
	gamejoininterface->AddObject(changeteambutton->id);
	gamejoininterface->AddObject(gamestartbutton->id);
	gamejoininterface->buttonenter = gamestartbutton->id;
	return gamejoininterface;
}

Interface * Game::CreateGameTechInterface(void){
	Interface * gametechinterface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
	gametechinterface->x = 403;
	gametechinterface->y = 87;
	gametechinterface->width = 222;
	gametechinterface->height = 390;
	Button * teamsbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	teamsbutton->x = 242;
	teamsbutton->y = 68;
	teamsbutton->SetType(Button::B156x21);
	teamsbutton->uid = 28;
	strcpy(teamsbutton->text, "Back To Teams");
	Overlay * techslotsoverlay = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	if(techslotsoverlay){
		techslotsoverlay->text = new char[64];
		techslotsoverlay->x = 455;
		techslotsoverlay->y = 100;
		techslotsoverlay->textbank = 133;
		techslotsoverlay->textwidth = 6;
		techslotsoverlay->effectcolor = 129;
		techslotsoverlay->effectbrightness = 128 + 16;
		techslotsoverlay->textcolorramp = true;
		techslotsoverlay->uid = 70;
		gametechinterface->AddObject(techslotsoverlay->id);
	}
	for(int i = 0; i < 3; i++){
		int j = 2 - i;
		Overlay * techoverlay = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
		if(techoverlay){
			techoverlay->text = new char[64];
			sprintf(techoverlay->text, "Player %d", i + 1);
			techoverlay->textbank = 133;
			techoverlay->textwidth = 6;
			techoverlay->uid = 80 + i;
			techoverlay->x = 375 - (strlen(techoverlay->text) * 6);
			techoverlay->y = 112 + (j * 16);
			techoverlay->draw = false;
			gametechinterface->AddObject(techoverlay->id);
		}
		Overlay * techlineoverlay = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
		if(techlineoverlay){
			techlineoverlay->x = 0;
			techlineoverlay->y = 0;
			techlineoverlay->uid = 90 + i;
			techlineoverlay->res_bank = 7;
			techlineoverlay->res_index = 20 + j;
			techlineoverlay->draw = false;
			gametechinterface->AddObject(techlineoverlay->id);
		}
	}
	Team * team = world.GetPeerTeam(world.localpeerid);
	if(team){
		for(int x = 0; x < 4; x++){
			int i = 0;
			int ipos = 0;
			for(std::vector<BuyableItem *>::iterator it = world.buyableitems.begin(); it != world.buyableitems.end(); it++){
				BuyableItem * buyableitem = *it;
				if(buyableitem->techslots){
					if(buyableitem->agencyspecific == -1 || (buyableitem->agencyspecific == team->agency)){
						Button * button = (Button *)world.CreateObject(ObjectTypes::BUTTON);
						if(button){
							button->x = 410 + (x * 14);
							button->y = 125 + (ipos * 13);
							button->uid = 110 + (30 * x) + i;
							button->SetType(Button::BCHECKBOX);
							if(x < 3){
								button->effectbrightness = 64;
								button->draw = false;
							}
							gametechinterface->AddObject(button->id);
							if(x == 3){
								Overlay * technameoverlay = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
								if(technameoverlay){
									technameoverlay->text = new char[64];
									sprintf(technameoverlay->text, "%s (%d)", buyableitem->name, buyableitem->techslots);
									technameoverlay->x = 425 + (x * 14);
									technameoverlay->y = 127 + (ipos * 13);
									technameoverlay->textbank = 133;
									technameoverlay->textwidth = 6;
									technameoverlay->uid = 230 + i;
									gametechinterface->AddObject(technameoverlay->id);
								}
							}
						}
						ipos++;
					}
					i++;
				}
			}
		}
	}
	Overlay * techname = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	if(techname){
		techname->text = new char[64];
		strcpy(techname->text, "");
		techname->textbank = 134;
		techname->textwidth = 8;
		techname->uid = 60;
		techname->x = 401 + (116 - ((strlen(techname->text) * techname->textwidth) / 2));
		techname->y = 350;
		gametechinterface->AddObject(techname->id);
	}
	for(int i = 0; i < 8; i++){
		Overlay * techdesc = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
		if(techdesc){
			techdesc->text = new char[64];
			strcpy(techdesc->text, "");
			techdesc->textbank = 133;
			techdesc->textwidth = 6;
			techdesc->effectcolor = 129;
			techdesc->effectbrightness = 128 + 16;
			techdesc->textcolorramp = true;
			techdesc->uid = 61 + i;
			techdesc->x = 405;
			techdesc->y = 370 + (i * 10);
			gametechinterface->AddObject(techdesc->id);
		}
	}
	gametechinterface->AddObject(teamsbutton->id);
	gametechinterface->buttonescape = teamsbutton->id;
	return gametechinterface;
}

Interface * Game::CreateGameSummaryInterface(Stats & stats, Uint8 agency){
	Overlay * background = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	background->res_bank = 6;
	background->res_index = 0;
	Overlay * background2 = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	background2->res_bank = 7;
	background2->res_index = 5;
	Overlay * title = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	title->text = new char[256];
	strcpy(title->text, "Mission Summary");
	title->textbank = 135;
	title->textwidth = 12;
	title->x = 192 - ((strlen(title->text) * 12) / 2);
	title->y = 44;
	Interface * iface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
	TextBox * textbox = (TextBox *)world.CreateObject(ObjectTypes::TEXTBOX);
	textbox->x = 89;
	textbox->y = 92;
	textbox->width = 180;
	textbox->height = 300;
	textbox->res_bank = 133;
	textbox->lineheight = 11;
	textbox->fontwidth = 6;
	
	AddSummaryLine(*textbox, "Kills:", stats.kills);
	AddSummaryLine(*textbox, "Deaths:", stats.deaths);
	AddSummaryLine(*textbox, "Suicides", stats.suicides);
	textbox->AddLine("");
	textbox->AddLine("Secrets");
	AddSummaryLine(*textbox, "  Returned:", stats.secretsreturned);
	AddSummaryLine(*textbox, "  Stolen:", stats.secretsstolen);
	AddSummaryLine(*textbox, "  Picked up:", stats.secretspickedup);
	AddSummaryLine(*textbox, "  Fumbled:", stats.secretsdropped);
	textbox->AddLine("");
	AddSummaryLine(*textbox, "Civilians killed:", stats.civilianskilled);
	AddSummaryLine(*textbox, "Guards killed:", stats.guardskilled);
	AddSummaryLine(*textbox, "Robots killed:", stats.robotskilled);
	AddSummaryLine(*textbox, "Defenses destroyed:", stats.defensekilled);
	AddSummaryLine(*textbox, "Fixed Cannons destroyed:", stats.fixedcannonsdestroyed);
	textbox->AddLine("");
	textbox->AddLine("Files");
	AddSummaryLine(*textbox, "  Hacked:", stats.fileshacked);
	AddSummaryLine(*textbox, "  Returned:", stats.filesreturned);
	textbox->AddLine("");
	AddSummaryLine(*textbox, "Powerups picked up:", stats.powerupspickedup);
	AddSummaryLine(*textbox, "Health packs used:", stats.healthpacksused);
	AddSummaryLine(*textbox, "Cameras placed:", stats.camerasplanted);
	AddSummaryLine(*textbox, "Detonators planted:", stats.detsplanted);
	AddSummaryLine(*textbox, "Fixed Cannons placed:", stats.fixedcannonsplaced);
	AddSummaryLine(*textbox, "Viruses used:", stats.virusesused);
	AddSummaryLine(*textbox, "Poisons:", stats.poisons);
	AddSummaryLine(*textbox, "Lazarus Tracts planted:", stats.tractsplanted);
	textbox->AddLine("");
	textbox->AddLine("Grenades thrown");
	AddSummaryLine(*textbox, "  E.M.P:", stats.empsthrown);
	AddSummaryLine(*textbox, "  Plasma:", stats.plasmasthrown);
	AddSummaryLine(*textbox, "  Shaped:", stats.shapedthrown);
	AddSummaryLine(*textbox, "  Flare:", stats.flaresthrown);
	AddSummaryLine(*textbox, "  Poison Flare:", stats.poisonflaresthrown);
	AddSummaryLine(*textbox, "  Neutron:", stats.neutronsthrown);
	for(int i = 0; i < 4; i++){
		textbox->AddLine("");
		switch(i){
			case 0: textbox->AddLine("Blaster"); break;
			case 1: textbox->AddLine("Laser"); break;
			case 2: textbox->AddLine("Rocket"); break;
			case 3: textbox->AddLine("Flamer"); break;
		}
		AddSummaryLine(*textbox, "  Shots fired:", stats.weaponfires[i]);
		AddSummaryLine(*textbox, "  Hits:", stats.weaponhits[i]);
		AddSummaryLine(*textbox, "  Accuracy:", (float(stats.weaponhits[i]) / stats.weaponfires[i]) * 100, true);
		AddSummaryLine(*textbox, "  Player kills:", stats.playerkillsweapon[i]);
	}
	ScrollBar * scrollbar = (ScrollBar *)world.CreateObject(ObjectTypes::SCROLLBAR);
	scrollbar->res_index = 9;
	scrollbar->scrollposition = 0;
	scrollbar->scrollmax = textbox->text.size() - (textbox->height / textbox->lineheight);
	scrollbar->scrollpixels = textbox->lineheight;
	scrollbar->scrollposition = 0;
	
	Overlay * xptext = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	xptext->text = new char[64];
	sprintf(xptext->text, "+ %d XP", stats.CalculateExperience());
	xptext->textbank = 136;
	xptext->textwidth = 15;
	xptext->x = 467 - ((strlen(xptext->text) * xptext->textwidth) / 2);
	xptext->y = 45;
	
	bool upgradeavailable = true;
	
	Overlay * line1text = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	line1text->text = new char[64];
	sprintf(line1text->text, "*NEW UPGRADE AVAILABLE*");
	line1text->textbank = 133;
	line1text->effectcolor = 129;
	line1text->effectbrightness = 128 + 32;
	line1text->textcolorramp = true;
	line1text->textwidth = 6;
	line1text->uid = 1;
	line1text->x = 467 - ((strlen(line1text->text) * line1text->textwidth) / 2);
	line1text->y = 77;
	line1text->draw = false;
	iface->AddObject(line1text->id);
	if(upgradeavailable){
		line1text->draw = true;
	}else{
		line1text->draw = false;
	}
	
	for(int i = 0; i < 6; i++){
		Overlay * text = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
		Overlay * textlevel = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
		text->text = new char[64];
		textlevel->text = new char[64];
		strcpy(textlevel->text, "");
		switch(i){
			default:
			case 0: strcpy(text->text, "Current Endurance Level:"); break;
			case 1: strcpy(text->text, "Current Shield Level:"); break;
			case 2: strcpy(text->text, "Current Jetpack Level:"); break;
			case 3: strcpy(text->text, "Current Tech Slot Level:"); break;
			case 4: strcpy(text->text, "Current Hacking Level:"); break;
			case 5: strcpy(text->text, "Current Contacts Level:"); break;
		}
		text->textbank = 133;
		textlevel->textbank = 133;
		text->textwidth = 6;
		textlevel->textwidth = 6;
		textlevel->uid = 20 + i;
		text->x = 390;
		textlevel->x = 556 - (strlen(textlevel->text) * textlevel->textwidth);
		text->y = 97 + (i * 46);
		textlevel->y = text->y;
		iface->AddObject(text->id);
		iface->AddObject(textlevel->id);
	}
	
	Button * okbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	okbutton->y = 100;
	okbutton->x = 62;
	okbutton->uid = 0;
	strcpy(okbutton->text, "Done");
	iface->AddObject(background->id);
	iface->AddObject(background2->id);
	iface->AddObject(title->id);
	iface->AddObject(textbox->id);
	iface->AddObject(scrollbar->id);
	iface->AddObject(okbutton->id);
	iface->buttonenter = okbutton->id;
	iface->buttonescape = okbutton->id;
	iface->scrollbar = scrollbar->id;
	gamesummaryinterface = iface->id;
	UpdateGameSummaryInterface();
	return iface;
}

Interface * Game::CreateModalDialog(const char * message, bool ok){
	DestroyModalDialog();
	// 40:4 model dialog background
	Overlay * background = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	background->renderpass = 3;
	background->res_bank = 40;
	background->res_index = 4;
	Overlay * text = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	text->renderpass = 3;
	text->text = new char[strlen(message) + 1];
	strcpy(text->text, message);
	text->textbank = 134;
	text->textwidth = 8;
	text->x = 320 - ((strlen(message) * 8) / 2);
	text->y = 200;
	Interface * dialoginterface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
	if(ok){
		Button * okbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
		okbutton->renderpass = 3;
		okbutton->x = 242;
		okbutton->y = 230;
		okbutton->SetType(Button::B156x21);
		okbutton->uid = 50;
		strcpy(okbutton->text, "OK");
		dialoginterface->AddObject(okbutton->id);
		dialoginterface->buttonenter = okbutton->id;
	}else{
		text->y = 218;
	}
	modaldialoghasok = ok;
	dialoginterface->AddObject(background->id);
	dialoginterface->AddObject(text->id);
	dialoginterface->modal = true;
	modalinterface = dialoginterface->id;
	Interface * iface = static_cast<Interface *>(world.GetObjectFromId(currentinterface));
	if(iface){
		iface->AddObject(modalinterface);
	}
	aftermodalinterface = currentinterface;
	currentinterface = dialoginterface->id;
	return dialoginterface;
}

void Game::DestroyModalDialog(void){
	if(modalinterface){
		currentinterface = aftermodalinterface;
		aftermodalinterface = 0;
		Interface * iface = static_cast<Interface *>(world.GetObjectFromId(currentinterface));
		if(iface){
			Interface * modaliface = static_cast<Interface *>(world.GetObjectFromId(modalinterface));
			if(modaliface){
				modaliface->DestroyInterface(world, iface);
			}
		}
		modalinterface = 0;
	}
}

Interface * Game::CreatePasswordDialog(void){
	// 40:2 model dialog password input
	DestroyModalDialog();
	Overlay * background = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	background->renderpass = 3;
	background->res_bank = 40;
	background->res_index = 2;
	background->x = 320 - (world.resources.spritewidth[background->res_bank][background->res_index] / 2);
	background->y = 240 - (world.resources.spriteheight[background->res_bank][background->res_index] / 2);
	Interface * dialoginterface = (Interface *)world.CreateObject(ObjectTypes::INTERFACE);
	Overlay * text = (Overlay *)world.CreateObject(ObjectTypes::OVERLAY);
	text->renderpass = 3;
	text->text = new char[256];
	strcpy(text->text, "This game requires a password");
	text->textbank = 134;
	text->textwidth = 8;
	text->x = 320 - ((strlen(text->text) * 8) / 2);
	text->y = 196;
	TextInput * passwordinput = (TextInput *)world.CreateObject(ObjectTypes::TEXTINPUT);
	passwordinput->renderpass = 3;
	passwordinput->x = 210;
	passwordinput->y = 243;
	passwordinput->width = 180;
	passwordinput->height = 14;
	passwordinput->res_bank = 135;
	passwordinput->fontwidth = 11;
	passwordinput->maxchars = 20;
	passwordinput->maxwidth = 20;
	passwordinput->password = true;
	passwordinput->uid = 1;
	modaldialoghasok = true;
	Button * okbutton = (Button *)world.CreateObject(ObjectTypes::BUTTON);
	okbutton->renderpass = 3;
	okbutton->x = 242;
	okbutton->y = 267;
	okbutton->SetType(Button::B156x21);
	okbutton->uid = 50;
	strcpy(okbutton->text, "OK");
	dialoginterface->AddObject(background->id);
	dialoginterface->AddObject(text->id);
	dialoginterface->AddObject(passwordinput->id);
	dialoginterface->AddObject(okbutton->id);
	dialoginterface->AddTabObject(passwordinput->id);
	dialoginterface->AddTabObject(okbutton->id);
	dialoginterface->activeobject = passwordinput->id;
	dialoginterface->buttonenter = okbutton->id;
	dialoginterface->buttonescape = okbutton->id;
	dialoginterface->modal = true;
	modalinterface = dialoginterface->id;
	aftermodalinterface = currentinterface;
	Interface * iface = static_cast<Interface *>(world.GetObjectFromId(currentinterface));
	if(iface){
		iface->AddObject(modalinterface);
	}
	currentinterface = dialoginterface->id;
	passwordinterface = dialoginterface->id;
	return dialoginterface;
}

bool Game::GoBack(void){
	if(gamejoininterface || gametechinterface){
		world.Disconnect();
		world.lobby.gamesprocessed = false;
		world.lobby.channelchanged = true;
		world.SwitchToLocalAuthorityMode();
		sharedstate = 0;
		Object * object = world.GetObjectFromId(currentinterface);
		Interface * iface = static_cast<Interface *>(object);
		if(gamejoininterface){
			Interface * gamejoiniface = static_cast<Interface *>(world.GetObjectFromId(gamejoininterface));
			if(gamejoiniface){
				gamejoiniface->DestroyInterface(world, iface);
			}
			gamejoininterface = 0;
			for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
				Object * object = *it;
				switch(object->type){
					case ObjectTypes::TEAM:{
						Team * team = static_cast<Team *>(object);
						team->DestroyOverlays(world);
						world.MarkDestroyObject(object->id);
					}break;
				}
			}
		}else
		if(gametechinterface){
			Interface * gametechiface = static_cast<Interface *>(world.GetObjectFromId(gametechinterface));
			if(gametechiface){
				gametechiface->DestroyInterface(world, iface);
			}
			gametechinterface = 0;
			world.choosingtech = false;
			for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
				Object * object = *it;
				switch(object->type){
					case ObjectTypes::TEAM:{
						Team * team = static_cast<Team *>(object);
						team->DestroyOverlays(world);
						world.MarkDestroyObject(object->id);
					}break;
				}
			}
		}
		gameselectinterface = CreateGameSelectInterface()->id;
		world.lobby.JoinChannel(lastchannel);
		iface->AddObject(gameselectinterface);
		currentinterface = iface->id;
		return true;
	}else
	if(gamecreateinterface){
		Object * object = world.GetObjectFromId(currentinterface);
		Interface * iface = static_cast<Interface *>(object);
		Interface * gamecreateiface = static_cast<Interface *>(world.GetObjectFromId(gamecreateinterface));
		if(gamecreateiface){
			gamecreateiface->DestroyInterface(world, iface);
		}
		gamecreateinterface = 0;
		gameselectinterface = CreateGameSelectInterface()->id;
		world.lobby.gamesprocessed = false;
		iface->AddObject(gameselectinterface);
		currentinterface = iface->id;
		return true;
	}else{
		GoToState(MAINMENU);
	}
	return false;
}

bool Game::ProcessMainMenuInterface(Interface * iface){
	for(std::vector<Uint16>::iterator it = iface->objects.begin(); it != iface->objects.end(); it++){
		Object * object = world.GetObjectFromId(*it);
		if(object->type == ObjectTypes::BUTTON){
			Button * button = static_cast<Button *>(object);
			if(button->clicked){
				switch(button->uid){
					case 0:
						GoToState(SINGLEPLAYERGAME);
					break;
					case 1:
						GoToState(LOBBYCONNECT);
					break;
					case 2:
						GoToState(OPTIONS);
					break;
					case 3:
						return false;
					break;
					case 4:
						GoToState(HOSTGAME);
					break;
					case 5:
						GoToState(JOINGAME);
					break;
					case 6:
						GoToState(TESTGAME);
					break;
					case 7:
						GoToState(REPLAYGAME);
					break;
				}
			}
		}
	}
	return true;
}

void Game::ProcessLobbyConnectInterface(Interface * iface){
	for(std::vector<Uint16>::iterator it = iface->objects.begin(); it != iface->objects.end(); it++){
		Object * object = world.GetObjectFromId(*it);
		if(object->type == ObjectTypes::TEXTBOX){
			TextBox * textbox = static_cast<TextBox *>(object);
			if(textbox){
				world.lobby.LockMutex();
				switch(world.lobby.state){
					case Lobby::CONNECTING:
					
					break;
					case Lobby::WAITINGFORRESOLVER:
						
					break;
					case Lobby::AUTHSENT:
						
					break;
					case Lobby::IDLE:
						
					break;
					case Lobby::WAITING:
						textbox->AddLine("Connecting to lobby.zsilencer.com:517");
						world.lobby.Connect("lobby.zsilencer.com", 517);
						//world.lobby.state = Lobby::AUTHENTICATED;
					break;
					case Lobby::RESOLVING:
						textbox->AddLine("Resolving hostname...");
						world.lobby.state = Lobby::WAITINGFORRESOLVER;
					break;
					case Lobby::RESOLVEFAILED:
						textbox->AddLine("Could not resolve hostname");
						//world.lobby.Disconnect();
						world.lobby.state = Lobby::IDLE;
					break;
					case Lobby::RESOLVED:
						textbox->AddLine("Hostname resolved");
						world.lobby.Connect("lobby.zsilencer.com", 517);
					break;
					case Lobby::CONNECTED:
						textbox->AddLine("Connected");
						textbox->AddLine("Checking version...");
						world.lobby.SendVersion();
						world.lobby.state = Lobby::CHECKINGVERSION;
					break;
					case Lobby::CHECKINGVERSION:
						if(world.lobby.versionchecked){
							if(world.lobby.versionok){
								textbox->AddLine("Software version is current");
								world.lobby.state = Lobby::AUTHENTICATING;
							}else{
								textbox->AddLine("Software is out of date");
								textbox->AddLine("Get latest version at:");
								textbox->AddLine("http://zsilencer.com");
								world.lobby.Disconnect();
								world.lobby.state = Lobby::IDLE;
							}
						}
					break;
					case Lobby::AUTHENTICATING:
						//world.lobby.state = Lobby::AUTHENTICATED;
					break;
					case Lobby::AUTHFAILED:
						textbox->AddLine("Authentication failed");
						if(strlen(world.lobby.failmessage) > 0){
							textbox->AddLine(world.lobby.failmessage);
						}
						world.lobby.state = Lobby::AUTHENTICATING;
						//world.lobby.Disconnect();
					break;
					case Lobby::AUTHENTICATED:
						textbox->AddLine("Authenticated");
						GoToState(LOBBY);
					break;
					case Lobby::CONNECTIONFAILED:
						textbox->AddLine("Connection failed");
						world.lobby.state = Lobby::IDLE;
					break;
					case Lobby::DISCONNECTED:
						textbox->AddLine("Disconnected");
						world.lobby.state = Lobby::IDLE;
					break;
				}
				if(world.lobby.motdreceived && !motdprinted){
					char * line = strtok(world.lobby.motd, "\n");
					while(line != 0){
						textbox->AddLine(line);
						line = strtok(NULL, "\n");
					}
					motdprinted = true;
				}
				world.lobby.UnlockMutex();
			}
		}else
		if(object->type == ObjectTypes::BUTTON){
			Button * button = static_cast<Button *>(object);
			if(button && button->clicked){
				switch(button->uid){
					case 0:{
						if(world.lobby.state == Lobby::AUTHENTICATING){
							TextInput * usernameinput = static_cast<TextInput *>(iface->GetObjectWithUid(world, 1));
							TextInput * passwordinput = static_cast<TextInput *>(iface->GetObjectWithUid(world, 2));
							if(usernameinput && passwordinput){
								strcpy(localusername, usernameinput->text);
								world.lobby.SendCredentials(usernameinput->text, passwordinput->text);
								world.lobby.state = Lobby::AUTHSENT;
							}
						}
					}break;
					case 1:{
						GoToState(MAINMENU);
					}break;
				}
				button->clicked = false;
			}
		}else
		if(object->type == ObjectTypes::TEXTINPUT){
			TextInput * textinput = static_cast<TextInput *>(object);
			if(textinput){
				if(world.lobby.state == Lobby::AUTHSENT){
					textinput->inactive = true;
				}else{
					textinput->inactive = false;
				}
			}
		}
	}
}

bool Game::ProcessLobbyInterface(Interface * iface){
	UpdateTechInterface();
	for(std::vector<Uint16>::iterator it = iface->objects.begin(); it != iface->objects.end(); it++){
		Object * object = world.GetObjectFromId(*it);
		if(object){
			switch(object->type){
				case ObjectTypes::SCROLLBAR:{
					ScrollBar * scrollbar = static_cast<ScrollBar *>(object);
					if(scrollbar){
						
					}
				}break;
				case ObjectTypes::INTERFACE:{
					Interface * iface = static_cast<Interface *>(object);
					if(iface){
						if(!ProcessLobbyInterface(iface)){
							return false;
						}
					}
				}break;
				case ObjectTypes::TEXTINPUT:{
					TextInput * textinput = static_cast<TextInput *>(object);
					if(textinput){
						Interface * chatiface = static_cast<Interface *>(world.GetObjectFromId(chatinterface));
						if(chatiface && iface->activeobject == chatiface->activeobject){
							if(textinput->enterpressed && strlen(textinput->text) > 0){
								world.lobby.SendChat(world.lobby.channel, textinput->text);
								textinput->Clear();
							}
						}
						if(textinput->enterpressed){
							textinput->enterpressed = false;
						}
					}
				}break;
				case ObjectTypes::SELECTBOX:{
					SelectBox * selectbox = static_cast<SelectBox *>(object);
					if(selectbox){
						if(selectbox->uid == 10 && !world.lobby.gamesprocessed){
							bool deleted;
							do{
								deleted = false;
								unsigned int index = 0;
								for(std::deque<Uint32>::iterator it = selectbox->itemids.begin(); it != selectbox->itemids.end(); it++, index++){
									Uint32 gameid = (*it);
									if(!world.lobby.GetGameById(gameid)){
										selectbox->DeleteItem(index);
										deleted = true;
										break;
									}
								}
							}while(deleted);
							for(std::list<LobbyGame *>::iterator it = world.lobby.games.begin(); it != world.lobby.games.end(); it++){
								LobbyGame * lobbygame = (*it);
								if(selectbox->IdToIndex(lobbygame->id) == -1){
									selectbox->AddItem(lobbygame->name, lobbygame->id);
								}
							}
							world.lobby.gamesprocessed = true;
						}
						
						Object * object = world.GetObjectFromId(iface->scrollbar);
						ScrollBar * scrollbar = static_cast<ScrollBar *>(object);
						if(scrollbar){
							selectbox->scrolled = scrollbar->scrollposition;
							if(selectbox->items.size() > ceil(float(selectbox->height) / selectbox->lineheight)){
								scrollbar->draw = true;
								scrollbar->scrollmax = selectbox->items.size() - ceil(float(selectbox->height) / selectbox->lineheight);
							}else{
								scrollbar->draw = false;
							}
							LobbyGame * lobbygame = world.lobby.GetGameById(selectbox->IndexToId(selectbox->selecteditem));
							Object * tobject = iface->GetObjectWithUid(world, 1);
							if(tobject && tobject->type == ObjectTypes::OVERLAY){
								Overlay * overlay = static_cast<Overlay *>(tobject);
								if(overlay){
									if(lobbygame){
										strcpy(overlay->text, lobbygame->name);
									}else{
										strcpy(overlay->text, "");
									}
								}
							}
							tobject = iface->GetObjectWithUid(world, 2);
							if(tobject && tobject->type == ObjectTypes::OVERLAY){
								Overlay * overlay = static_cast<Overlay *>(tobject);
								if(overlay){
									if(lobbygame){
										char temp[256];
										//sprintf(temp, "Host: %s:%d", lobbygame->hostname, lobbygame->port);
										sprintf(temp, "Map: %s", lobbygame->mapname);
										strcpy(overlay->text, temp);
										//strcpy(overlay->text, "Map: THET06e.SIL");
									}else{
										strcpy(overlay->text, "");
									}
								}
							}
							tobject = iface->GetObjectWithUid(world, 3);
							if(tobject && tobject->type == ObjectTypes::OVERLAY){
								Overlay * overlay = static_cast<Overlay *>(tobject);
								if(overlay){
									if(lobbygame){
										const char * passwordlock = "";
										if(strlen(lobbygame->password) > 0){
											passwordlock = "*PASSWORD LOCK*";
										}
										const char * security = "No";
										switch(lobbygame->securitylevel){
											case LobbyGame::SECLOW:
												security = "Low";
												break;
											case LobbyGame::SECMEDIUM:
												security = "Medium";
												break;
											case LobbyGame::SECHIGH:
												security = "High";
												break;
										}
										sprintf(overlay->text, "%s Security", security);
										while(strlen(overlay->text) < 21){
											strcat(overlay->text, " ");
										}
										strcat(overlay->text, passwordlock);
									}else{
										strcpy(overlay->text, "");
									}
								}
							}
							tobject = iface->GetObjectWithUid(world, 4);
							if(tobject && tobject->type == ObjectTypes::OVERLAY){
								Overlay * overlay = static_cast<Overlay *>(tobject);
								if(overlay){
									if(lobbygame){
										sprintf(overlay->text, "Creator: %s", world.lobby.GetUserInfo(lobbygame->accountid)->name);
									}else{
										strcpy(overlay->text, "");
									}
								}
							}
							tobject = iface->GetObjectWithUid(world, 5);
							if(tobject && tobject->type == ObjectTypes::OVERLAY){
								Overlay * overlay = static_cast<Overlay *>(tobject);
								if(overlay){
									if(lobbygame){
										sprintf(overlay->text, "MinLv:%d MaxLv:%d MaxPl:%d MaxTm:%d", lobbygame->minlevel, lobbygame->maxlevel, lobbygame->maxplayers, lobbygame->maxteams);
									}else{
										strcpy(overlay->text, "");
									}
								}
							}
						}
					}
				}break;
				case ObjectTypes::TOGGLE:{
					if(iface->id == characterinterface){
						Uint8 selectedagency = GetSelectedAgency();
						if(selectedagency != oldselectedagency){
							Config::GetInstance().defaultagency = selectedagency;
							Config::GetInstance().Save();
							oldselectedagency = selectedagency;
							agencychanged = true;
						}
					}
				}break;
				case ObjectTypes::TEXTBOX:{
					TextBox * textbox = static_cast<TextBox *>(object);
					if(textbox){
						Object * object = world.GetObjectFromId(iface->scrollbar);
						ScrollBar * scrollbar = static_cast<ScrollBar *>(object);
						if(minimized && world.lobby.chatmessages.size() > chatlinesprinted){
							SDL_SysWMinfo info;
							SDL_VERSION(&info.version);
							if(SDL_GetWindowWMInfo(window, &info)){
#ifdef _WIN32
								if(info.subsystem == SDL_SYSWM_WINDOWS){
									FLASHWINFO flashinfo;
									flashinfo.cbSize = sizeof(flashinfo);
									flashinfo.hwnd = info.info.win.window;
									flashinfo.dwFlags = FLASHW_ALL | FLASHW_TIMERNOFG;
									flashinfo.uCount = 0xFFFFFFFF;
									flashinfo.dwTimeout = 0;
									FlashWindowEx(&flashinfo);
								}
#endif
#ifdef __APPLE__
								if(info.subsystem == SDL_SYSWM_COCOA){
									NSWindow * nswindow = info.info.cocoa.window;
									RequestUserAttention(nswindow);
								}
#endif
							}
						}
						while(world.lobby.chatmessages.size() > chatlinesprinted){
							char * message = world.lobby.chatmessages.front();
							Uint8 color = message[strlen(message) + 1];
							Uint8 brightness = message[strlen(message) + 2];
							textbox->AddText(message, color, brightness);
							delete[] message;
							world.lobby.chatmessages.pop_front();
							if(scrollbar){
								scrollbar->scrollposition = textbox->scrolled;
							}
						}
						if(scrollbar){
							textbox->scrolled = scrollbar->scrollposition;
							if(textbox->text.size() > ceil(float(textbox->height) / textbox->lineheight)){
								scrollbar->draw = true;
								scrollbar->scrollmax = textbox->text.size() - ceil(float(textbox->height) / textbox->lineheight);
							}else{
								scrollbar->draw = false;
							}
						}
					}
				}break;
				case ObjectTypes::OVERLAY:{
					Overlay * overlay = static_cast<Overlay *>(object);
					if(overlay){
						switch(overlay->uid){
							case 1:{
								if(world.lobby.channelchanged){
									if(strlen(lastchannel) == 0){
										strcpy(lastchannel, world.lobby.channel);
									}
									strcpy(overlay->text, world.lobby.channel);
									world.lobby.channelchanged = false;
								}
							}break;
						}
						if(agencychanged && iface->id == characterinterface){
							//printf("agency changed\n");
							User * user = world.lobby.GetUserInfo(world.lobby.accountid);
							if(user && !user->retrieving){
								Uint8 selectedagency = GetSelectedAgency();
								switch(overlay->uid){
									case 2:{
										sprintf(overlay->text, "LEVEL: %d", user->agency[selectedagency].level);
									}break;
									case 3:{
										sprintf(overlay->text, "WINS: %d", user->agency[selectedagency].wins);
									}break;
									case 4:{
										sprintf(overlay->text, "LOSSES: %d", user->agency[selectedagency].losses);
									}break;
									case 5:{
										sprintf(overlay->text, "XP TO NEXT LEVEL: %d", user->agency[selectedagency].xptonextlevel);
										agencychanged = false;
									}break;
								}
							}
						}
					}
				}break;
				case ObjectTypes::BUTTON:{
					Button * button = static_cast<Button *>(object);
					if(button && button->clicked && button->type != Button::BCHECKBOX){
						button->clicked = false;
						switch(button->uid){
							case 10:{ // go back
								if(GoBack()){
									return false;
								}
							}break;
							case 20:{ // join game
								if(gameselectinterface){
									Interface * gameselectiface = static_cast<Interface *>(world.GetObjectFromId(gameselectinterface));
									if(gameselectiface){
										for(std::vector<Uint16>::iterator it = gameselectiface->objects.begin(); it != gameselectiface->objects.end(); it++){
											Object * object = world.GetObjectFromId(*it);
											if(object && object->type == ObjectTypes::SELECTBOX){
												SelectBox * selectbox = static_cast<SelectBox *>(object);
												if(selectbox->selecteditem != -1){
													Uint32 gameid = selectbox->IndexToId(selectbox->selecteditem);
													if(gameid){
														LobbyGame * lobbygame = world.lobby.GetGameById(gameid);
														if(lobbygame){
															if(world.state == World::IDLE){
																User * user = world.lobby.GetUserInfo(world.lobby.accountid);
																bool canjoin = true;
																if(user){
																	if(lobbygame->minlevel > user->agency[GetSelectedAgency()].level){
																		canjoin = false;
																		CreateModalDialog("Your player level is too low");
																	}else
																	if(lobbygame->maxlevel < user->agency[GetSelectedAgency()].level){
																		canjoin = false;
																		CreateModalDialog("Your player level is too high");
																	}
																}
																if(canjoin){
																	currentlobbygameid = lobbygame->id;
																	if(strlen(lobbygame->password) > 0 && lobbygame->accountid != world.lobby.accountid){
																		currentinterface = CreatePasswordDialog()->id;
																	}else{
																		JoinGame(*lobbygame);
																	}
																}
															}
														}
													}
												}else{
													CreateModalDialog("No game selected");
												}
											}
										}
									}
								}
							}break;
							case 25:{ // start game/ready
								if(gamejoininterface){
									world.SendReady();
								}
							}break;
							case 26:{ // change team
								if(gamejoininterface){
									world.ChangeTeam();
								}
							}break;
							case 27:{ // choose tech
								if(gamejoininterface){
									Object * object = world.GetObjectFromId(currentinterface);
									Interface * iface = static_cast<Interface *>(object);
									Interface * gamejoiniface = static_cast<Interface *>(world.GetObjectFromId(gamejoininterface));
									if(gamejoiniface){
										gamejoiniface->DestroyInterface(world, iface);
									}
									world.choosingtech = true;
									ShowTeamOverlays(false);
									gamejoininterface = 0;
									gametechinterface = CreateGameTechInterface()->id;
									iface->AddObject(gametechinterface);
									iface->activeobject = gametechinterface;
									Interface * chatiface = static_cast<Interface *>(world.GetObjectFromId(chatinterface));
									if(chatiface){
										chatiface->activeobject = 0;
									}
									currentinterface = iface->id;
									iface->ActiveChanged(world, iface, false);
									UpdateTechInterface();
									return false;
								}
							}break;
							case 28:{ // back to teams
								if(!gamejoininterface){
									Object * object = world.GetObjectFromId(currentinterface);
									Interface * iface = static_cast<Interface *>(object);
									Interface * gametechiface = static_cast<Interface *>(world.GetObjectFromId(gametechinterface));
									if(gametechiface){
										gametechiface->DestroyInterface(world, iface);
									}
									gametechinterface = 0;
									gamejoininterface = CreateGameJoinInterface()->id;
									iface->AddObject(gamejoininterface);
									world.choosingtech = false;
									ShowTeamOverlays(true);
									iface->activeobject = gamejoininterface;
									Interface * chatiface = static_cast<Interface *>(world.GetObjectFromId(chatinterface));
									if(chatiface){
										chatiface->activeobject = 0;
									}
									iface->ActiveChanged(world, iface, false);
									return false;
								}
							}break;
							case 30:{ // create game
								if(gameselectinterface){
									Object * object = world.GetObjectFromId(currentinterface);
									Interface * iface = static_cast<Interface *>(object);
									if(gameselectinterface){
										Interface * gameselectiface = static_cast<Interface *>(world.GetObjectFromId(gameselectinterface));
										if(gameselectiface){
											gameselectiface->DestroyInterface(world, iface);
										}
										gameselectinterface = 0;
									}
									gamecreateinterface = CreateGameCreateInterface()->id;
									iface->AddObject(gamecreateinterface);
									iface->activeobject = gamecreateinterface;
									Interface * chatiface = static_cast<Interface *>(world.GetObjectFromId(chatinterface));
									if(chatiface){
										chatiface->activeobject = 0;
									}
									iface->ActiveChanged(world, iface, false);
									return false;
								}
							}break;
							case 35:{ // create game create
								if(!creategameclicked){
									const char * gamename = "";
									const char * mapname = "";
									const char * password = 0;
									Interface * gamecreateiface = static_cast<Interface *>(world.GetObjectFromId(gamecreateinterface));
									if(gamecreateiface){
										Object * tobject = gamecreateiface->GetObjectWithUid(world, 5);
										if(tobject){
											TextInput * textinput = static_cast<TextInput *>(tobject);
											gamename = textinput->text;
										}
										tobject = gamecreateiface->GetObjectWithUid(world, 6);
										if(tobject){
											TextInput * textinput = static_cast<TextInput *>(tobject);
											if(strlen(textinput->text) > 0){
												password = textinput->text;
											}
										}
										Uint8 securitylevel = LobbyGame::SECNONE;
										tobject = gamecreateiface->GetObjectWithUid(world, 40);
										if(tobject){
											Button * button = static_cast<Button *>(tobject);
											if(strcmp(button->text, "Low") == 0){
												securitylevel = LobbyGame::SECLOW;
											}else
											if(strcmp(button->text, "Medium") == 0){
												securitylevel = LobbyGame::SECMEDIUM;
											}else
											if(strcmp(button->text, "High") == 0){
												securitylevel = LobbyGame::SECHIGH;
											}
										}
										Uint8 minlevel = 0;
										tobject = gamecreateiface->GetObjectWithUid(world, 41);
										if(tobject){
											TextInput * textinput = static_cast<TextInput *>(tobject);
											minlevel = atoi(textinput->text);
										}
										Uint8 maxlevel = 0;
										tobject = gamecreateiface->GetObjectWithUid(world, 42);
										if(tobject){
											TextInput * textinput = static_cast<TextInput *>(tobject);
											maxlevel = atoi(textinput->text);
										}
										Uint8 maxplayers = 0;
										tobject = gamecreateiface->GetObjectWithUid(world, 43);
										if(tobject){
											TextInput * textinput = static_cast<TextInput *>(tobject);
											maxplayers = atoi(textinput->text);
										}
										Uint8 maxteams = 0;
										tobject = gamecreateiface->GetObjectWithUid(world, 44);
										if(tobject){
											TextInput * textinput = static_cast<TextInput *>(tobject);
											maxteams = atoi(textinput->text);
										}
										if(strlen(gamename) == 0){
											CreateModalDialog("No game name");
										}else{
											tobject = gamecreateiface->GetObjectWithUid(world, 4);
											if(tobject){
												SelectBox * selectbox = static_cast<SelectBox *>(tobject);
												if(selectbox->selecteditem >= 0){
													mapname = selectbox->GetItemName(selectbox->selecteditem);
													world.lobby.CreateGame(gamename, mapname, password, securitylevel, minlevel, maxlevel, maxplayers, maxteams);
													creategameclicked = true;
													strcpy(Config::GetInstance().defaultgamename, gamename);
													Config::GetInstance().Save();
													CreateModalDialog("Creating game...", false);
												}else{
													CreateModalDialog("No map selected");
												}
											}
										}
									}
								}
							}break;
							case 40:{ // security level
								if(gamecreateinterface){
									Interface * gamecreateiface = static_cast<Interface *>(world.GetObjectFromId(gamecreateinterface));
									if(gamecreateiface){
										if(strcmp(button->text, "Off") == 0){
											strcpy(button->text, "Low");
										}else
										if(strcmp(button->text, "Low") == 0){
											strcpy(button->text, "Medium");
										}else
										if(strcmp(button->text, "Medium") == 0){
											strcpy(button->text, "High");
										}else
										if(strcmp(button->text, "High") == 0){
											strcpy(button->text, "Off");
										}
									}
								}
							}break;
							case 50: // modal ok button pressed
								if(modalinterface){
									Interface * modaliface = static_cast<Interface *>(world.GetObjectFromId(modalinterface));
									if(iface->id == passwordinterface && modaliface){
										TextInput * passwordinput = static_cast<TextInput *>(modaliface->GetObjectWithUid(world, 1));
										LobbyGame * lobbygame = world.lobby.GetGameById(currentlobbygameid);
										if(lobbygame && passwordinput){
											JoinGame(*lobbygame, passwordinput->text);
										}
									}
									DestroyModalDialog();
									creategameclicked = false;
									if(gamejoininterface || gametechinterface){
										if(GoBack()){
											return false;
										}
									}
									return false;
								}
							break;
						}
					}
				}break;
			}
		}
	}
	return true;
}

void Game::ProcessGameSummaryInterface(Interface * iface){
	if(world.lobby.statupgraded || !gamesummaryinfoloaded){
		User * user = world.lobby.GetUserInfo(world.lobby.accountid);
		if(user && !user->retrieving){
			UpdateGameSummaryInterface();
			world.lobby.statupgraded = false;
		}
	}
	for(std::vector<Uint16>::iterator it = iface->objects.begin(); it != iface->objects.end(); it++){
		Object * object = world.GetObjectFromId(*it);
		if(object){
			switch(object->type){
				case ObjectTypes::TEXTBOX:{
					TextBox * textbox = static_cast<TextBox *>(object);
					if(textbox){
						Object * object = world.GetObjectFromId(iface->scrollbar);
						ScrollBar * scrollbar = static_cast<ScrollBar *>(object);
						if(scrollbar){
							textbox->scrolled = scrollbar->scrollposition;
						}
					}
				}break;
				case ObjectTypes::BUTTON:{
					Button * button = static_cast<Button *>(object);
					if(button && button->clicked){
						button->clicked = false;
						User * user = world.lobby.GetUserInfo(world.lobby.accountid);
						switch(button->uid){
							case 0:{ // continue
								if(world.lobby.state == Lobby::AUTHENTICATED){
									GoToState(LOBBY);
									world.lobby.JoinChannel(lastchannel);
								}else{
									GoToState(MAINMENU);
								}
							}break;
							case 10:{ // upgrade endurance
								world.lobby.UpgradeStat(user->statsagency, 0);
							}break;
							case 11:{ // upgrade shield
								world.lobby.UpgradeStat(user->statsagency, 1);
							}break;
							case 12:{ // upgrade jetpack
								world.lobby.UpgradeStat(user->statsagency, 2);
							}break;
							case 13:{ // upgrade techslots
								world.lobby.UpgradeStat(user->statsagency, 3);
							}break;
							case 14:{ // upgrade hacking
								world.lobby.UpgradeStat(user->statsagency, 4);
							}break;
							case 15:{ // upgrade contacts
								world.lobby.UpgradeStat(user->statsagency, 5);
							}break;
						}
					}
				}
			}
		}
	}
}

void Game::UpdateTechInterface(void){
	if(gametechinterface){
		int techslotsleft = 0;
		Interface * gametechiface = static_cast<Interface *>(world.GetObjectFromId(gametechinterface));
		if(gametechiface){
			Overlay * overlay = static_cast<Overlay *>(gametechiface->GetObjectWithUid(world, 70));
			if(overlay){
				Peer * peer = world.peerlist[world.localpeerid];
				if(peer){
					Team * team = world.GetPeerTeam(world.localpeerid);
					User * user = world.lobby.GetUserInfo(peer->accountid);
					if(user && team){
						techslotsleft = user->agency[team->agency].techslots - world.TechSlotsUsed(*peer);
						sprintf(overlay->text, "Tech slots left: %d", techslotsleft);
					}
				}
			}
		}
		Team * team = world.GetPeerTeam(world.localpeerid);
		if(team){
			int peerindex = 0;
			for(int i = 0; i < 4; i++){
				Peer * peer = world.peerlist[team->peers[i]];
				User * user = 0;
				if(peer){
					user = world.lobby.GetUserInfo(peer->accountid);
				}
				bool draw = true;
				if(i >= team->numpeers){
					draw = false;
				}
				int b = 0;
				int bpos = 0;
				for(std::vector<BuyableItem *>::iterator it = world.buyableitems.begin(); it != world.buyableitems.end(); it++){
					BuyableItem * buyableitem = *it;
					if(buyableitem->techslots){
						if(buyableitem->agencyspecific == -1 || buyableitem->agencyspecific == team->agency){
							bool usable = true;
							Uint8 uid = 110 + (30 * peerindex) + b;
							if(team->peers[i] == world.localpeerid){
								uid = 110 + (30 * 3) + b;
								if(buyableitem->techslots <= techslotsleft || (peer && peer->techchoices & buyableitem->techchoice)){
									usable = false;
								}
							}
							Interface * gametechiface = static_cast<Interface *>(world.GetObjectFromId(gametechinterface));
							if(gametechiface){
								Button * button = static_cast<Button *>(gametechiface->GetObjectWithUid(world, uid));
								if(button){
									if(peer && peer->techchoices & buyableitem->techchoice){
										button->res_index = 18; // on
									}else{
										button->res_index = 19; // off
									}
									if(team->peers[i] == world.localpeerid){
										if(!usable){
											button->effectbrightness = 128;
										}else{
											button->effectbrightness = 64;
										}
									}
									if(button && button->type == Button::BCHECKBOX){
										if(button->clicked){
											if(button->uid >= 200 && button->effectbrightness == 128){
												//Uint32 techchoice = 1 << (button->uid - 200);
												Peer * peer = world.peerlist[world.localpeerid];
												//printf("tech slots used: %d, %d\n", world.TechSlotsUsed(*peer), peer->techchoices);
												if(peer){
													world.SetTech(peer->techchoices ^ buyableitem->techchoice);
													Team * team = world.GetPeerTeam(world.localpeerid);
													if(team){
														Config::GetInstance().defaulttechchoices[team->agency] = peer->techchoices ^ buyableitem->techchoice;
														Config::GetInstance().Save();
													}
												}
											}
											button->clicked = false;
										}
									}
									button->draw = draw;
								}
							}
							if(gametechiface){
								Overlay * overlay = static_cast<Overlay *>(gametechiface->GetObjectWithUid(world, 230 + b));
								if(overlay){
									if(overlay->clicked){
										overlay->clicked = false;
										Overlay * nameoverlay = static_cast<Overlay *>(gametechiface->GetObjectWithUid(world, 60));
										if(nameoverlay){
											sprintf(nameoverlay->text, "-%s-", buyableitem->name);
											nameoverlay->x = 401 + (116 - ((strlen(nameoverlay->text) * nameoverlay->textwidth) / 2));
										}
										char desc[1024];
										strcpy(desc, buyableitem->description);
										int linenum = 0;
										char * descline = strtok(desc, "\n");
										while(descline){
											Overlay * descoverlay = static_cast<Overlay *>(gametechiface->GetObjectWithUid(world, 61 + linenum));
											if(descoverlay){
												strcpy(descoverlay->text, descline);
											}
											linenum++;
											descline = strtok(NULL, "\n");
										}
										for(int i = linenum; i < 9; i++){
											Overlay * descoverlay = static_cast<Overlay *>(gametechiface->GetObjectWithUid(world, 61 + i));
											if(descoverlay){
												strcpy(descoverlay->text, "");
											}
										}
									}
									if(team->peers[i] == world.localpeerid){
										if(!usable){
											overlay->effectbrightness = 128;
										}else{
											overlay->effectbrightness = 64;
										}
									}
								}
							}
							bpos++;
						}
						b++;
					}
				}
				if(team->peers[i] != world.localpeerid){
					Interface * gametechiface = static_cast<Interface *>(world.GetObjectFromId(gametechinterface));
					if(gametechiface){
						Overlay * overlay = static_cast<Overlay *>(gametechiface->GetObjectWithUid(world, 80 + peerindex));
						Overlay * overlayline = static_cast<Overlay *>(gametechiface->GetObjectWithUid(world, 90 + peerindex));
						if(overlay && overlayline){
							overlay->draw = draw;
							if(user){
								strcpy(overlay->text, user->name);
								overlay->x = 375 - (strlen(overlay->text) * 6);
							}
							overlayline->draw = draw;
						}
					}
					peerindex++;
				}
			}
		}
	}
}

void Game::UpdateGameSummaryInterface(void){
	if(!gamesummaryinterface){
		return;
	}
	Interface * gamesummaryiface = static_cast<Interface *>(world.GetObjectFromId(gamesummaryinterface));
	if(gamesummaryiface){
		printf("Updated game summary\n");
		bool upgradeavailable = false;
		bool upgradesavailable[6];
		for(int i = 0; i < 6; i++){
			upgradesavailable[i] = false;
		}
		int totalbonusupgrades = 0;
		User * user = world.lobby.GetUserInfo(world.lobby.accountid);
		if(user && !user->retrieving){
			printf("user found\n");
			gamesummaryinfoloaded = true;
			totalbonusupgrades += user->agency[user->statsagency].endurance;
			totalbonusupgrades += user->agency[user->statsagency].shield;
			totalbonusupgrades += user->agency[user->statsagency].jetpack;
			totalbonusupgrades += user->agency[user->statsagency].techslots;
			totalbonusupgrades += user->agency[user->statsagency].hacking;
			totalbonusupgrades += user->agency[user->statsagency].contacts;
			if(user->agency[user->statsagency].endurance < user->maxendurance){
				upgradesavailable[0] = true;
			}
			if(user->agency[user->statsagency].shield < user->maxshield){
				upgradesavailable[1] = true;
			}
			if(user->agency[user->statsagency].jetpack < user->maxjetpack){
				upgradesavailable[2] = true;
			}
			if(user->agency[user->statsagency].techslots < user->maxtechslots){
				upgradesavailable[3] = true;
			}
			if(user->agency[user->statsagency].hacking < user->maxhacking){
				upgradesavailable[4] = true;
			}
			if(user->agency[user->statsagency].contacts < user->maxcontacts){
				upgradesavailable[5] = true;
			}
			if(totalbonusupgrades - user->agency[user->statsagency].defaultbonuses < user->agency[user->statsagency].level){
				upgradeavailable = true;
			}
		}
		for(int i = 0; i < 6; i++){
			if(upgradesavailable[i] && upgradeavailable){
				if(!gamesummaryiface->GetObjectWithUid(world, 10 + i)){
					Button * button = (Button *)world.CreateObject(ObjectTypes::BUTTON);
					button->y = -180 + (i * 46);
					button->x = 62;
					button->uid = 10 + i;
					switch(i){
						case 0: sprintf(button->text, "+1 Endurance"); break;
						case 1: sprintf(button->text, "+1 Shield   "); break;
						case 2: sprintf(button->text, "+1 Jetpack  "); break;
						case 3: sprintf(button->text, "+1 Tech Slot"); break;
						case 4: sprintf(button->text, "+1 Hacking  "); break;
						case 5: sprintf(button->text, "+1 Contacts "); break;
					}
					gamesummaryiface->AddObject(button->id);
				}
			}else{
				Button * button = static_cast<Button *>(gamesummaryiface->GetObjectWithUid(world, 10 + i));
				if(button){
					gamesummaryiface->RemoveObject(button->id);
					world.MarkDestroyObject(button->id);
				}
			}
			Overlay * overlay = static_cast<Overlay *>(gamesummaryiface->GetObjectWithUid(world, 20 + i));
			if(overlay){
				switch(i){
					case 0: sprintf(overlay->text, "%d", user->agency[user->statsagency].endurance); break;
					case 1: sprintf(overlay->text, "%d", user->agency[user->statsagency].shield); break;
					case 2: sprintf(overlay->text, "%d", user->agency[user->statsagency].jetpack); break;
					case 3: sprintf(overlay->text, "%d", user->agency[user->statsagency].techslots); break;
					case 4: sprintf(overlay->text, "%d", user->agency[user->statsagency].hacking); break;
					case 5: sprintf(overlay->text, "%d", user->agency[user->statsagency].contacts); break;
				}
				overlay->x = 556 - (strlen(overlay->text) * overlay->textwidth);
			}
		}
		Overlay * overlay = static_cast<Overlay *>(gamesummaryiface->GetObjectWithUid(world, 1));
		if(overlay){
			if(upgradeavailable){
				overlay->draw = true;
			}else{
				overlay->draw = false;
			}
		}
	}
}

void Game::AddSummaryLine(TextBox & textbox, const char * name, Uint32 value, bool percentage){
	char string[256];
	char valuetext[64];
	sprintf(valuetext, "%d%s", value, percentage ? "%" : " ");
	int maxchars = textbox.width / textbox.fontwidth;
	int used = strlen(name) + strlen(valuetext);
	strcpy(string, name);
	for(int i = 0; i < maxchars - used; i++){
		strcat(string, " ");
	}
	strcat(string, valuetext);
	textbox.AddLine(string);
}

void Game::ShowTeamOverlays(bool show){
	for(std::list<Object *>::iterator it = world.objectlist.begin(); it != world.objectlist.end(); it++){
		Object * object = *it;
		if(object->type == ObjectTypes::TEAM){
			Team * team = static_cast<Team *>(object);
			team->ShowOverlays(world, show);
		}
	}
}

Uint8 Game::GetSelectedAgency(void){
	Interface * characteriface = static_cast<Interface *>(world.GetObjectFromId(characterinterface));
	for(std::vector<Uint16>::iterator it = characteriface->objects.begin(); it != characteriface->objects.end(); it++){
		Object * object = world.GetObjectFromId(*it);
		if(object && object->type == ObjectTypes::TOGGLE){
			Toggle * toggle = static_cast<Toggle *>(object);
			if(toggle && toggle->selected){
				switch(toggle->uid){
					case 1:
						return Team::NOXIS;
					break;
					case 2:
						return Team::LAZARUS;
					break;
					case 3:
						return Team::CALIBER;
					break;
					case 4:
						return Team::STATIC;
					break;
					case 5:
						return Team::BLACKROSE;
					break;
				}
			}
		}
	}
	return 0;
}

void Game::IndexToConfigKey(int index, SDL_Scancode ** key1, SDL_Scancode ** key2, bool ** keyop){
	switch(index){
		case 0:
			*key1 = &Config::GetInstance().keymoveupbinding[0];
			*key2 = &Config::GetInstance().keymoveupbinding[1];
			*keyop = &Config::GetInstance().keymoveupoperator;
		break;
		case 1:
			*key1 = &Config::GetInstance().keymovedownbinding[0];
			*key2 = &Config::GetInstance().keymovedownbinding[1];
			*keyop = &Config::GetInstance().keymovedownoperator;
		break;
		case 2:
			*key1 = &Config::GetInstance().keymoveleftbinding[0];
			*key2 = &Config::GetInstance().keymoveleftbinding[1];
			*keyop = &Config::GetInstance().keymoveleftoperator;
		break;
		case 3:
			*key1 = &Config::GetInstance().keymoverightbinding[0];
			*key2 = &Config::GetInstance().keymoverightbinding[1];
			*keyop = &Config::GetInstance().keymoverightoperator;
		break;
		case 4:
			*key1 = &Config::GetInstance().keylookupleftbinding[0];
			*key2 = &Config::GetInstance().keylookupleftbinding[1];
			*keyop = &Config::GetInstance().keylookupleftoperator;
		break;
		case 5:
			*key1 = &Config::GetInstance().keylookuprightbinding[0];
			*key2 = &Config::GetInstance().keylookuprightbinding[1];
			*keyop = &Config::GetInstance().keylookuprightoperator;
		break;
		case 6:
			*key1 = &Config::GetInstance().keylookdownleftbinding[0];
			*key2 = &Config::GetInstance().keylookdownleftbinding[1];
			*keyop = &Config::GetInstance().keylookdownleftoperator;
		break;
		case 7:
			*key1 = &Config::GetInstance().keylookdownrightbinding[0];
			*key2 = &Config::GetInstance().keylookdownrightbinding[1];
			*keyop = &Config::GetInstance().keylookdownrightoperator;
		break;
		case 8:
			*key1 = &Config::GetInstance().keyjumpbinding[0];
			*key2 = &Config::GetInstance().keyjumpbinding[1];
			*keyop = &Config::GetInstance().keyjumpoperator;
		break;
		case 9:
			*key1 = &Config::GetInstance().keyjetpackbinding[0];
			*key2 = &Config::GetInstance().keyjetpackbinding[1];
			*keyop = &Config::GetInstance().keyjetpackoperator;
		break;
		case 10:
			*key1 = &Config::GetInstance().keyactivatebinding[0];
			*key2 = &Config::GetInstance().keyactivatebinding[1];
			*keyop = &Config::GetInstance().keyactivateoperator;
		break;
		case 11:
			*key1 = &Config::GetInstance().keyusebinding[0];
			*key2 = &Config::GetInstance().keyusebinding[1];
			*keyop = &Config::GetInstance().keyuseoperator;
		break;
		case 12:
			*key1 = &Config::GetInstance().keyfirebinding[0];
			*key2 = &Config::GetInstance().keyfirebinding[1];
			*keyop = &Config::GetInstance().keyfireoperator;
		break;
		case 13:
			*key1 = &Config::GetInstance().keychatbinding[0];
			*key2 = &Config::GetInstance().keychatbinding[1];
			*keyop = &Config::GetInstance().keychatoperator;
		break;
		case 14:
			*key1 = &Config::GetInstance().keynextinvbinding[0];
			*key2 = &Config::GetInstance().keynextinvbinding[1];
			*keyop = &Config::GetInstance().keynextinvoperator;
		break;
		case 15:
			*key1 = &Config::GetInstance().keynextcambinding[0];
			*key2 = &Config::GetInstance().keynextcambinding[1];
			*keyop = &Config::GetInstance().keynextcamoperator;
		break;
		case 16:
			*key1 = &Config::GetInstance().keyprevcambinding[0];
			*key2 = &Config::GetInstance().keyprevcambinding[1];
			*keyop = &Config::GetInstance().keyprevcamoperator;
		break;
		case 17:
			*key1 = &Config::GetInstance().keydetonatebinding[0];
			*key2 = &Config::GetInstance().keydetonatebinding[1];
			*keyop = &Config::GetInstance().keydetonateoperator;
		break;
		case 18:
			*key1 = &Config::GetInstance().keydisguisebinding[0];
			*key2 = &Config::GetInstance().keydisguisebinding[1];
			*keyop = &Config::GetInstance().keydisguiseoperator;
		break;
		case 19:
			*key1 = &Config::GetInstance().keynextweaponbinding[0];
			*key2 = &Config::GetInstance().keynextweaponbinding[1];
			*keyop = &Config::GetInstance().keynextweaponoperator;
		break;
	}
}

const char * Game::GetKeyName(SDL_Scancode sym){
#ifdef OUYA // Custom scancodes for ouya controller
	switch((int)sym){
		case SDL_SCANCODE_LALT: return "L2"; break;
		case SDL_SCANCODE_RALT: return "R2"; break;
		case SDL_SCANCODE_HOME: return "Menu"; break;
		case SDL_SCANCODE_RETURN: return "O"; break;
		case SDL_SCANCODE_ESCAPE: return "A"; break;
		case 99: return "U"; break;
		case 100: return "Y"; break;
		case 102: return "L1"; break;
		case 103: return "R1"; break;
		case 106: return "L3"; break;
		case 107: return "R3"; break;
		case SDL_SCANCODE_KP_2: return "RUp"; break;
		case SDL_SCANCODE_KP_4: return "RLeft"; break;
		case SDL_SCANCODE_KP_6: return "RRight"; break;
		case SDL_SCANCODE_KP_8: return "RDown"; break;
	}
#endif
	switch(sym){
		case SDL_SCANCODE_UNKNOWN: return ""; break;
		case SDL_SCANCODE_UP: return "Up"; break;
		case SDL_SCANCODE_DOWN: return "Down"; break;
		case SDL_SCANCODE_LEFT: return "Left"; break;
		case SDL_SCANCODE_RIGHT: return "Right"; break;
		case SDL_SCANCODE_TAB: return "Tab"; break;
		case SDL_SCANCODE_CAPSLOCK: return "CapsLock"; break;
		case SDL_SCANCODE_RSHIFT: return "RShift"; break;
		case SDL_SCANCODE_LSHIFT: return "LShift"; break;
		case SDL_SCANCODE_RETURN: return "Enter"; break;
		case SDL_SCANCODE_SEMICOLON: return ";"; break;
		case SDL_SCANCODE_COMMA: return ","; break;
		case SDL_SCANCODE_PERIOD: return "."; break;
		case SDL_SCANCODE_LEFTBRACKET: return "("; break;
		case SDL_SCANCODE_RIGHTBRACKET: return ")"; break;
		case SDL_SCANCODE_BACKSLASH: return "Backslash"; break;
		case SDL_SCANCODE_BACKSPACE: return "Backspace"; break;
		case SDL_SCANCODE_SLASH: return "Slash"; break;
		case SDL_SCANCODE_SPACE: return "Space"; break;
		case SDL_SCANCODE_RALT: return "RAlt"; break;
		case SDL_SCANCODE_LALT: return "LAlt"; break;
		case SDL_SCANCODE_RCTRL: return "RCtrl"; break;
		case SDL_SCANCODE_LCTRL: return "LCtrl"; break;
		case SDL_SCANCODE_EQUALS: return "="; break;
		case SDL_SCANCODE_MINUS: return "Minus"; break;
		case SDL_SCANCODE_RGUI: return "RWin"; break;
		case SDL_SCANCODE_LGUI: return "LWin"; break;
		case SDL_SCANCODE_APOSTROPHE: return "'"; break;
		case SDL_SCANCODE_GRAVE: return "'"; break;
		case SDL_SCANCODE_ESCAPE: return "Escape"; break;
		case SDL_SCANCODE_INSERT: return "Insert"; break;
		case SDL_SCANCODE_HOME: return "Home"; break;
		case SDL_SCANCODE_END: return "End"; break;
		case SDL_SCANCODE_PAGEUP: return "Page Up"; break;
		case SDL_SCANCODE_PAGEDOWN: return "Page Down"; break;
		case SDL_SCANCODE_NUMLOCKCLEAR: return "NumLock"; break;
		case SDL_SCANCODE_SCROLLLOCK: return "ScrollLock"; break;
		case SDL_SCANCODE_KP_0: return "NumPad 0"; break;
		case SDL_SCANCODE_KP_1: return "NumPad 1"; break;
		case SDL_SCANCODE_KP_2: return "NumPad 2"; break;
		case SDL_SCANCODE_KP_3: return "NumPad 3"; break;
		case SDL_SCANCODE_KP_4: return "NumPad 4"; break;
		case SDL_SCANCODE_KP_5: return "NumPad 5"; break;
		case SDL_SCANCODE_KP_6: return "NumPad 6"; break;
		case SDL_SCANCODE_KP_7: return "NumPad 7"; break;
		case SDL_SCANCODE_KP_8: return "NumPad 8"; break;
		case SDL_SCANCODE_KP_9: return "NumPad 9"; break;
		case SDL_SCANCODE_KP_PERIOD: return "NumPad ."; break;
		case SDL_SCANCODE_KP_DIVIDE: return "NumPad /"; break;
		case SDL_SCANCODE_KP_ENTER: return "NumPad E"; break;
		case SDL_SCANCODE_KP_EQUALS: return "NumPad ="; break;
		case SDL_SCANCODE_KP_MINUS: return "NumPad -"; break;
		case SDL_SCANCODE_KP_MULTIPLY: return "NumPad x"; break;
		case SDL_SCANCODE_KP_PLUS: return "NumPad +"; break;
		case SDL_SCANCODE_F1: return "F1"; break;
		case SDL_SCANCODE_F2: return "F2"; break;
		case SDL_SCANCODE_F3: return "F3"; break;
		case SDL_SCANCODE_F4: return "F4"; break;
		case SDL_SCANCODE_F5: return "F5"; break;
		case SDL_SCANCODE_F6: return "F6"; break;
		case SDL_SCANCODE_F7: return "F7"; break;
		case SDL_SCANCODE_F8: return "F8"; break;
		case SDL_SCANCODE_F9: return "F9"; break;
		case SDL_SCANCODE_F10: return "F10"; break;
		case SDL_SCANCODE_F11: return "F11"; break;
		case SDL_SCANCODE_F12: return "F12"; break;
		case SDL_SCANCODE_F13: return "F13"; break;
		case SDL_SCANCODE_F14: return "F14"; break;
		case SDL_SCANCODE_F15: return "F15"; break;
		case SDL_SCANCODE_A: return "A"; break;
		case SDL_SCANCODE_B: return "B"; break;
		case SDL_SCANCODE_C: return "C"; break;
		case SDL_SCANCODE_D: return "D"; break;
		case SDL_SCANCODE_E: return "E"; break;
		case SDL_SCANCODE_F: return "F"; break;
		case SDL_SCANCODE_G: return "G"; break;
		case SDL_SCANCODE_H: return "H"; break;
		case SDL_SCANCODE_I: return "I"; break;
		case SDL_SCANCODE_J: return "J"; break;
		case SDL_SCANCODE_K: return "K"; break;
		case SDL_SCANCODE_L: return "L"; break;
		case SDL_SCANCODE_M: return "M"; break;
		case SDL_SCANCODE_N: return "N"; break;
		case SDL_SCANCODE_O: return "O"; break;
		case SDL_SCANCODE_P: return "P"; break;
		case SDL_SCANCODE_Q: return "Q"; break;
		case SDL_SCANCODE_R: return "R"; break;
		case SDL_SCANCODE_S: return "S"; break;
		case SDL_SCANCODE_T: return "T"; break;
		case SDL_SCANCODE_U: return "U"; break;
		case SDL_SCANCODE_V: return "V"; break;
		case SDL_SCANCODE_W: return "W"; break;
		case SDL_SCANCODE_X: return "X"; break;
		case SDL_SCANCODE_Y: return "Y"; break;
		case SDL_SCANCODE_Z: return "Z"; break;
		case SDL_SCANCODE_1: return "1"; break;
		case SDL_SCANCODE_2: return "2"; break;
		case SDL_SCANCODE_3: return "3"; break;
		case SDL_SCANCODE_4: return "4"; break;
		case SDL_SCANCODE_5: return "5"; break;
		case SDL_SCANCODE_6: return "6"; break;
		case SDL_SCANCODE_7: return "7"; break;
		case SDL_SCANCODE_8: return "8"; break;
		case SDL_SCANCODE_9: return "9"; break;
		case SDL_SCANCODE_0: return "0"; break;
		default: return "?"; break;
	}
}

void Game::GetGameChannelName(LobbyGame & lobbygame, char * name){
	sprintf(name, "#%s-%d", lobbygame.name, lobbygame.accountid);
}

void Game::CreateAmbienceChannels(void){
	const char * bgchannelbanks[3] = {"wndloopb.wav", "cphum11.wav", "wndloop1.wav"};
	for(int i = 0; i < sizeof(bgchannel) / sizeof(int); i++){
		if(bgchannel[i] == -1){
			bgchannel[i] = Audio::GetInstance().Play(world.resources.soundbank[bgchannelbanks[i]], 0, true);
		}
	}
}

void Game::UpdateAmbienceChannels(void){
	Player * localplayer = world.GetPeerPlayer(world.localpeerid);
	if(localplayer){
		int columns = 5;
		int rows = 5;
		int w = 640;
		int h = 480;
		int outsideamount = 0;
		int maxamount = columns * rows;
		for(int x = 0; x < columns; x++){
			for(int y = 0; y < rows; y++){
				int x1 = (w * (x / float(columns))) - (w / 2);
				x1 += - renderer.camera.GetXOffset();
				int x2 = (w * ((x + 1) / float(columns))) - (w / 2);
				x2 += - renderer.camera.GetXOffset();
				int y1 = (h * (y / float(rows))) - (h / 2);
				y1 += - renderer.camera.GetYOffset();
				int y2 = (h * ((y + 1) / float(rows))) - (h / 2);
				y2 += - renderer.camera.GetYOffset();
				if(world.map.TestAABB(x1, y1, x2, y2, Platform::OUTSIDEROOM)){
					outsideamount++;
				}
			}
		}
		if(localplayer->InBase(world)){
			Audio::GetInstance().SetVolume(bgchannel[BG_BASE], 32);
			Audio::GetInstance().SetVolume(bgchannel[BG_AMBIENT], 0);
			Audio::GetInstance().SetVolume(bgchannel[BG_OUTSIDE], 0);
		}else{
			Audio::GetInstance().SetVolume(bgchannel[BG_BASE], 0);
			Audio::GetInstance().SetVolume(bgchannel[BG_AMBIENT], 8 * (1 - (outsideamount / float(maxamount))));
			Audio::GetInstance().SetVolume(bgchannel[BG_OUTSIDE], 8 * (outsideamount / float(maxamount)));
		}
	}
}

bool Game::FadedIn(void){
	if(fade_i == 16){
		return true;
	}
	return false;
}

std::vector<std::string> Game::ListFiles(const char * directory){
	std::vector<std::string> files;
#ifdef POSIX
	DIR * dir = opendir(directory);
	if(dir){
		dirent * info;
		while((info = readdir(dir))){
			struct stat st;
			char filename[PATH_MAX];
			strcpy(filename, directory);
			strcat(filename, "/");
			strcat(filename, info->d_name);
			if(stat(filename, &st) == 0){
				if(info->d_type != DT_DIR && !S_ISDIR(st.st_mode) && info->d_name[0] != '.'){
					files.push_back(std::string(info->d_name));
				}
			}
		}
		closedir(dir);
	}
#elif _WIN32
	WIN32_FIND_DATA info;
	char directory2[MAX_PATH];
	strcpy(directory2, directory);
	strcat(directory2, "\\*");
	HANDLE dir = FindFirstFile(directory2, &info);
	if(dir){
		do{
			char fullname[MAX_PATH];
			sprintf(fullname, "%s\\%s", directory, info.cFileName);
			if(!(GetFileAttributes(fullname) & FILE_ATTRIBUTE_DIRECTORY)){
				files.push_back(std::string(info.cFileName));
			}
		}while(FindNextFile(dir, &info));
		FindClose(dir);
	}
#endif
	return files;
}

void Game::LoadRandomGameMusic(void){
	if(!Config::GetInstance().music){
		return;
	}
	if(world.resources.gamemusic){
		Mix_FreeMusic(world.resources.gamemusic);
		world.resources.gamemusic = 0;
	}
	if(!world.resources.gamemusic){
		const char * directory = "music";
		std::vector<std::string> files = ListFiles(directory);
		if(files.size() > 0){
			char filename[1024];
			strcpy(filename, directory);
			strcat(filename, "/");
			strcat(filename, files[rand() % files.size()].c_str());
			world.resources.gamemusic = Mix_LoadMUS(filename);
		}
	}
}

bool Game::HandleSDLEvents(void){
	SDL_Event event;
	while(SDL_PollEvent(&event) > 0){
		switch(event.type){
			case SDL_WINDOWEVENT:{
				switch(event.window.event){
					case SDL_WINDOWEVENT_RESIZED:{
						if(usingopengl){
							//glViewport(0, 0, event.window.data1, event.window.data2);
						}
					}break;
					case SDL_WINDOWEVENT_FOCUS_GAINED:{
						Audio::GetInstance().Unmute();
						minimized = false;
					}break;
					case SDL_WINDOWEVENT_FOCUS_LOST:{
						Audio::GetInstance().Mute(25);
						minimized = true;
					}break;
					case SDL_WINDOWEVENT_MINIMIZED:{
						minimized = true;
					}break;
					case SDL_WINDOWEVENT_MAXIMIZED:{
						minimized = false;
					}break;
					case SDL_WINDOWEVENT_RESTORED:{
						minimized = false;
					}break;
				}
			}break;
			case SDL_TEXTINPUT:{
				char ascii = event.text.text[0] & 0x7F;
				bool skip = true;
				if(ascii >= 0x20 && ascii <= 0x7F){
					skip = false;
				}
				switch(ascii){
					case '[':
					case '\\':
					case ']':
					case '^':
					case '_':
					case '`':
					case '{':
					case '|':
					case '}':
					case '~':
						skip = true;
					break;
				}
				Interface * iface = (Interface *)world.GetObjectFromId(currentinterface);
				if(iface){
					//iface->lastsym = ascii;
					if(!skip){
						iface->ProcessKeyPress(world, ascii);
					}
				}
			}break;
			case SDL_KEYDOWN:{
				if(event.key.keysym.scancode == quitscancode){
					Player * localplayer = world.GetPeerPlayer(world.localpeerid);
					if(localplayer && !localplayer->chatinterfaceid && !localplayer->buyinterfaceid){
						if(world.quitstate == 0){
							world.quitstate = 1;
						}else
						if(world.quitstate == 2){
							world.quitstate = 3;
						}
					}
				}
				keystate[event.key.keysym.scancode] = true;
				bool skip = true;
				Uint8 ascii;
				switch(event.key.keysym.scancode){
					case SDL_SCANCODE_LEFT:
						ascii = 1;
						skip = false;
					break;
					case SDL_SCANCODE_RIGHT:
						ascii = 2;
						skip = false;
					break;
					case SDL_SCANCODE_UP:
						ascii = 3;
						skip = false;
					break;
					case SDL_SCANCODE_DOWN:
						ascii = 4;
						skip = false;
					break;
					case SDL_SCANCODE_BACKSPACE:
						ascii = '\b';
						skip = false;
					break;
					case SDL_SCANCODE_TAB:
						ascii = '\t';
						skip = false;
					break;
					case SDL_SCANCODE_RETURN:
						ascii = '\n';
						skip = false;
					break;
					case SDL_SCANCODE_ESCAPE:
						ascii = 0x1B;
						skip = false;
					break;
					default:{
						if(Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keymoveupbinding, Config::GetInstance().keymoveupoperator)){
							ascii = 3;
							skip = false;
						}
						if(Config::GetInstance().KeyIsPressed(keystate, Config::GetInstance().keymovedownbinding, Config::GetInstance().keymovedownoperator)){
							ascii = 4;
							skip = false;
						}
					}break;
				}
				Interface * iface = (Interface *)world.GetObjectFromId(currentinterface);
				if(iface){
					iface->lastsym = event.key.keysym.scancode;
					if(!skip){
						iface->ProcessKeyPress(world, ascii);
					}
				}
			}break;
			case SDL_KEYUP:{
				if(event.key.keysym.scancode == quitscancode){
					if(world.quitstate == 1){
						world.quitstate = 2;
					}
					if(world.quitstate == 3){
						world.quitstate = 0;
					}
				}
				keystate[event.key.keysym.scancode] = false;
			}break;
			case SDL_MOUSEWHEEL:{
				Interface * iface = (Interface *)world.GetObjectFromId(currentinterface);
				if(iface){
					if(event.wheel.y > 0){
						iface->ProcessMouseWheelUp(world);
					}else
					if(event.wheel.y < 0){
						iface->ProcessMouseWheelDown(world);
					}
				}
			}break;
			case SDL_MOUSEBUTTONDOWN:{
				Interface * iface = (Interface *)world.GetObjectFromId(currentinterface);
				if(iface){
					if(event.button.button == SDL_BUTTON_LEFT){
						int w, h;
						SDL_GetWindowSize(window, &w, &h);
						iface->ProcessMousePress(world, true, (float(event.button.x) / w) * 640, (float(event.button.y) / h) * 480);
					}
				}
			}break;
			case SDL_MOUSEBUTTONUP:{
				if(event.button.button == SDL_BUTTON_LEFT){
					Interface * iface = (Interface *)world.GetObjectFromId(currentinterface);
					if(iface){
						int w, h;
						SDL_GetWindowSize(window, &w, &h);
						iface->ProcessMousePress(world, false, (float(event.button.x) / w) * 640, (float(event.button.y) / h) * 480);
					}
				}
			}break;
			case SDL_MOUSEMOTION:{
				Interface * iface = (Interface *)world.GetObjectFromId(currentinterface);
				if(iface){
					int w, h;
					SDL_GetWindowSize(window, &w, &h);
					iface->ProcessMouseMove(world, (float(event.button.x) / w) * 640, (float(event.button.y) / h) * 480);
				}
			}break;
			case SDL_QUIT:
				return false;
			break;
		}
	}
	return true;
}