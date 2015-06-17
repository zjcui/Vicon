//========================================================================
// University of Michigan 3D Lab
// um3d@umich.edu
// http:\\um3d.dc.umich.edu
//
// Copyright (C) 2001-2006, University of Michigan 3D Lab
//========================================================================
#include <stdarg.h>

//========================================================================
#include "j_Utility.h"

//========================================================================
void readLine(FILE* f,char* string)
{	
	do { fgets(string, 255, f); } 
	while ((string[0] == ';') || (string[0] == '\n') || (string[0] == '#'));
	return;	
}

//========================================================================
char* getExtension(char* const in)
{
	// parse name from back to front finding the extension
	const int length = (int)strlen(in);
	char* p = in + (length-1);

	// separate the path from filename
	while(*p != '.') { if(p != in) --p; else break; }
	p++;

	return(p);
}

//========================================================================
void setLogLevel(unsigned int level) { LOG_LEVEL = level; }

//========================================================================
int getLogLevel(void) { return LOG_LEVEL;  }

//========================================================================
void print(int indent,const char* const text,...)
{
	if(text==NULL) { return; }
	std::string log_output = "";

	// if we're < 0, offset from last explicit level
	int offset = 0;
	if(indent<0) { offset = abs(indent); }
	else				 { LOG_LEVEL = indent; }

	// add our tabs + one for this message
	for(unsigned int i=0; i<LOG_LEVEL+offset; ++i)
	{ log_output += "  "; }

	// variables for finding arguments
	char buffer[256];
	va_list	args;

	// parses through the string for variables
	va_start(args,text);
		vsprintf(buffer,text,args);	
	va_end(args);

	// add our text + arguments
	log_output += buffer;
	log_output += "\n";
	printf(log_output.c_str());
}

//========================================================================