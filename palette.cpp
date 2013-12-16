#include "palette.h"
#include <cmath>

Palette::Palette(){
	currentpalette = 0;
	for(int i = 0; i < 11; i++){
		brightness[i] = 0;
		colored[i] = 0;
		alphaed[i] = 0;
	}
}

Palette::~Palette(){
	for(int i = 0; i < 11; i++){
		if(brightness[i]){
			delete[] brightness[i];
		}
		if(colored[i]){
			delete[] colored[i];
		}
		if(alphaed[i]){
			delete[] alphaed[i];
		}
	}
}

bool Palette::Load(void){
	if(!brightness[currentpalette]){
		brightness[currentpalette] = new Uint8[256 * 256];
		memset(brightness[currentpalette], 0, 256 * 256);
	}
	if(!colored[currentpalette]){
		colored[currentpalette] = new Uint8[256 * 256];
		memset(colored[currentpalette], 0, 256 * 256);
	}
	if(!alphaed[currentpalette]){
		alphaed[currentpalette] = new Uint8[256 * 256];
		memset(alphaed[currentpalette], 0, 256 * 256);
	}
	currentalphaedpalette = alphaed[currentpalette];
	currentbrightnesspalette = brightness[currentpalette];
	currentcoloredpalette = colored[currentpalette];
	char filename[256];
	sprintf(filename, "PALETTECALC%d.BIN", currentpalette);
	/*system("rm PALETTECALC0.BIN");
	system("rm PALETTECALC1.BIN");
	system("rm PALETTECALC2.BIN");*/
	SDL_RWops * file = SDL_RWFromFile("PALETTE.BIN", "rb");
	SDL_RWops * filec = SDL_RWFromFile(filename, "rb");
	if(file){
		for(int offset = 0; offset < 11; offset++){
			SDL_RWseek(file, (offset * (768 + 4)) + 4, RW_SEEK_SET);
			for(unsigned int i = 0; i < 256; i++){
				Uint8 b, g, r;
				SDL_RWread(file, &r, 1, 1);
				SDL_RWread(file, &g, 1, 1);
				SDL_RWread(file, &b, 1, 1);
				colors[offset][i].b = b << 2;
				colors[offset][i].g = g << 2;
				colors[offset][i].r = r << 2;
			}
		}
		SDL_RWclose(file);
		if(filec){
			for(unsigned int i = 0; i < 256; i++){
				SDL_RWread(filec, &brightness[currentpalette][(i * 256)], 256, 1);
				SDL_RWread(filec, &colored[currentpalette][(i * 256)], 256, 1);
				SDL_RWread(filec, &alphaed[currentpalette][(i * 256)], 256, 1);
			}
			SDL_RWclose(filec);
		}else{
			printf("%s not found, calculating lookup tables...", filename);
			Calculate(2, 256 - 30);
			Save();
		}
		return true;
	}else{
		printf("Could not load PALETTE.BIN");
		return false;
	}
}

void Palette::SetParallaxColors(Uint8 parallax){
	Uint8 parallaxpalette = 5;
	switch(parallax){
		case 0:
			parallaxpalette = 5;
		break;
		case 1:
			parallaxpalette = 6;
		break;
		case 2:
			parallaxpalette = 7;
		break;
		case 3:
			parallaxpalette = 8;
		break;
		case 4:
			parallaxpalette = 9;
		break;
	}
	for(unsigned int i = 256 - 30; i < 256; i++){
		colors[currentpalette][i] = colors[parallaxpalette][i];
	}
	Calculate(256 - 30, 255);
}

bool Palette::SetPalette(Uint8 palette){
	if(palette >= 11){
		return false;
	}
	currentpalette = palette;
	currentalphaedpalette = alphaed[currentpalette];
	currentbrightnesspalette = brightness[currentpalette];
	currentcoloredpalette = colored[currentpalette];
	return Load();
}

SDL_Color * Palette::GetColors(void){
	return colors[currentpalette];
}

void Palette::Save(void){
	char filename[256];
	sprintf(filename, "PALETTECALC%d.BIN", currentpalette);
	SDL_RWops * file = SDL_RWFromFile(filename, "wb");
	for(unsigned int i = 0; i < 256; i++){
		SDL_RWwrite(file, &brightness[currentpalette][(i * 256)], 256, 1);
		SDL_RWwrite(file, &colored[currentpalette][(i * 256)], 256, 1);
		SDL_RWwrite(file, &alphaed[currentpalette][(i * 256)], 256, 1);
	}
	SDL_RWclose(file);
}

Uint8 Palette::ClosestMatch(SDL_Color color, bool upperonly){
	Uint8 choice = 0;
	int smallestdiff = (256 * 256) + (256 * 256) + (256 * 256);
	int start = 2;
	int end = 256;
	if(upperonly && currentpalette == 0){
		start = 114;
	}
	for(unsigned int i = start; i < end; i++){
		int x = colors[currentpalette][i].r - color.r;
		int y = colors[currentpalette][i].g - color.g;
		int z = colors[currentpalette][i].b - color.b;
		int diff = (x * x) + (y * y) + (z * z);
		//diff += abs(colors[currentpalette][i].r - color.r);
		//diff += abs(colors[currentpalette][i].g - color.g);
		//diff += abs(colors[currentpalette][i].b - color.b);
		if(diff < smallestdiff){
			smallestdiff = diff;
			choice = i;
		}
	}
	//char temp[1234];
	//sprintf(temp, "Closest match with (%d, %d, %d) is palette index %d -> (%d, %d, %d)", color.r, color.g, color.b, choice, colors[choice].r, colors[choice].g, colors[choice].b);
	//MessageBoxA(NULL, temp, temp, MB_OK);
	return choice;
}

void Palette::Calculate(Uint8 begin, Uint8 end){
	for(unsigned int i = 2; i < 256; i++){
		bool upperonly = false;
		if(i >= 114){
			upperonly = true;
		}
		float alpha = ((i - 2) % 16) / float(16);
		if(alpha > 0.5){
			alpha = 1;
		}
		for(unsigned int j = begin; j <= end; j++){
			Uint8 intensity = j;
			if(intensity > 128 || currentpalette != 0){
				brightness[currentpalette][(i * 256) + j] = ClosestMatch(Brightness(colors[currentpalette][i], intensity), upperonly);
			}else{
				int originalintensity = ((i - 2) % 16);
				int colorramp = (((i - 2) / 16) * 16);
				int index = originalintensity * (((intensity / 8)) / 16.0);
				if(index > 15){
					index = 15;
				}
				index += colorramp + 2;
				if(colorramp == 16){
					brightness[currentpalette][(i * 256) + j] = ClosestMatch(Brightness(colors[currentpalette][i], intensity), upperonly);
				}else{
					brightness[currentpalette][(i * 256) + j] = index;
				}
			}
			colored[currentpalette][(j * 256) + i] = ClosestMatch(Color(colors[currentpalette][j], colors[currentpalette][i]), upperonly);
			alphaed[currentpalette][(i * 256) + j] = ClosestMatch(Alpha(colors[currentpalette][i], colors[currentpalette][j], alpha), upperonly);
		}
	}
}

void Palette::CalculateLighted(Uint8 ambiencelevel){
	for(int i = 0; i < 256; i++){
		for(int j = 0; j < 16; j++){
			Uint8 * entry = &currentlightedpalette[(i * 16) + j];
			if(i <= 1){
				*entry = i;
			}else{
				if(i >= 114){
					*entry = i;
				}else{
					int lum = (j * 8) + ambiencelevel;
					if(lum > 128){
						lum = 128;
					}
					if(lum == 0){
						*entry = i;
					}else{
						Uint8 newcolor = Brightness(i, lum);
						int newcolorbrightness = (newcolor - 2) % 16;
						Uint8 oldcolorbrightness = (i - 2) % 16;
						if(newcolorbrightness >= oldcolorbrightness * (float(ambiencelevel) / 128)){
							*entry = newcolor + 112;
						}else{
							*entry = i;
							//Uint8 newcolor2 = Brightness(i, ambiencelevel - 8);
							//*entry = newcolor2 + 112;
						}
					}
				}
			}
		}
	}
}

SDL_Color * Palette::CopyWithBrightness(SDL_Color * palette, Uint8 brightness, Uint8 begin, Uint8 end){
	for(int i = begin; i <= end; i++){
		temppalette[i] = Brightness(palette[i], brightness);
	}
	return temppalette;
}

SDL_Color * Palette::GetTempPalette(void){
	return temppalette;
}

SDL_Color Palette::HSVtoRGB(Palette::HSV hsv){
    SDL_Color rgb;
    unsigned char region, remainder, p, q, t;
	
    if(hsv.s == 0){
        rgb.r = hsv.v;
        rgb.g = hsv.v;
        rgb.b = hsv.v;
        return rgb;
    }
	
    region = hsv.h / 43;
    remainder = (hsv.h - (region * 43)) * 6;
	
    p = (hsv.v * (255 - hsv.s)) >> 8;
    q = (hsv.v * (255 - ((hsv.s * remainder) >> 8))) >> 8;
    t = (hsv.v * (255 - ((hsv.s * (255 - remainder)) >> 8))) >> 8;
	
    switch(region){
        case 0:
            rgb.r = hsv.v; rgb.g = t; rgb.b = p;
		break;
        case 1:
            rgb.r = q; rgb.g = hsv.v; rgb.b = p;
		break;
        case 2:
            rgb.r = p; rgb.g = hsv.v; rgb.b = t;
		break;
        case 3:
            rgb.r = p; rgb.g = q; rgb.b = hsv.v;
		break;
        case 4:
            rgb.r = t; rgb.g = p; rgb.b = hsv.v;
		break;
        default:
            rgb.r = hsv.v; rgb.g = p; rgb.b = q;
		break;
    }
	
    return rgb;
}

Palette::HSV Palette::RGBtoHSV(SDL_Color rgb){
	Palette::HSV hsv;
    unsigned char rgbMin, rgbMax;
	
    rgbMin = rgb.r < rgb.g ? (rgb.r < rgb.b ? rgb.r : rgb.b) : (rgb.g < rgb.b ? rgb.g : rgb.b);
    rgbMax = rgb.r > rgb.g ? (rgb.r > rgb.b ? rgb.r : rgb.b) : (rgb.g > rgb.b ? rgb.g : rgb.b);
	
    hsv.v = rgbMax;
    if(hsv.v == 0){
        hsv.h = 0;
        hsv.s = 0;
        return hsv;
    }
	
    hsv.s = 255 * long(rgbMax - rgbMin) / hsv.v;
    if(hsv.s == 0){
        hsv.h = 0;
        return hsv;
    }
	
    if(rgbMax == rgb.r){
        hsv.h = 0 + 43 * (rgb.g - rgb.b) / (rgbMax - rgbMin);
    }else
	if(rgbMax == rgb.g){
        hsv.h = 85 + 43 * (rgb.b - rgb.r) / (rgbMax - rgbMin);
	}else{
        hsv.h = 171 + 43 * (rgb.r - rgb.g) / (rgbMax - rgbMin);
	}
	
    return hsv;
}

Palette::HSL Palette::RGBtoHSL(SDL_Color rgb){
	float r = float(rgb.r) / 255;
	float g = float(rgb.g) / 255;
	float b = float(rgb.b) / 255;
    float max = r;
	if(g > max){
		max = g;
	}
	if(b > max){
		max = b;
	}
	float min = r;
	if(g < min){
		min = g;
	}
	if(b < min){
		min = b;
	}
	
    float h, s, l = (max + min) / 2;
	
    if(max == min){
        h = s = 0; // achromatic
    }else{
        float d = max - min;
        s = l > 0.5 ? d / (2 - max - min) : d / (max + min);
		if(max == r){
			h = (g - b) / d + (g < b ? 6 : 0);
		}
		if(max == g){
			h = (b - r) / d + 2;
		}
		if(max == b){
			h = (r - g) / d + 4;
		}
        h /= 6;
    }
	
	HSL hsl;
	hsl.h = h;
	hsl.s = s;
	hsl.l = l;
    return hsl;
}

SDL_Color Palette::HSLtoRGB(Palette::HSL hsl){
	float r, g, b;
	
    if(hsl.s == 0){
        r = g = b = hsl.l; // achromatic
    }else{
        float q = hsl.l < 0.5 ? hsl.l * (1 + hsl.s) : hsl.l + hsl.s - hsl.l * hsl.s;
        float p = 2 * hsl.l - q;
        r = HueToRGB(p, q, hsl.h + float(1.0/3.0));
        g = HueToRGB(p, q, hsl.h);
        b = HueToRGB(p, q, hsl.h - float(1.0/3.0));
    }
	
	SDL_Color rgb;
	rgb.r = r * 255;
	rgb.g = g * 255;
	rgb.b = b * 255;
	
    return rgb;
}

float Palette::HueToRGB(float p, float q, float t){
	if(t < 0) t += 1;
	if(t > 1) t -= 1;
	if(t < float(1.0/6.0)) return p + (q - p) * 6 * t;
	if(t < float(1.0/2.0)) return q;
	if(t < float(2.0/3.0)) return p + (q - p) * (float(2.0/3.0) - t) * 6;
	return p;
}

Uint8 Palette::Lum(SDL_Color color){
	return (float(color.r) * 0.3) + (float(color.g) * 0.59) + (float(color.b) * 0.11);
}

SDL_Color Palette::Mix(SDL_Color a, SDL_Color b){
	SDL_Color mixed;
	mixed.r = (a.r + b.r) / 2;
	mixed.g = (a.g + b.g) / 2;
	mixed.b = (a.b + b.b) / 2;
	return mixed;
}

SDL_Color Palette::Brightness(SDL_Color a, Uint8 i){
	SDL_Color brightness;
	
	if(i > 128){
		float percent = float(i - 127) / 128;
		float percentr = 1 - percent;
		brightness.r = (a.r * percentr) + (255 * percent);
		brightness.g = (a.g * percentr) + (255 * percent);
		brightness.b = (a.b * percentr) + (255 * percent);
	}else
	if(i < 128){
		float percent = float(i) / 128;
		brightness.r = (a.r * percent);
		brightness.g = (a.g * percent);
		brightness.b = (a.b * percent);
	}else{
		brightness = a;
	}
	/*Sint16 lr, lg, lb;
	lr = Sint16(a.r * ((float)b.r / 255));
	lg = Sint16(a.g * ((float)b.g / 255));
	lb = Sint16(a.b * ((float)b.b / 255));
	brightness.r = (Uint8)lr;
	brightness.g = (Uint8)lg;
	brightness.b = (Uint8)lb;*/
	return brightness;
}

SDL_Color Palette::Color(SDL_Color a, SDL_Color b){
	SDL_Color colored;
	Uint8 luma = Lum(a);
	Uint8 lumb = Lum(b);
	int diff = luma - lumb;
	int cr = b.r + diff; if(cr > 255) cr = 255; if(cr < 0) cr = 0;
	int cg = b.g + diff; if(cg > 255) cg = 255; if(cg < 0) cg = 0;
	int cb = b.b + diff; if(cb > 255) cb = 255; if(cb < 0) cb = 0;
	colored.r = cr;
	colored.g = cg;
	colored.b = cb;
	//printf("%d %d %d = %d %d %d + %d %d %d\n", tinted.r, tinted.g, tinted.b, a.r, a.g, a.b, b.r, b.g, b.b);
	return colored;
}

SDL_Color Palette::Alpha(SDL_Color a, SDL_Color b, float alpha){
	SDL_Color alphaed;
	//float alpha = float((a.r + a.g + a.b) / 3) / 255;
	alphaed.r = (a.r * alpha) + (b.r * (1 - alpha));
	alphaed.g = (a.g * alpha) + (b.g * (1 - alpha));
	alphaed.b = (a.b * alpha) + (b.b * (1 - alpha));
	return alphaed;
}