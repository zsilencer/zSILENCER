#include "selectbox.h"

SelectBox::SelectBox() : Object(ObjectTypes::SELECTBOX){
	res_bank = 0xFF;
	lineheight = 13;
	scrolled = 0;
	maxlines = 256;
	selecteditem = -1;
	requiresmaptobeloaded = false;
	uid = 0;
	enterpressed = false;
}

SelectBox::~SelectBox(){
	while(DeleteItem(0));
}

bool SelectBox::AddItem(const char * text, Uint32 id){
	char * newtext = new char[strlen(text) + 1];
	strcpy(newtext, text);
	items.push_back(newtext);
	itemids.push_back(id);
	if(items.size() > height / lineheight){
		scrolled = items.size() - (height / lineheight);
	}else{
		scrolled = 0;
	}
	return true;
}

bool SelectBox::DeleteItem(unsigned int index){
	bool found = false;
	std::deque<char *>::iterator it = items.begin() + index;
	if(it != items.end()){
		char * item = (*it);
		delete[] item;
		items.erase(it);
		found = true;
	}
	std::deque<Uint32>::iterator it2 = itemids.begin() + index;
	if(it2 != itemids.end()){
		itemids.erase(it2);
	}
	return found;
}

Uint32 SelectBox::IndexToId(unsigned int index){
	if(index >= itemids.size()){
		return 0;
	}
	std::deque<Uint32>::iterator it2 = itemids.begin() + index;
	return (*it2);
}

int SelectBox::IdToIndex(Uint32 id){
	int index = 0;
	for(std::deque<Uint32>::iterator it = itemids.begin(); it != itemids.end(); it++, index++){
		if((*it) == id){
			return index;
		}
	}
	return -1;
}

char * SelectBox::GetItemName(unsigned int index){
	std::deque<char *>::iterator it = items.begin() + index;
	if(it != items.end()){
		char * item = (*it);
		return item;
	}
	return 0;
}

int SelectBox::MouseInside(World & world, Uint16 mousex, Uint16 mousey){
	Sint16 x1, y1, x2, y2;
	x1 = x - world.resources.spriteoffsetx[res_bank][res_index];
	x2 = x + (width) - world.resources.spriteoffsetx[res_bank][res_index] - 16;
	y1 = y - world.resources.spriteoffsety[res_bank][res_index];
	y2 = y + (height) - world.resources.spriteoffsety[res_bank][res_index];
	if(mousex < x2 && mousex > x1 && mousey < y2 && mousey > y1){
		int index = ((mousey - y1) / lineheight) + scrolled;
		if(index < items.size()){
			return index;
		}
	}
	return -1;
}

void SelectBox::ListFiles(const char * directory){
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
				if(info->d_type != DT_DIR && !S_ISDIR(st.st_mode)){
					AddItem(info->d_name);
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
				AddItem(info.cFileName);
			}
		}while(FindNextFile(dir, &info));
		FindClose(dir);
	}
#endif
}