#include <QCoreApplication>
#include "viconrecorder.h"
#pragma comment(lib,"Ws2_32.lib")

int main(int argc, char **argv) {
	QCoreApplication app(argc, argv);

	ViconRecorder vr;

	if ( vr.Initialize() )
		// Call our main loop that continuously updates our positions
		vr.ProcessPositions();

	return 0;
 }
