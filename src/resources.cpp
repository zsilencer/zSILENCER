#include "resources.h"
#include <vector>
#include "game.h"
#include "os.h"

Resources::Resources(){
	spritebank.assign(256, std::vector<std::shared_ptr<Surface>>(256, std::make_shared<Surface>(0, 0)));
	tilebank.assign(256, std::vector<std::shared_ptr<Surface>>(256, std::make_shared<Surface>(0, 0)));
	tileflippedbank.assign(256, std::vector<std::shared_ptr<Surface>>(256, std::make_shared<Surface>(0, 0)));
	spriteoffsetx.assign(256, std::vector<int>(256, 0));
	spriteoffsety.assign(256, std::vector<int>(256, 0));
	spritewidth.assign(256, std::vector<unsigned int>(256, 0));
	spriteheight.assign(256, std::vector<unsigned int>(256, 0));
	menumusic = 0;
	gamemusic = 0;
}

bool Resources::Load(Game & game, bool dedicatedserver){
	progress = 0;
	totalprogressitems = 621;
	CDResDir();
	if(!LoadSprites(game, dedicatedserver)){
		return false;
	}
	if(!LoadTiles(game, dedicatedserver)){
		return false;
	}
	if(!LoadSounds(game, dedicatedserver)){
		//return false;
	}
	return true;
}

bool Resources::LoadSprites(Game & game, bool dedicatedserver){
	char FileName[256];
	SDL_RWops * file = SDL_RWFromFile((GetResDir() + "BIN_SPR.DAT").c_str(), "rb");
	if(!file){
		printf("Could not open BIN_SPR.DAT\n");
		return false;
	}
	Uint8 headers[256][64];
	SDL_RWread(file, headers, 64, 256);
	for(unsigned int i = 0; i < 256; i++){
		progress++;
		game.LoadProgressCallback(progress, totalprogressitems);
		if(headers[i][2]){
			sprintf(FileName, "bin_spr/SPR_%.3d.BIN", i);
			SDL_RWops * file2 = SDL_RWFromFile((GetResDir() + FileName).c_str(), "rb");
			if(file2){
				Uint8 header2[(344 * 256) + 4];
				SDL_RWread(file2, header2, (344 * headers[i][2]) + 4, 1);
				for(unsigned int j = 0; j < headers[i][2]; j++){
					Uint16 width = SDL_SwapLE16(((Uint16 *)header2)[(j * 172)]);
					Uint16 height = SDL_SwapLE16(((Uint16 *)header2)[(j * 172) + 1]);
					Sint16 offsetx = SDL_SwapLE16(((Sint16 *)header2)[(j * 172) + 2]);
					Sint16 offsety = SDL_SwapLE16(((Sint16 *)header2)[(j * 172) + 3]);
					spriteoffsetx[i][j] = offsetx;
					spriteoffsety[i][j] = offsety;
					spritewidth[i][j] = width;
					spriteheight[i][j] = height;
					if(dedicatedserver){
						spritebank[i][j] = std::make_shared<Surface>(0, 0);;
						continue;
					}
					Uint32 size = SDL_SwapLE32(((Uint32 *)header2)[(j * 86) + 3]);
					Uint8 offsets = ((Uint8 *)header2)[(j * 344) + 20];
					std::vector<Uint8> data(size);
					std::vector<Uint8> decompressed(width * height);
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
											((Uint32 *)decompressed.data())[((y * width) / 4) + (x / 4)] = SDL_SwapLE32(tempvalue);
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
											((Uint32 *)decompressed.data())[((y * width) / 4) + (x / 4)] = SDL_SwapLE32(tempvalue);
										}
									}
								}
							}
						}
					}else{
						SDL_RWread(file2, data.data(), size, 1);
						for(unsigned int j = 0, k = 0; j < size / 4; j++, k++){
							Uint32 tempvalue = SDL_SwapLE32(((Uint32 *)data.data())[j]);
							if(tempvalue >= 0xFF000000){
								Uint32 count = tempvalue & 0x0000FFFF;
								tempvalue &= 0x00FF0000;
								tempvalue |= tempvalue << 8;
								tempvalue |= tempvalue >> 16;
								while(count){
									((Uint32 *)decompressed.data())[k] = SDL_SwapLE32(tempvalue);
									count -= 4;
									k++;
								}
								k--;
							}else{
								((Uint32 *)decompressed.data())[k] = SDL_SwapLE32(tempvalue);
							}
						}
					}
					std::vector<Uint8> sprite(width * height);
					memcpy(sprite.data(), decompressed.data(), width * height);
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
					spritebank[i][j] = std::make_shared<Surface>(width, height);
					Surface * surface = spritebank[i][j].get();
					memcpy(surface->pixels.data() , sprite.data(), width * height);
					RLESurface(surface);
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

bool Resources::LoadTiles(Game & game, bool dedicatedserver){
	if(dedicatedserver){
		progress += 255;
		return true;
	}
	char filename[256];
	SDL_RWops * file = SDL_RWFromFile((GetResDir() + "BIN_TIL.DAT").c_str(), "rb");
	if(!file){
		printf("Could not open BIN_TIL.DAT\n");
		return false;
	}
	Uint8 headers[256][64];
	SDL_RWread(file, headers, 64, 256);
	for(unsigned int i = 0; i < 256; i++){
		progress++;
		game.LoadProgressCallback(progress, totalprogressitems);
		if(headers[i][2]){
			sprintf(filename, "bin_til/TIL_%.3d.BIN", i);
			SDL_RWops * file2 = SDL_RWFromFile((GetResDir() + filename).c_str(), "rb");
			if(file2){
				Uint8 header2[(12 * 256) + 4];
				SDL_RWread(file2, header2, (12 * headers[i][2]) + 4, 1);
				std::vector<Uint8> data(64 * 64 * 256);
				size_t datasize = SDL_RWread(file2, data.data(), 1, data.size());
				std::vector<Uint8> decompressed(64 * 64 * 256);
				for(unsigned int j = 0, k = 0; j < datasize / 4; j++, k++){
					Uint32 tempvalue = SDL_SwapLE32(((Uint32 *)data.data())[j]);
					if(tempvalue >= 0xFF000000){
						Uint32 count = tempvalue & 0x0000FFFF;
						tempvalue &= 0x00FF0000;
						tempvalue |= tempvalue << 8;
						tempvalue |= tempvalue >> 16;
						while(count){
							((Uint32 *)decompressed.data())[k] = SDL_SwapLE32(tempvalue);
							count -= 4;
							k++;
						}
						k--;
					}else{
						((Uint32 *)decompressed.data())[k] = SDL_SwapLE32(tempvalue);
					}
				}
				for(unsigned int j = 0; j < headers[i][2]; j++){
					Uint8 tile[64 * 64];
					memcpy(tile, decompressed.data() + (j * 64 * 64), sizeof(tile));
					tilebank[i][j] = std::make_shared<Surface>(64, 64);
					Surface * surface = tilebank[i][j].get();
					memcpy(surface->pixels.data(), tile, 64 * 64);
					RLESurface(tilebank[i][j].get());
					tileflippedbank[i][j] = std::make_shared<Surface>(64, 64);
					memcpy(tileflippedbank[i][j]->pixels.data(), surface->pixels.data(), surface->w * surface->h);
					MirrorY(tileflippedbank[i][j].get());
					RLESurface(tileflippedbank[i][j].get());
				}
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

bool Resources::LoadSounds(Game & game, bool dedicatedserver){
	if(dedicatedserver){
		progress += 101;
		return true;
	}
	SDL_RWops * file = SDL_RWFromFile((GetResDir() + "sound.bin").c_str(), "rb");
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
		progress++;
		game.LoadProgressCallback(progress, totalprogressitems);
		SDL_RWseek(file, sizeof(numsounds) + sizeof(soundssize) + (i * sizeof(soundheader)), RW_SEEK_SET);
		SDL_RWread(file, &soundheader, sizeof(soundheader), 1);
		memcpy(&name, &soundheader[4], 0x10);
		memcpy(&offset, &soundheader[4 + 0x10], sizeof(offset));
		memcpy(&length, &soundheader[4 + 0x10 + 4], sizeof(length));
		memcpy(&wavinfo, &soundheader[4 + 0x10 + 4 + 4], sizeof(wavinfo));
		
		if(!length){
			continue;
		}
		
		if(length < 256){
			// really small sound data such as grnup.wav cause issues with Mix_LoadWAV_RW
			// will need to revisit this sound loading below, for now just going to skip it
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
		Uint32 lengthminus = length - sizeof(header);
		
		int memsize = sizeof(header) + length;
		memcpy(&header[4], &lengthplus, sizeof(lengthplus));
		memcpy(&header[sizeof(header) - 4], &lengthminus, sizeof(lengthminus));
		std::vector<Uint8> mem(memsize);
		memset(mem.data(), 0, memsize);
		memcpy(mem.data(), header, sizeof(header));
		
		SDL_RWseek(file, sizeof(numsounds) + sizeof(soundssize) + (numsounds * sizeof(soundheader)) + offset, RW_SEEK_SET);
		SDL_RWread(file, &mem[sizeof(header)], length, 1);
		memset(&mem[sizeof(header) + length - 24], 0, 24);
		
		/*FILE * file2 = fopen(name, "w");
		fwrite(&mem[sizeof(header)], 1, length, file2);
		fclose(file2);*/
		
		Mix_Chunk * chunk = Mix_LoadWAV_RW(SDL_RWFromConstMem(mem.data(), mem.size()), true);
		if(!chunk){
			printf("Could not load sound %s - %s\n", name, Mix_GetError());
			SDL_RWclose(file);
			return false;
		}else{
			soundbank[name] = chunk;
			// fix pop at end of sounds
			int v = 0;
			int length = (chunk->alen / sizeof(Sint16));
			for(int i = length - 1; i > length - 100; i--, v++){
				if(i >= 0){
					((Sint16 *)chunk->abuf)[i] *= v / float(100);
				}
			}
			v = 0;
			for(int i = 0; i < 100; i++, v++){
				if(i < length - 1){
					((Sint16 *)chunk->abuf)[i] *= v / float(100);
				}
			}
			//
		}
	}
	SDL_RWclose(file);
	menumusic = Mix_LoadMUS("CLOSER2.MP3");
	return true;
}

void Resources::UnloadSounds(void){
	for(std::map<std::string, Mix_Chunk *>::iterator it = soundbank.begin(); it != soundbank.end(); it++){
		Mix_FreeChunk((*it).second);
	}
	soundbank.clear();
	Mix_FadeOutMusic(0);
	Mix_ResumeMusic();
	if(menumusic){
		Mix_FreeMusic(menumusic);
		menumusic = 0;
	}
	if(gamemusic){
		Mix_FreeMusic(gamemusic);
		gamemusic = 0;
	}
}

void Resources::MirrorY(Surface * surface){
    Surface newsurface(surface->w, surface->h);
	memcpy(newsurface.pixels.data(), surface->pixels.data(), surface->w * surface->h);
    for(int y = 0; y < surface->h; y++){
        for(int x = 0; x < surface->w; x++){
            ((Uint8 *)surface->pixels.data())[x + (y * surface->w)] = ((Uint8 *)newsurface.pixels.data())[(y * surface->w) + (surface->w - (x + 1))];
        }
    }
}

void Resources::RLESurface(Surface * surface){
	Uint8 *dst;
	std::vector<Uint8> rlebuf;
	int maxn;
	int y;
	Uint8 *srcbuf, *curbuf, *lastline;
	int maxsize = 0;
	int skip, run;
	int bpp = 1;
	Uint32 ckey, rgbmask;
	int w, h;

	/* calculate the worst case size for the compressed surface */

	/* worst case is alternating opaque and transparent pixels,
	 starting with an opaque pixel */
	maxsize = surface->h * 3 * (surface->w / 2 + 1) + 2;

	rlebuf.resize(maxsize);

	/* Set up the conversion */
	srcbuf = (Uint8 *) surface->pixels.data();
	curbuf = srcbuf;
	maxn = 255;
	skip = run = 0;
	dst = rlebuf.data();
	rgbmask = 0xFF;//~surface->format->Amask;
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
		
		srcbuf += surface->w;//surface->pitch;
	}
	dst = lastline;             /* back up bast trailing blank lines */
	ADD_COUNTS(0, 0);

	#undef ADD_COUNTS

	/* realloc the buffer to release unused memory */
	int newsize = dst - rlebuf.data();
	surface->rlepixels.assign(rlebuf.data(), rlebuf.data() + newsize);
	//delete[] surface->pixels;
	//surface->pixels = 0;
}