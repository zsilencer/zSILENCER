#include "resources.h"
#include <vector>
#include "sdl_internal.h"

Resources::Resources(){
	spritebank = new SDL_Surface**[256];
	tilebank = new SDL_Surface**[256];
	tileflippedbank = new SDL_Surface**[256];
	spriteoffsetx = new int*[256];
	spriteoffsety = new int*[256];
	spritewidth = new unsigned int*[256];
	spriteheight = new unsigned int*[256];
	for(int i = 0; i < 256; i++){
		spritebank[i] = new SDL_Surface*[256];
		tilebank[i] = new SDL_Surface*[256];
		tileflippedbank[i] = new SDL_Surface*[256];
		spriteoffsetx[i] = new int[256];
		spriteoffsety[i] = new int[256];
		spritewidth[i] = new unsigned int[256];
		spriteheight[i] = new unsigned int[256];
		for(int j = 0; j < 256; j++){
			spritebank[i][j] = 0;
			tilebank[i][j] = 0;
			tileflippedbank[i][j] = 0;
			spriteoffsetx[i][j] = 0;
			spriteoffsety[i][j] = 0;
			spritewidth[i][j] = 0;
			spriteheight[i][j] = 0;
		}
	}
	music = 0;
}

Resources::~Resources(){
	for(int i = 0; i < 256; i++){
		for(int j = 0; j < 256; j++){
			if(spritebank[i][j] && spritebank[i][j] != (SDL_Surface *)true){
				SDL_FreeSurface(spritebank[i][j]);
			}
			if(tilebank[i][j]){
				SDL_FreeSurface(tilebank[i][j]);
			}
			if(tileflippedbank[i][j]){
				SDL_FreeSurface(tileflippedbank[i][j]);
			}
		}
		delete[] spritebank[i];
		delete[] tilebank[i];
		delete[] tileflippedbank[i];
		delete[] spriteoffsetx[i];
		delete[] spriteoffsety[i];
		delete[] spritewidth[i];
		delete[] spriteheight[i];
	}
	delete[] spritebank;
	delete[] tilebank;
	delete[] tileflippedbank;
	delete[] spriteoffsetx;
	delete[] spriteoffsety;
	delete[] spritewidth;
	delete[] spriteheight;
}

bool Resources::Load(bool dedicatedserver){
	if(!LoadSprites(dedicatedserver)){
		return false;
	}
	if(!LoadTiles(dedicatedserver)){
		return false;
	}
	if(!LoadSounds(dedicatedserver)){
		//return false;
	}
	return true;
}

bool Resources::LoadSprites(bool dedicatedserver){
	char FileName[256];
	SDL_RWops * file = SDL_RWFromFile("BIN_SPR.DAT", "rb");
	if(!file){
		printf("Could not open BIN_SPR.DAT\n");
		return false;
	}
	for(unsigned int i = 0; i < 256; i++){
		Uint8 header[64];
		SDL_RWread(file, header, sizeof(header), 1);
		if(header[2]){
			sprintf(FileName, "bin_spr/SPR_%.3d.BIN", i);
			SDL_RWops * file2 = SDL_RWFromFile(FileName, "rb");
			if(file2){
				Uint8 header2[(344 * 256) + 4];
				SDL_RWread(file2, header2, (344 * header[2]) + 4, 1);
				for(unsigned int j = 0; j < header[2]; j++){
					Uint16 width = SDL_SwapLE16(((Uint16 *)header2)[(j * 172)]);
					Uint16 height = SDL_SwapLE16(((Uint16 *)header2)[(j * 172) + 1]);
					Sint16 offsetx = SDL_SwapLE16(((Sint16 *)header2)[(j * 172) + 2]);
					Sint16 offsety = SDL_SwapLE16(((Sint16 *)header2)[(j * 172) + 3]);
					spriteoffsetx[i][j] = offsetx;
					spriteoffsety[i][j] = offsety;
					spritewidth[i][j] = width;
					spriteheight[i][j] = height;
					if(dedicatedserver){
						spritebank[i][j] = (SDL_Surface *)true;
						continue;
					}
					Uint32 size = SDL_SwapLE32(((Uint32 *)header2)[(j * 86) + 3]);
					Uint8 offsets = ((Uint8 *)header2)[(j * 344) + 20];
					Uint8 * data = new Uint8[size];
					Uint8 * decompressed = new Uint8[width * height];
					if(offsets){
						Uint32 tempvalue = 0;
						Uint32 count = 0;
						for(unsigned int y2 = 0; y2 < (height + 63) / 64; y2++){
							for(unsigned int x2 = 0; x2 < (width + 63) / 64; x2++){
								unsigned int ymax = (y2 * 64) + 64;
								if(ymax > height){
									ymax = height;
								}
								unsigned int xmax = (x2 * 64) + 64;
								if(xmax > width){
									xmax = width;
								}
								for(unsigned int y = y2 * 64; y < ymax; y++){
									for(unsigned int x = x2 * 64; x < xmax; x += 4){
										if(count){
											((Uint32 *)decompressed)[((y * width) / 4) + (x / 4)] = SDL_SwapLE32(tempvalue);
											count -= 4;
										}else{
											SDL_RWread(file2, &tempvalue, sizeof(Uint32), 1);
											if(tempvalue >= 0xFF000000){
												count = tempvalue & 0x0000FFFF;
												tempvalue &= 0x00FF0000;
												tempvalue |= tempvalue << 8;
												tempvalue |= tempvalue >> 16;
												count -= 4;
											}
											((Uint32 *)decompressed)[((y * width) / 4) + (x / 4)] = SDL_SwapLE32(tempvalue);
										}
									}
								}
							}
						}
					}else{
						SDL_RWread(file2, data, size, 1);
						for(unsigned int j = 0, k = 0; j < size / 4; j++, k++){
							Uint32 tempvalue = SDL_SwapLE32(((Uint32 *)data)[j]);
							if(tempvalue >= 0xFF000000){
								Uint32 count = tempvalue & 0x0000FFFF;
								tempvalue &= 0x00FF0000;
								tempvalue |= tempvalue << 8;
								tempvalue |= tempvalue >> 16;
								while(count){
									((Uint32 *)decompressed)[k] = SDL_SwapLE32(tempvalue);
									count -= 4;
									k++;
								}
								k--;
							}else{
								((Uint32 *)decompressed)[k] = SDL_SwapLE32(tempvalue);
							}
						}
					}
					Uint8 * sprite = new Uint8[width * height];
					memcpy(sprite, decompressed, width * height);
					SDL_Surface * surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 8, 0, 0, 0, 0);
					//SDL_SetColorKey(surface, SDL_TRUE, 0);
					unsigned int paletteoffset = 0;
					switch(i){
						case 0:
							paletteoffset = 5;
						break;
						case 1:
							paletteoffset = 6;
						break;
						case 2:
							paletteoffset = 7;
						break;
						case 3:
							paletteoffset = 8;
						break;
						case 6:
							paletteoffset = 1;
						break;
						case 7:
							paletteoffset = 2;
						break;
					}
					//SDL_SetColors(surface, palette.colors[paletteoffset], 0, 256);
					//SDL_SetPalette(surface,  SDL_LOGPAL, palette.colors[paletteoffset], 0, 256);
					spritebank[i][j] = surface;
					SDL_LockSurface(surface);
					memcpy(surface->pixels, sprite, width * height);
					SDL_UnlockSurface(surface);
					//RLESurface(surface);
					delete[] sprite;
					delete[] data;
					delete[] decompressed;
				}
				SDL_RWclose(file2);
			}else{
				printf("Could not open %s\n", FileName);
				SDL_RWclose(file);
				return false;
			}
		}
	}
	SDL_RWclose(file);
	return true;
}

bool Resources::LoadTiles(bool dedicatedserver){
	if(dedicatedserver){
		return true;
	}
	char filename[256];
	SDL_RWops * file = SDL_RWFromFile("BIN_TIL.DAT", "rb");
	if(!file){
		printf("Could not open BIN_TIL.DAT\n");
		return false;
	}
	for(unsigned int i = 0; i < 256; i++){
		Uint8 header[64];
		SDL_RWread(file, header, sizeof(header), 1);
		if(header[2]){
			sprintf(filename, "bin_til/TIL_%.3d.BIN", i);
			SDL_RWops * file2 = SDL_RWFromFile(filename, "rb");
			if(file2){
				Uint8 header2[(12 * 256) + 4];
				SDL_RWread(file2, header2, (12 * header[2]) + 4, 1);
				Uint8 * data = new Uint8[64 * 64 * 256];
				size_t datasize = SDL_RWread(file2, data, 1, 64 * 64 * 256);
				Uint8 * decompressed = new Uint8[64 * 64 * 256];
				for(unsigned int j = 0, k = 0; j < datasize / 4; j++, k++){
					Uint32 tempvalue = SDL_SwapLE32(((Uint32 *)data)[j]);
					if(tempvalue >= 0xFF000000){
						Uint32 count = tempvalue & 0x0000FFFF;
						tempvalue &= 0x00FF0000;
						tempvalue |= tempvalue << 8;
						tempvalue |= tempvalue >> 16;
						while(count){
							((Uint32 *)decompressed)[k] = SDL_SwapLE32(tempvalue);
							count -= 4;
							k++;
						}
						k--;
					}else{
						((Uint32 *)decompressed)[k] = SDL_SwapLE32(tempvalue);
					}
				}
				for(unsigned int j = 0; j < header[2]; j++){
					Uint8 tile[64 * 64];
					memcpy(tile, decompressed + (j * 64 * 64), sizeof(tile));
					SDL_Surface * surface = SDL_CreateRGBSurface(SDL_SWSURFACE, 64, 64, 8, 0, 0, 0, 0);
					//SDL_SetColorKey(surface, SDL_TRUE, 0);
					//SDL_SetColors(surface, palette.colors[0], 0, 256);
					tilebank[i][j] = surface;
					SDL_LockSurface(surface);
					memcpy(surface->pixels, tile, 64 * 64);
					SDL_UnlockSurface(surface);
					tileflippedbank[i][j] = SDL_CreateRGBSurface(SDL_SWSURFACE, 64, 64, 8, 0, 0, 0, 0);
				    //SDL_SetColorKey(tileflippedbank[i][j], SDL_TRUE, 0);
					//SDL_SetColors(tileflippedbank[i][j], palette.colors[0], 0, 256);
					SDL_LockSurface(tileflippedbank[i][j]);
					memcpy(tileflippedbank[i][j]->pixels, surface->pixels, surface->w * surface->h);
					SDL_UnlockSurface(tileflippedbank[i][j]);
					MirrorY(tileflippedbank[i][j]);
				}
				delete[] data;
				delete[] decompressed;
				SDL_RWclose(file2);
			}else{
				printf("Could not open %s %d\n", filename, errno);
				return false;
			}
		}
	}
	SDL_RWclose(file);
	return true;
}

bool Resources::LoadSounds(bool dedicatedserver){
	if(dedicatedserver){
		return true;
	}
	SDL_RWops * file = SDL_RWFromFile("sound.bin", "rb");
	if(!file){
		printf("Could not open sound.bin\n");
		return false;
	}
	Uint32 numsounds;
	Uint32 soundssize;
	Uint8 soundheader[0x60];
	char name[0x10 + 1];
	memset(name, 0, sizeof(name));
	Uint32 offset;
	Uint32 length;
	Uint32 wavinfo;
	SDL_RWread(file, &numsounds, sizeof(numsounds), 1);
	SDL_RWread(file, &soundssize, sizeof(soundssize), 1);
	for(unsigned int i = 0; i < numsounds; i++){
		SDL_RWseek(file, sizeof(numsounds) + sizeof(soundssize) + (i * sizeof(soundheader)), RW_SEEK_SET);
		SDL_RWread(file, &soundheader, sizeof(soundheader), 1);
		memcpy(&name, &soundheader[4], 0x10);
		memcpy(&offset, &soundheader[4 + 0x10], sizeof(offset));
		memcpy(&length, &soundheader[4 + 0x10 + 4], sizeof(length));
		memcpy(&wavinfo, &soundheader[4 + 0x10 + 4 + 4], sizeof(wavinfo));
		
		if(!length){
			continue;
		}
	
		Uint8 header[] = {
			0x52, 0x49, 0x46, 0x46, // RIFF
			0x00, 0x00, 0x00, 0x00, // file size - 8
			0x57, 0x41, 0x56, 0x45, // WAVE
			0x66, 0x6D, 0x74, 0x20, // fmt chunk
			0x14, 0x00, 0x00, 0x00, // chunk data size
			0x11, 0x00, // WAVE_FORMAT_DVI_ADPCM
			0x01, 0x00, // 1 channel
			0x11, 0x2B, 0x00, 0x00, // 11025 hz			
			0xD4, 0x15, 0x00, 0x00, // 5588 average bytes per second
			0x00, 0x01, // 256 block align
			0x04, 0x00, // 4 bits per sample
			0x02, 0x00, // 2 samples per block
			0xF9, 0x01, // extra format bytes
			0x66, 0x61, 0x63, 0x74, // fact chunk
			0x04, 0x00, 0x00, 0x00, // 4 bytes long
			0x3F, 0xB5, 0x00, 0x00,
			0x64, 0x61, 0x74, 0x61, // data chunk
			0x00, 0x00, 0x00, 0x00
		};
		
		length += 24;
		
		//printf("name: %s, wavinfo: %d length: %X\n", name, wavinfo, length);
		
		Uint32 lengthplus = length + 0x34;
		Uint32 lengthminus = length;
		
		memcpy(&header[4], &lengthplus, sizeof(lengthplus));
		memcpy(&header[sizeof(header) - 4], &lengthminus, sizeof(lengthminus));
		Uint8 * mem = new Uint8[sizeof(header) + length];
		memcpy(mem, header, sizeof(header));
		
		SDL_RWseek(file, sizeof(numsounds) + sizeof(soundssize) + (numsounds * sizeof(soundheader)) + offset, RW_SEEK_SET);
		SDL_RWread(file, &mem[sizeof(header)], length, 1);
		memset(&mem[sizeof(header) + length - 24], 0, 24);
		
		/*FILE * file2 = fopen(name, "w");
		fwrite(&mem[sizeof(header)], 1, length, file2);
		fclose(file2);*/
		
		if(!SDL_RWFromMem(mem, length)){
			printf("SDL_RWFromMem failed\n");
		}
		Mix_Chunk * chunk = Mix_LoadWAV_RW(SDL_RWFromMem(mem, sizeof(header) + length), true);
		delete[] mem;
		if(!chunk){
			printf("Could not load sound %s - %s\n", name, Mix_GetError());
			SDL_RWclose(file);
			return false;
		}else{
			soundbank[name] = chunk;
		}
	}
	SDL_RWclose(file);
	music = Mix_LoadMUS("CLOSER2.MP3");
	if(!music){
		return false;
	}
	return true;
}

void Resources::UnloadSounds(void){
	for(std::map<std::string, Mix_Chunk *>::iterator it = soundbank.begin(); it != soundbank.end(); it++){
		Mix_FreeChunk((*it).second);
	}
	soundbank.clear();
	if(music){
		Mix_FreeMusic(music);
		music = 0;
	}
}

void Resources::MirrorY(SDL_Surface * surface){
    SDL_Surface * newsurface = SDL_CreateRGBSurface(SDL_SWSURFACE, surface->w, surface->h, surface->format->BitsPerPixel, 0, 0, 0, 0);
    //SDL_SetColors(newsurface, palette.colors[0], 0, 256);
    SDL_BlitSurface(surface, 0, newsurface, 0);
    SDL_LockSurface(surface);
    SDL_LockSurface(newsurface);
    for(int y = 0; y < surface->h; y++){
        for(int x = 0; x < surface->w; x++){
            ((Uint8 *)surface->pixels)[x + (y * surface->w)] = ((Uint8 *)newsurface->pixels)[(y * surface->w) + (surface->w - (x + 1))];
        }
    }
    SDL_UnlockSurface(newsurface);
    SDL_UnlockSurface(surface);
    SDL_FreeSurface(newsurface);
}

void Resources::RLESurface(SDL_Surface * surface){
	Uint8 *rlebuf, *dst;
    int maxn;
    int y;
    Uint8 *srcbuf, *curbuf, *lastline;
    int maxsize = 0;
    int skip, run;
    int bpp = surface->format->BytesPerPixel;
    Uint32 ckey, rgbmask;
    int w, h;
	
    /* calculate the worst case size for the compressed surface */

	/* worst case is alternating opaque and transparent pixels,
	 starting with an opaque pixel */
	maxsize = surface->h * 3 * (surface->w / 2 + 1) + 2;
	
    rlebuf = (Uint8 *) SDL_malloc(maxsize);
    if (rlebuf == NULL) {
        SDL_OutOfMemory();
        return;
    }
	
    /* Set up the conversion */
    srcbuf = (Uint8 *) surface->pixels;
    curbuf = srcbuf;
    maxn = 255;
    skip = run = 0;
    dst = rlebuf;
    rgbmask = ~surface->format->Amask;
    ckey = 0;
    lastline = dst;
    w = surface->w;
    h = surface->h;
	
#define ADD_COUNTS(n, m)			\
if(bpp == 4) {				\
((Uint16 *)dst)[0] = n;		\
((Uint16 *)dst)[1] = m;		\
dst += 4;				\
} else {				\
dst[0] = n;				\
dst[1] = m;				\
dst += 2;				\
}
	
    for (y = 0; y < h; y++) {
        int x = 0;
        int blankline = 0;
        do {
            int run, skip, len;
            int runstart;
            int skipstart = x;
			
            /* find run of transparent, then opaque pixels */
            while (x < w && (*(srcbuf + x * bpp) & rgbmask) == ckey)
                x++;
            runstart = x;
            while (x < w && (*(srcbuf + x * bpp) & rgbmask) != ckey)
                x++;
            skip = runstart - skipstart;
            if (skip == w)
                blankline = 1;
            run = x - runstart;
			
            /* encode segment */
            while (skip > maxn) {
                ADD_COUNTS(maxn, 0);
                skip -= maxn;
            }
            len = (run < maxn ? run : maxn);
            ADD_COUNTS(skip, len);
            SDL_memcpy(dst, srcbuf + runstart * bpp, len * bpp);
            dst += len * bpp;
            run -= len;
            runstart += len;
            while (run) {
                len = (run < maxn ? run : maxn);
                ADD_COUNTS(0, len);
                SDL_memcpy(dst, srcbuf + runstart * bpp, len * bpp);
                dst += len * bpp;
                runstart += len;
                run -= len;
            }
            if (!blankline)
                lastline = dst;
        } while (x < w);
		
        srcbuf += surface->pitch;
    }
    dst = lastline;             /* back up bast trailing blank lines */
    ADD_COUNTS(0, 0);
	
#undef ADD_COUNTS
	
    /* Now that we have it encoded, release the original pixels */
    if (!(surface->flags & SDL_PREALLOC)) {
        SDL_free(surface->pixels);
        surface->pixels = NULL;
    }
	
    /* realloc the buffer to release unused memory */
    {
        /* If realloc returns NULL, the original block is left intact */
        Uint8 *p = (Uint8 *)SDL_realloc(rlebuf, dst - rlebuf);
        if (!p)
            p = rlebuf;
		surface->map->sw_data->aux_data = p;
    }
}