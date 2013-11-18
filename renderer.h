#ifndef RENDERER_H
#define RENDERER_H

#include "world.h"
#include "resources.h"
#include "palette.h"
#include "camera.h"
#include "textinput.h"

class Renderer
{
public:
	Renderer(class World & world);
	struct Rect { int w, h, x, y; };
	void Tick(void);
	void Draw(SDL_Surface * surface, float frametime = 0);
	void DrawWorld(SDL_Surface * surface, Camera & camera, Uint8 * lightmap, bool drawminimap = true, int recursion = 2, float frametime = 0);
	void DrawMiniMap(Object * object);
	void DrawWorldScaled(SDL_Surface * surface, Camera & camera, int recursion, float frametime = 0, int factor = 2);
	void BlitSurface(SDL_Surface * src, Rect * srcrect, SDL_Surface * dst, Rect * dstrect);
	bool BlitSurfaceUpper(SDL_Surface * src, Rect * srcrect, SDL_Surface * dst, Rect * dstrect);
	void BlitSurfaceSlow(SDL_Surface * src, Rect * srcrect, SDL_Surface * dst, Rect * dstrect);
	void BlitSurfaceFast(SDL_Surface * src, Rect * srcrect, SDL_Surface * dst, Rect * dstrect);
	void BlitSprite(Object * object, Camera & camera, SDL_Surface * dst, Rect * dstrect, SDL_Surface * src, Rect * srcrect);
	void DrawText(SDL_Surface * surface, Uint16 x, Uint16 y, const char * text, Uint8 bank, Uint8 width, bool alpha = false, Uint8 tint = 0, Uint8 brightness = 128, bool rampcolor = false);
	void DrawTextInput(SDL_Surface * surface, TextInput & textinput);
	void DrawTinyText(SDL_Surface * surface, Uint16 x, Uint16 y, const char * text, Uint8 tint = 0, Uint8 brightness = 128);
	void DrawShadow(SDL_Surface * surface, Object * object);
	void DrawRain(SDL_Surface * surface, Camera & camera, float frametime = 0);
	void DrawRainPuddles(SDL_Surface * surface, Camera & camera);
	inline void SetPixel(SDL_Surface * surface, unsigned int x, unsigned int y, Uint8 color);
	inline Uint8 GetPixel(SDL_Surface * surface, unsigned int x, unsigned int y);
	void DrawDebug(SDL_Surface * surface);
	void DrawMessage(SDL_Surface * surface);
	void DrawStatus(SDL_Surface * surface);
	void DrawScaled(SDL_Surface * src, Rect * srcrect, SDL_Surface *dst, Rect * dstrect, int factor = 2);
	void DrawCheckered(SDL_Surface * src, Rect * srcrect, SDL_Surface * dst, Rect * dstrect);
	void DrawColored(SDL_Surface * src, Rect * srcrect, SDL_Surface * dst, Rect * dstrect);
	void DrawRampColored(SDL_Surface * src, Rect * srcrect, SDL_Surface * dst, Rect * dstrect);
	void DrawBrightened(SDL_Surface * src, Rect * srcrect, SDL_Surface * dst, Rect * dstrect, Uint8 brightness);
	void DrawAlphaed(SDL_Surface * src, Rect * srcrect, SDL_Surface * dst, Rect * dstrect);
	SDL_Surface * CopySurfaceWithOffset(SDL_Surface * src, Uint8 offset);
	SDL_Surface * CreateSurfaceCopy(SDL_Surface * src);
	SDL_Surface * CreateSurface(Uint32 width, Uint32 height);
	SDL_Surface * CreateSameSizeSurface(SDL_Surface * src);
	void EffectHacking(SDL_Surface * dst, Rect * dstrect, Uint8 color);
	void EffectTeamColor(SDL_Surface * dst, Rect * dstrect, Uint8 values);
	Uint8 TeamColorToIndex(Uint8 values);
	void EffectBrightness(SDL_Surface * dst, Rect * dstrect, Uint8 brightness);
	void EffectColor(SDL_Surface * dst, Rect * dstrect, Uint8 color);
	void EffectRampColor(SDL_Surface * dst, Rect * dstrect, Uint8 color);
	void EffectHit(SDL_Surface * dst, Rect * dstrect, Uint8 hitx, Uint8 hity, Uint8 state_hit);
	void EffectShieldDamage(SDL_Surface * dst, Rect * dstrect, Uint8 color);
	void EffectWarp(SDL_Surface * dst, Rect * dstrect, Uint8 state_warp);
	void MiniMapBlit(Uint8 res_bank, Uint8 res_index, int x, int y, bool alpha = false, Uint8 teamcolor = 0);
	void MiniMapCircle(int x, int y, Uint8 color);
	void DrawMirrored(SDL_Surface * src, Rect * srcrect, SDL_Surface * dst, Rect * dstrect);
	void ApplyLighting(SDL_Surface * surface, SDL_Surface * src, Rect * Rect, Uint8 * lightmap, bool clear = false);
	void ApplyAmbience(SDL_Surface * surface, Uint8 * lightmap);
	void DrawTile(SDL_Surface * surface, SDL_Surface * tile, Rect * Rect);
	void DrawParallax(SDL_Surface * surface, Camera & camera);
	void DrawBackground(SDL_Surface * surface, Camera & camera, Uint8 * lightmap);
	void DrawForeground(SDL_Surface * surface, Camera & camera, Uint8 * lightmap);
	void DrawHUD(SDL_Surface * surface, float frametime = 0);
	void DrawMessageBackground(SDL_Surface * surface, Rect * dstrect);
	void DrawLine(SDL_Surface * surface, int x1, int y1, int x2, int y2, Uint8 color, int thickness = 1);
	void DrawFilledRectangle(SDL_Surface * surface, int x1, int y1, int x2, int y2, Uint8 color);
	void DrawCircle(SDL_Surface * surface, int x, int y, int radius, Uint8 color);
	Uint8 InvIdToResIndex(Uint8 id);
	Camera camera;
	class World & world;
	Palette palette;
	Player * localplayer;
	Uint8 lightmap[640 * 480];
	Sint8 ambience_r;
	Uint8 state_i;
	Uint8 ex, ey;
	bool playerinbaseold;
	static const int raindropscount = 100;
	int raindropsx[raindropscount];
	int raindropsy[raindropscount];
	int raindropsoldx[raindropscount];
	int raindropsoldy[raindropscount];
	static const Uint8 enemycolor = (8 << 4) + 10;
	static const Uint8 teamcolor = (8 << 4) + 13;
};

#endif