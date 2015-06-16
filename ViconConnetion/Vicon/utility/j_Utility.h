//========================================================================
// University of Michigan 3D Lab
// um3d@umich.edu
// http:\\um3d.dc.umich.edu
//
// Copyright (C) 2001-2006, University of Michigan 3D Lab
//========================================================================
#ifndef J_UTILITY_H
#define J_UTILITY_H
#include <stdio.h>
#include <iostream>
	//======================================================================


	//======================================================================
	// reads a single line of text from the passed file
	//======================================================================
	void readLine(FILE* f,char* string);

	//======================================================================
	// returns the extension of the passed in filename
	//======================================================================
	char* getExtension(char* const in);

	//======================================================================
	// prints log messages to the console. user can specify a specific
	// level of indentation with: KERNEL, SYSTEM, SUBSYSTEM. if user 
	// specifies a negative # that is used as an offset of the last 
	// explicit level that was set.
	//======================================================================
	static const unsigned int KERNEL		= 0;
	static const unsigned int SYSTEM		= 1;
	static const unsigned int SUBSYSTEM	= 2;
	static unsigned int				LOG_LEVEL = 0;

	//======================================================================
	void setLogLevel(unsigned int level);
	int  getLogLevel(void);

	//======================================================================
	void print(int indent,const char* const text,...);

	//======================================================================

#endif
