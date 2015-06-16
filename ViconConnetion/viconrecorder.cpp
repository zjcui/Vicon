
#include "dos.h"
#include "viconrecorder.h"

using namespace std;

#define BOT_INVALID		-1

// Takes the name given and queries the database for that bot's name
// If it is found, then the online state is determined. If the robot is
// reported to be online then the bot's unique id is returned, otherwise
// BOT_INVALID is returned
int FindBotID( QString name )
{
	if ( name.isEmpty() )
		return BOT_INVALID;

	// Find the bot id that corresponds to this object (if it exists)
	QSqlQuery query;
	query.prepare( "SELECT botid, online FROM activebots WHERE botname = :botname" );
	query.bindValue( ":botname", name );
	query.exec();

	if ( query.lastError().isValid() )
	{
		cout << query.lastError().text().toStdString();
		return BOT_INVALID;
	}

	if ( query.first() )
	{
		// If the body is online return our bot id
		if ( query.value(1).toBool() )
			return query.value(0).toInt();
	}
	
	// If we cannot find the bot or it is offline set this body's id invalid
	return BOT_INVALID;
}

// Given a bot's unique id this function tried to find the bot and ascertain
// it's online status. It returns that status as a boolean
bool isBotOnline( int botid )
{
	if ( botid <= 0 )
		return false;

	// Find the bot id that corresponds to this object (if it exists)
	QSqlQuery query;
	query.prepare( "SELECT online FROM activebots WHERE botid = :botid" );
	query.bindValue( ":botid", botid );
	query.exec();

	if ( query.first() )
		return query.value(0).toBool();

	return false;
}

ViconRecorder::ViconRecorder()
{
	// Initialize our m_pMocap variable
	m_pMocap = new JMocap();
}

ViconRecorder::~ViconRecorder( void )
{
	// Take care of our messes
	m_SqlHandler.closeDB();
	m_pMocap->cleanup();
}

// Returns true if we started everything properly
bool ViconRecorder::Initialize( void )
{
	// open database
	if( !m_SqlHandler.openDB("192.168.1.150", "skynet", "root", "Anouck Girard") )
	{
		cout << "Failed  to connect to database" << endl;
		cout << "Quiting in 3 seconds..." << endl;
		Sleep(3000);
		return false;
	}

	// Clear out the botdata table so we are only working with new data
	m_SqlHandler.queryCommand("truncate botdata");

	//Initialization of m_pMocap System
    string address = "192.168.1.130";
	if( !m_pMocap->initialize(address) )
	{
		cout << "Failed to connect to Mocap at " << address << endl;
		cout << "Quiting in 3 seconds..." << endl;
		Sleep(3000);
		return false;
	}

	// setup m_pBodies
	int numbodies = 0;
	cout << "Number of Bodies: ";
	cin >> numbodies;

	AddBodies( numbodies );

	// No bodies? Quit the app
	if ( numbodies == 0 )
		return false;

	return true;
}

void ViconRecorder::ProcessPositions(void)
{	
	cout << "Recording vicon data..." << endl;

	while(!_kbhit())
	{
		m_pMocap->update();
		QLinkedListIterator<ViconBody *> iter(m_pBodies);

		while(iter.hasNext() && !_kbhit())
		{
			ViconBody *body = iter.next();

			// If we don't have a botid we are either offline or were not online
			// at the start of the recorder so check again!
			if ( body->id == -1 )
			{
				body->id = FindBotID( body->name );
				continue;
			}
			else if ( !isBotOnline(body->id) )
			{
				// This bot is not online and we shouldn't send a position update
				continue;
			}

			UpdatePosition(body);
		}

		// Wait 10 ms before polling for the next position
		Sleep(10);
	}

	cout << "Stopped recording" << endl;
}

void ViconRecorder::AddBodies( int num )
{
	// Setup our bodyname information
	std::string bodyName;
	int i = 0;

	for(i=0; i < num; i++ )
	{
		do {
			cout << "Enter body name #" << QString::number(i).toStdString() << ": ";
			cin >> bodyName;
			
			if ( bodyName.find(":") == string::npos )
				// Turn us into the standard format for Vicon (name:name)
				bodyName += ":" + bodyName;

			if( !m_pMocap->isBodyPresent(bodyName) )
				cout << "Body: [" << bodyName << "] not found.\n\n";
		} while ( !m_pMocap->isBodyPresent(bodyName) );

		cout << "Loading Body: [" << bodyName << "] \n\n";

		// Add a new link to the list
		m_pBodies.append( new ViconBody(bodyName.c_str()) );
	}
}

void ViconRecorder::UpdatePosition( ViconBody *body, bool bForce /*= false */ )
{
	gmtl::Point3f posraw;	
	gmtl::Matrix44f matrix;
	gmtl::EulerAngleXYZf eoriraw;

	double pos[3];
	double eori[3];
	int posecmp[6];

	posraw = m_pMocap->getBodyPosition( body->name.toStdString() );
	matrix = m_pMocap->getBodyTM( body->name.toStdString() );

	eoriraw = gmtl::make<gmtl::EulerAngleXYZf>(matrix);

	// Positions are in METERS, Angles are in RADIANS
	pos[0] = -posraw[2];	// x = -z Vicon
	pos[1] = posraw[0];		// y = x Vicon
	pos[2] = -posraw[1];	// z = -y Vicon
	eori[0] = eoriraw[1];			// roll = pitch Vicon
	eori[1] = eoriraw[0] + PI/2;	// pitch = roll + pi/2 Vicon
	eori[2] = -eoriraw[2];			// yaw = -yaw Vicon

	// Load comparison pose data with mm precision
	posecmp[0] = pos[0]*1000;
	posecmp[1] = pos[1]*1000;
	posecmp[2] = pos[2]*1000;
	posecmp[3] = eori[0]*100;
	posecmp[4] = eori[1]*100;
	posecmp[5] = eori[2]*100;

	// Only update if we have changed positions or are forcing it
	if((posecmp[0] != body->prevPos[0] || posecmp[1] != body->prevPos[1] || posecmp[2] != body->prevPos[2] ||
		posecmp[3] != body->prevEori[0] || posecmp[4] != body->prevEori[1] || posecmp[5] != body->prevEori[2]) 
		|| bForce)
	{
		QSqlQuery query;
		query.prepare("INSERT INTO botdata (botid, x, y, z, roll, pitch, yaw) "
			"VALUES (:id, :x, :y, :z, :roll, :pitch, :yaw)");
		query.bindValue(":id", body->id);
		query.bindValue(":x", pos[0]);
		query.bindValue(":y", pos[1]);
		query.bindValue(":z", pos[2]);
		query.bindValue(":roll", eori[0]);
		query.bindValue(":pitch", eori[1]);
		query.bindValue(":yaw", eori[2]);

		query.exec();

        cout << "X: " << pos[0] <<" Y: " << pos[1] << " Z: " << pos[2] << "Roll: " << eori[0] <<" Pitch: " << eori[1] << " Yaw: " << eori[2] << endl;

		body->prevPos[0] = posecmp[0];
		body->prevPos[1] = posecmp[1];
		body->prevPos[2] = posecmp[2];
		body->prevEori[0] = posecmp[3];
		body->prevEori[1] = posecmp[4];
		body->prevEori[2] = posecmp[5];
	}
}
