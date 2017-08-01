#include "objecttypes.h"
#include "overlay.h"
#include "interface.h"
#include "button.h"
#include "toggle.h"
#include "selectbox.h"
#include "scrollbar.h"
#include "textbox.h"
#include "textinput.h"
#include "state.h"
#include "team.h"
#include "player.h"
#include "civilian.h"
#include "guard.h"
#include "robot.h"
#include "terminal.h"
#include "vent.h"
#include "healmachine.h"
#include "creditmachine.h"
#include "secretreturn.h"
#include "surveillancemonitor.h"
#include "techstation.h"
#include "inventorystation.h"
#include "teambillboard.h"
#include "plume.h"
#include "shrapnel.h"
#include "bodypart.h"
#include "blasterprojectile.h"
#include "laserprojectile.h"
#include "rocketprojectile.h"
#include "flamerprojectile.h"
#include "plasmaprojectile.h"
#include "flareprojectile.h"
#include "wallprojectile.h"
#include "basedoor.h"
#include "pickup.h"
#include "warper.h"
#include "detonator.h"
#include "fixedcannon.h"
#include "grenade.h"
#include "walldefense.h"
#include "baseexit.h"

ObjectTypes::ObjectTypes(){
	memset(serializedsize, 0, sizeof(serializedsize));
}

Object * ObjectTypes::CreateFromType(Uint8 type){
	switch(type){
		case OVERLAY:
			return (Object *)new Overlay();
		break;
		case INTERFACE:
			return (Object *)new Interface();
		break;
		case BUTTON:
			return (Object *)new Button();
		break;
		case TOGGLE:
			return (Object *)new Toggle();
		break;
		case SELECTBOX:
			return (Object *)new SelectBox();
		break;
		case SCROLLBAR:
			return (Object *)new ScrollBar();
		break;
		case TEXTBOX:
			return (Object *)new TextBox();
		break;
		case TEXTINPUT:
			return (Object *)new TextInput();
		break;
		case STATE:
			return (Object *)new State();
		break;
		case TEAM:
			return (Object *)new Team();
		break;
		case PLAYER:
			return (Object *)new Player();
		break;
		case CIVILIAN:
			return (Object *)new Civilian();
		break;
		case GUARD:
			return (Object *)new Guard();
		break;
		case ROBOT:
			return (Object *)new Robot();
		break;
		case TERMINAL:
			return (Object *)new Terminal();
		break;
		case VENT:
			return (Object *)new Vent();
		break;
		case HEALMACHINE:
			return (Object *)new HealMachine();
		break;
		case CREDITMACHINE:
			return (Object *)new CreditMachine();
		break;
		case SECRETRETURN:
			return (Object *)new SecretReturn();
		break;
		case SURVEILLANCEMONITOR:
			return (Object *)new SurveillanceMonitor();
		break;
		case TECHSTATION:
			return (Object *)new TechStation();
		break;
		case INVENTORYSTATION:
			return (Object *)new InventoryStation();
		break;
		case TEAMBILLBOARD:
			return (Object *)new TeamBillboard();
		break;
		case PLUME:
			return (Object *)new Plume();
		break;
		case SHRAPNEL:
			return (Object *)new Shrapnel();
		break;
		case BODYPART:
			return (Object *)new BodyPart();
		break;
		case BLASTERPROJECTILE:
			return (Object *)new BlasterProjectile();
		break;
		case LASERPROJECTILE:
			return (Object *)new LaserProjectile();
		break;
		case ROCKETPROJECTILE:
			return (Object *)new RocketProjectile();
		break;
		case FLAMERPROJECTILE:
			return (Object *)new FlamerProjectile();
		break;
		case PLASMAPROJECTILE:
			return (Object *)new PlasmaProjectile();
		break;
		case FLAREPROJECTILE:
			return (Object *)new FlareProjectile();
		break;
		case WALLPROJECTILE:
			return (Object *)new WallProjectile();
		break;
		case BASEDOOR:
			return (Object *)new BaseDoor();
		break;
		case PICKUP:
			return (Object *)new PickUp();
		break;
		case WARPER:
			return (Object *)new Warper();
		break;
		case DETONATOR:
			return (Object *)new Detonator();
		break;
		case FIXEDCANNON:
			return (Object *)new FixedCannon();
		break;
		case GRENADE:
			return (Object *)new Grenade();
		break;
		case WALLDEFENSE:
			return (Object *)new WallDefense();
		break;
		case BASEEXIT:
			return (Object *)new BaseExit();
		break;
	}
	return 0;
}

unsigned int ObjectTypes::SerializedSize(Uint8 type){
	if(!serializedsize[type]){
		Serializer data;
		Object * object = CreateFromType(type);
		if(object){
			object->Serialize(Serializer::WRITE, data);
			delete object;
			serializedsize[type] = data.offset;
		}
	}
	return serializedsize[type];
}