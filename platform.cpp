#include "platform.h"
#include <math.h>

Platform::Platform(Uint8 type, Uint16 id, int x1, int y1, int x2, int y2){
	Platform::type = type;
	Platform::id = id;
	Platform::x1 = x1;
	Platform::y1 = y1;
	Platform::x2 = x2;
	Platform::y2 = y2;
	adjacentl = 0;
	adjacentr = 0;
	set = 0;
}

void Platform::GetTopSegment(int & x1, int & y1, int & x2, int & y2){
	if(type == RECTANGLE || type == LADDER){
		x1 = Platform::x1;
		y1 = Platform::y1;
		x2 = Platform::x2;
		y2 = Platform::y1;
    }else
    if(type == STAIRSUP){
		x1 = Platform::x1;
		y1 = Platform::y2;
		x2 = Platform::x2;
		y2 = Platform::y1;
	}else
    if(type == STAIRSDOWN){
		x1 = Platform::x1;
		y1 = Platform::y1;
		x2 = Platform::x2;
		y2 = Platform::y2;         
    }
}

int Platform::XtoY(int x){
	if(type == STAIRSUP || type == STAIRSDOWN){
		int tx1, ty1, tx2, ty2;
		GetTopSegment(tx1, ty1, tx2, ty2);
		int length = abs(signed(tx2) - signed(tx1));
		int pos = abs(signed(x) - signed(tx1));
		float time = float(pos) / length;
		if(type == STAIRSUP){
			return (-abs(signed(ty1) - signed(ty2)) * time) + ty1;
		}else{
			return (abs(signed(ty1) - signed(ty2)) * time) + ty1;
		}
	}else{
		return y1;
	}
}

void Platform::GetNormal(int x, int y, float * xn, float * yn){
	switch(type){
		case RECTANGLE:{
			if(x <= x1){
				*xn = -1;
			}else
			if(x >= x2){
				*xn = 1;
			}else{
				*xn = 0;
			}
			if(y <= y1){
				*yn = -1;
			}else
			if(y >= y2){
				*yn = 1;
			}else{
				*yn = 0;
			}
		}break;
		case STAIRSUP:{
			//float slope = (y1 - y2) / (x2 - x1);
			if(x >= x2){
				*xn = 1;
			}else{
				*xn = (signed(y1) - signed(y2));
			}
			if(y >= y2){
				*yn = 1;
			}else{
				*yn = -(signed(x2) - signed(x1));
			}
		}break;
		case STAIRSDOWN:{
			//float slope = (y2 - y1) / (x2 - x1);
			if(x <= x1){
				*xn = -1;
			}else{
				*xn = (signed(y2) - signed(y1));
			}
			if(y >= y2){
				*yn = 1;
			}else{
				*yn = -(signed(x2) - signed(x1));
			}
		}break;
	}
	float length = sqrt((*xn * *xn) + (*yn * *yn));
	*xn = *xn / length;
	*yn = *yn / length;
}

int Platform::GetLength(void){
	return abs(x2 - x1);
}