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
public:
	Renderer(class World & world);
	struct Rect { int w, h, x, y; };
	void Tick(void);
	void Draw(Surface * surface, float frametime = 0);
	void DrawWorld(Surface * surface, Camera & camera, Uint8 * lightmap, bool drawminimap = true, int recursion = 2, float frametime = 0);
	void DrawMiniMap(Object * object);
	void DrawWorldScaled(Surface * surface, Camera & camera, int recursion, float frametime = 0, int factor = 2);
	void BlitSurface(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	bool BlitSurfaceUpper(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	void BlitSurfaceSlow(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	void BlitSurfaceFast(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	void BlitSurfaceRLE(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	void BlitSprite(Object * object, Camera & camera, Surface * dst, Rect * dstrect, Surface * src, Rect * srcrect);
	void DrawText(Surface * surface, Uint16 x, Uint16 y, const char * text, Uint8 bank, Uint8 width, bool alpha = false, Uint8 tint = 0, Uint8 brightness = 128, bool rampcolor = false);
	void DrawTextInput(Surface * surface, TextInput & textinput);
	void DrawTinyText(Surface * surface, Uint16 x, Uint16 y, const char * text, Uint8 tint = 0, Uint8 brightness = 128);
	void DrawShadow(Surface * surface, Object * object);
	void DrawRain(Surface * surface, Camera & camera, float frametime = 0);
	void DrawRainPuddles(Surface * surface, Camera & camera);
	inline void SetPixel(Surface * surface, unsigned int x, unsigned int y, Uint8 color);
	inline Uint8 GetPixel(Surface * surface, unsigned int x, unsigned int y);
	void DrawDebug(Surface * surface);
	void DrawMessage(Surface * surface);
	void DrawStatus(Surface * surface);
	void DrawScaled(Surface * src, Rect * srcrect, Surface *dst, Rect * dstrect, int factor = 2);
	void DrawCheckered(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	void DrawColored(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	void DrawRampColored(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	void DrawBrightened(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect, Uint8 brightness);
	void DrawAlphaed(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	//Surface * CopySurfaceWithOffset(Surface * src, Uint8 offset);
	Surface * CreateSurfaceCopy(Surface * src);
	//Surface * CreateSurface(Uint32 width, Uint32 height);
	//Surface * CreateSameSizeSurface(Surface * src);
	void EffectHacking(Surface * dst, Rect * dstrect, Uint8 color);
	void EffectTeamColor(Surface * dst, Rect * dstrect, Uint8 values);
	Uint8 TeamColorToIndex(Uint8 values);
	void EffectBrightness(Surface * dst, Rect * dstrect, Uint8 brightness);
	void EffectColor(Surface * dst, Rect * dstrect, Uint8 color);
	void EffectRampColor(Surface * dst, Rect * dstrect, Uint8 color);
	void EffectHit(Surface * dst, Rect * dstrect, Uint8 hitx, Uint8 hity, Uint8 state_hit);
	void EffectShieldDamage(Surface * dst, Rect * dstrect, Uint8 color);
	void EffectWarp(Surface * dst, Rect * dstrect, Uint8 state_warp);
	void MiniMapBlit(Uint8 res_bank, Uint8 res_index, int x, int y, bool alpha = false, Uint8 teamcolor = 0);
	void MiniMapCircle(int x, int y, Uint8 color);
	void DrawMirrored(Surface * src, Rect * srcrect, Surface * dst, Rect * dstrect);
	void DrawLight(Surface * surface, Surface * src, Rect * Rect, Uint8 * lightmap, bool clear = false);
	void ApplyLightmap(Surface * surface, Uint8 * lightmap);
	void DrawTile(Surface * surface, Surface * tile, Rect * Rect);
	void DrawParallax(Surface * surface, Camera & camera);
	void DrawBackground(Surface * surface, Camera & camera, Uint8 * lightmap);
	void DrawForeground(Surface * surface, Camera & camera, Uint8 * lightmap);
	void DrawHUD(Surface * surface, float frametime = 0);
	void DrawMessageBackground(Surface * surface, Rect * dstrect);
	void DrawLine(Surface * surface, int x1, int y1, int x2, int y2, Uint8 color, int thickness = 1);
	void DrawFilledRectangle(Surface * surface, int x1, int y1, int x2, int y2, Uint8 color);
	void DrawCircle(Surface * surface, int x, int y, int radius, Uint8 color);
	Uint8 InvIdToResIndex(Uint8 id);
	Camera camera;
	class World & world;
	Palette palette;
	Player * localplayer;
	Uint8 lightmap[640 * 480];
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