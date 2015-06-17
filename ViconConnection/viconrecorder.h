#ifndef VICONRECORDER_H
#define VICONRECORDER_H

#include "sqlhandler.h"
#include "viconbody.h"

#include <j_Mocap.h>
#include <j_math.h>
#include <j_utility.h>

#include <QtCore>
#include <QtSql>
#include <QLinkedList>
#include <QTextStream>

#include <time.h>
#include <iostream>
#include <conio.h>

class ViconRecorder
{
public:
	ViconRecorder();
	~ViconRecorder();

	bool Initialize();

	void ProcessPositions(void);

private:
    void UpdatePosition( ViconBody *body, bool bForce = true );
	void AddBodies( int num );

	JMocap		*m_pMocap;
	SqlHandler	m_SqlHandler;
	QLinkedList<ViconBody *>	m_pBodies;
};

#endif
