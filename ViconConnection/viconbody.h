#ifndef VICONBODY_H
#define VICONBODY_H

#include <QtCore>

struct ViconBody{
	ViconBody();
	ViconBody(QString bodyName);

	QString name;

	int id;

	int prevPos[3];
	int prevEori[3];
};

#endif
