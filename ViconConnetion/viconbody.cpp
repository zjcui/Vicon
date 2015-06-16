#include "viconbody.h"

extern int FindBotID( QString name );

ViconBody::ViconBody()
{
	name = "";
	id = -1;
}

ViconBody::ViconBody(QString bodyName){
	name = bodyName;
	prevPos[0] = 0;
	prevPos[1] = 0;
	prevPos[2] = 0;
	prevEori[0] = 0;
	prevEori[1] = 0;
	prevEori[2] = 0;

	// Get our botid from the database
	id = FindBotID(name);
}
