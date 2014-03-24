#ifndef RENDERER_H
#define RENDERER_H

#include "world.h"
#include "resources.h"
#include "palette.h"
#include "camera.h"
#include "textinput.h"
#include "surface.h"

class Renderer
{
private:
	struct Rect { int w, h, x, y; };

public:
	Renderer(class World & world);
	void Tick(void);
	void Draw(Surface * surface, float frametime = 0);
	void DrawWorld(Surface * surface, Camera & camera, bool drawminimap = true, bool drawluminance = true, int recursion = 2, float frametime = 0);
	void DrawMiniMap(Object * object);
	void DrawWorldScaled(Surface * surface, Camera & camera, int recursion, float frametime = 0, int factor = 2);
	static void BlitSurface(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	void BlitSprite(Object * object, Camera & camera, Surface * dst, Rect * dstrect, Surface * src, Rect * srcrect);
	static void DrawFilledRectangle(Surface * surface, int x1, int y1, int x2, int y2, Uint8 color);
	void DrawText(Surface * surface, Uint16 x, Uint16 y, const char * text, Uint8 bank, Uint8 width, bool alpha = false, Uint8 tint = 0, Uint8 brightness = 128, bool rampcolor = false);
	void DrawTextInput(Surface * surface, TextInput & textinput);
	void DrawTinyText(Surface * surface, Uint16 x, Uint16 y, const char * text, Uint8 tint = 0, Uint8 brightness = 128);
	void DrawShadow(Surface * surface, Object * object);
	void DrawRain(Surface * surface, Camera & camera, float frametime = 0);
	void DrawRainPuddles(Surface * surface, Camera & camera);
	static inline void SetPixel(Surface * surface, unsigned int x, unsigned int y, Uint8 color);
	static inline Uint8 GetPixel(Surface * surface, unsigned int x, unsigned int y);
	void DrawDebug(Surface * surface);
	void DrawMessage(Surface * surface);
	void DrawStatus(Surface * surface);
	static void DrawScaled(Surface * src, Rect * srcrect, Surface *dst, Rect * dstrect, int factor = 2);
	static void DrawCheckered(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	void DrawColored(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	void DrawRampColored(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	void DrawBrightened(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect, Uint8 brightness);
	void DrawAlphaed(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	Surface * CreateSurfaceCopy(Surface * src);
	void EffectHacking(Surface * dst, Rect * dstrect, Uint8 color);
	void EffectTeamColor(Surface * dst, Rect * dstrect, Uint8 values, bool robot = false);
	Uint8 TeamColorToIndex(Uint8 values);
	void EffectBrightness(Surface * dst, Rect * dstrect, Uint8 brightness);
	void EffectColor(Surface * dst, Rect * dstrect, Uint8 color);
	void EffectRampColor(Surface * dst, Rect * dstrect, Uint8 color);
	void EffectRampColorPlus(Surface * dst, Rect * dstrect, Uint8 color, Uint8 plus);
	void EffectHit(Surface * dst, Rect * dstrect, Uint8 hitx, Uint8 hity, Uint8 state_hit);
	void EffectShieldDamage(Surface * dst, Rect * dstrect, Uint8 color);
	void EffectWarp(Surface * dst, Rect * dstrect, Uint8 state_warp);
	void MiniMapBlit(Uint8 res_bank, Uint8 res_index, int x, int y, bool alpha = false, Uint8 teamcolor = 0);
	void MiniMapCircle(int x, int y, Uint8 color);
	static void DrawMirrored(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	void DrawLine(Surface * surface, int x1, int y1, int x2, int y2, Uint8 color, int thickness = 1);
	void DrawCircle(Surface * surface, int x, int y, int radius, Uint8 color);
	Uint8 InvIdToResIndex(Uint8 id);
	static const char * InvIdToLetter(Uint8 id);
	static bool BlitSurfaceUpper(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	static void BlitSurfaceSlow(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	static void BlitSurfaceFast(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	static void BlitSurfaceRLE(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	static void BlitSurfaceRLEClipped(int w, Uint8 * srcbuf, Rect * srcrect, Surface * dst, Rect * dstrect);
	void DrawLight(Surface * surface, Surface * src, Rect * Rect);
	static void DrawTile(Surface * surface, Surface * tile, Rect * Rect);
	void DrawParallax(Surface * surface, Camera & camera);
	void DrawBackground(Surface * surface, Camera & camera, bool drawluminance = true);
	void DrawForeground(Surface * surface, Camera & camera);
	void DrawForegroundLuminance(Surface * surface, Camera & camera);
	void DrawHUD(Surface * surface, float frametime = 0);
	void DrawMessageBackground(Surface * surface, Rect * dstrect);
	Uint8 GetAmbienceLevel(void);
	Camera camera;
	Palette palette;

private:
	class World & world;
	Player * localplayer;
	Sint8 ambience_r;
	Uint8 ambiencelevel;
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