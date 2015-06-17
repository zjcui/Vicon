//========================================================================
// University of Michigan 3D Lab
// um3d@umich.edu
// http:\\um3d.dc.umich.edu
//
// Copyright (C) 2001-2006, University of Michigan 3D Lab
//========================================================================
#ifndef J_MOCAP_H
#define J_MOCAP_H
	//======================================================================
	// System Includes
	//======================================================================
	#include <winsock.h>

	//======================================================================
	// local Includes
	//======================================================================
	#include "math\j_Math.h"
	#include "j_MocapCodes.h"

	//======================================================================	
	class JMocap
	{

	public:
		//------------------------------------------------------
		JMocap(void);
		~JMocap(void);

		//------------------------------------------------------
		// connects to a mocap system via IP address. returns
		// false on failure.
		//------------------------------------------------------
		bool initialize(std::string ip_address);

		//------------------------------------------------------
		// disconnects from the mocap system
		//------------------------------------------------------
		void cleanup(void);

		//------------------------------------------------------
		// test whether we are connected to a mocap system.
		//------------------------------------------------------
		bool isConnected(void);

		//------------------------------------------------------
		// grabs data from the mocap system. must be called 
		// once per frame prior to grabbing body/marker data
		//------------------------------------------------------
		void update(void);

		//------------------------------------------------------
		// checks to see if a certain body exists
		//------------------------------------------------------
		bool isBodyPresent(std::string name);

		//------------------------------------------------------
		// get number of markers or bodies
		//------------------------------------------------------
		int getNumMarkers(void);
		int getNumBodies(void);

		//------------------------------------------------------
		// retrieve the transform of a particular body
		//------------------------------------------------------
		gmtl::Point3f			getBodyPosition(std::string name,bool print_errors=true);
		gmtl::Point3f			getBodyPosition(unsigned int id);

		gmtl::Quatf				getBodyOrientation(std::string name,bool print_errors=true);
		gmtl::Quatf				getBodyOrientation(unsigned int id);

		gmtl::Matrix44f		getBodyTM(std::string name,bool print_errors=true);
		gmtl::Matrix44f		getBodyTM(unsigned int id);

		//------------------------------------------------------
		// retrieve transform of a particular marker
		//------------------------------------------------------
		gmtl::Point3f getMarkerPosition(unsigned int id);

		//------------------------------------------------------
		// get occlusion amount for a particular marker. 
		//------------------------------------------------------
		double getMarkerOcclusion(unsigned int id);

		//------------------------------------------------------
		// draws the bodies or markers for easy visualization
		//------------------------------------------------------
		void drawBodies(bool value,float size=0.05);
		void drawMarkers(bool value,float size=0.05);

		//------------------------------------------------------
		// returns the elapsed # of frames since the last update
		//------------------------------------------------------
		int getCurrentFrame(void);

	protected:
		//------------------------------------------------------
		// prints out some basic stats for mocap
		//------------------------------------------------------
		void printStats(void);

	private:
		//------------------------------------------------------
		// convenience routines for receiving data
		//------------------------------------------------------
		bool recieve(SOCKET Socket, char * pBuffer, int BufferSize);
		bool recieve(SOCKET Socket, long int & Val);
		bool recieve(SOCKET Socket, unsigned long int & Val);
		bool recieve(SOCKET Socket, double & Val);

		//------------------------------------------------------
		// convenience routine for winsock 
		//------------------------------------------------------
		void printErrorCode(int code);

	private:
		//------------------------------------------------------
		int													m_frame_channel;
		int													m_current_frame;
		SOCKET											m_socket_handle;
		ClientCodes									m_client_codes;

		std::vector<std::string>		m_channel_info;
		std::vector<MarkerChannel>	m_marker_channels;
		std::vector<MarkerData>			m_marker_data;
		std::vector<BodyChannel>		m_body_channels;
		std::vector<BodyData>				m_body_data;	

//		osg::NodePtr										m_body_subtree;
//		osg::NodePtr										m_marker_subtree;
//		std::vector<osg::TransformPtr>	m_body_xforms;
//		std::vector<osg::TransformPtr>	m_marker_xforms;
	};
	//======================================================================

#endif