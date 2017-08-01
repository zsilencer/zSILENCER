#ifndef PALETTE_H
#define PALETTE_H

#include "shared.h"
#include <vector>

class Palette
{
public:
	Palette();
	~Palette();
	bool Load(void);
	void SetParallaxColors(Uint8 parallax);
	bool SetPalette(Uint8 palette);
	SDL_Color * GetColors(void);
	Uint8 ClosestMatch(SDL_Color color, bool upperonly = false);
	inline Uint8 Brightness(Uint8 a, Uint8 i){
		return (*currentbrightnesspalette)[(a * 256) + i];
	}
	inline Uint8 Color(Uint8 a, Uint8 b){
		return (*currentcoloredpalette)[(a * 256) + b];
	}
	inline Uint8 Alpha(Uint8 a, Uint8 b){
		return (*currentalphaedpalette)[(a * 256) + b];
	}
	inline Uint8 Light(Uint8 a, Uint8 i){
		return currentlightedpalette[(a * 16) + (i % 16)];
	}
	inline Uint8 RampColor(Uint8 a, Uint8 b){
		if(a <= 1){
			a = 5;
		}
		if(a >= 256 - 30){
			return Color(a, b);
		}
		return ((a - 2) % 16) + (((b - 2) / 16) * 16) + 2;
	}
	inline Uint8 RampColorMin(Uint8 a, Uint8 b, Uint8 min = 8){
		return (((a - 2) % 16) | min) + (((b - 2) / 16) * 16) + 2;
	}
	inline Uint8 RampColorPlus(Uint8 a, Uint8 b, Uint8 min = 8){
		Uint8 l = ((a - 2) % 16) + min;
		if(l > 15){
			l = 15;
		}
		return (l) + (((b - 2) / 16) * 16) + 2;
	}
	SDL_Color * CopyWithBrightness(SDL_Color * palette, Uint8 brightness, Uint8 begin = 0, Uint8 end = 255);
	SDL_Color * GetTempPalette(void);
	void CalculateLighted(Uint8 ambiencelevel);
	SDL_Color colors[11][256];

private:
	struct HSV { Uint8 h, s, v; };
	struct HSL { float h, s, l; };
	void Calculate(Uint8 begin, Uint8 end);
	void Save(void);
	SDL_Color HSVtoRGB(HSV hsv);
	HSV RGBtoHSV(SDL_Color rgb);
	SDL_Color HSLtoRGB(HSL hsl);
	HSL RGBtoHSL(SDL_Color rgb);
	float HueToRGB(float p, float q, float t);
	Uint8 Lum(SDL_Color color);
	SDL_Color Mix(SDL_Color a, SDL_Color b);
	SDL_Color Brightness(SDL_Color a, Uint8 i);
	SDL_Color Color(SDL_Color a, SDL_Color b);
	SDL_Color Alpha(SDL_Color a, SDL_Color b, float alpha);
	SDL_Color temppalette[256];
	std::vector<Uint8> brightness[11];
	std::vector<Uint8> colored[11];
	std::vector<Uint8> alphaed[11];
	Uint8 currentpalette;
	std::vector<Uint8> * currentbrightnesspalette;
	std::vector<Uint8> * currentcoloredpalette;
	std::vector<Uint8> * currentalphaedpalette;
	Uint8 currentlightedpalette[256 * 16];
};

#endif