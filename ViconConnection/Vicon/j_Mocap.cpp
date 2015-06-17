//========================================================================
// University of Michigan 3D Lab
// um3d@umich.edu
// http:\\um3d.dc.umich.edu
//
// Copyright (C) 2001-2006, University of Michigan 3D Lab
//========================================================================
// --#pragma comment(lib,"VrtSDK10.lib")

//========================================================================

//========================================================================
#include "j_Mocap.h"
#include "utility/j_utility.h"



//========================================================================
JMocap::JMocap(void) 
{
// --	m_app = app;
  m_socket_handle = INVALID_SOCKET;
}

//========================================================================
JMocap::~JMocap(void){ cleanup(); }

//========================================================================
bool JMocap::initialize(std::string ip_address)
{
	print(SYSTEM,"- initializing motion capture system...");

	//----------------------------------
	// Build Connection to Server
	//----------------------------------
	// initialize our socket
	WSADATA data;
	WORD		version;
	version = MAKEWORD( 1, 1 ); 
	if(WSAStartup(version,&data)!=0)
	{	print(-1,"- problem initializing mocap socket"); 
		printErrorCode(WSAGetLastError()); return false; }	
	
	// define types for socket creation
	struct protoent*	protocol_info;
	char*							protocol = "tcp";
	int								type = SOCK_STREAM;

	// grab m_channel_info about our protocol
	protocol_info = getprotobyname(protocol);
	if(!protocol_info)
	{ print(-1,"- couldn't get mocap protocol"); 
		printErrorCode(WSAGetLastError()); return false; }

	// build up our socket
	m_socket_handle = socket(PF_INET, type, protocol_info->p_proto);
	if(m_socket_handle==INVALID_SOCKET)
	{ print(-1,"- couldn't create mocap socket"); 
		printErrorCode(WSAGetLastError()); cleanup(); return false; }

	// variables for connection
	struct hostent*			host_info;
	struct sockaddr_in	server;
	static const short  port = 800;

	// fill in out address info
	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port		= htons(port);
	
	// fill in our host info
	host_info = gethostbyname(ip_address.c_str());
	if(host_info)
	{ memcpy(&server.sin_addr, host_info->h_addr, host_info->h_length); }
	else
	{ server.sin_addr.s_addr = inet_addr(ip_address.c_str()); }

	// check server results
	if(server.sin_addr.s_addr == INADDR_NONE)
	{ print(-1,"- couldn't find mocap server [%s]",ip_address.c_str()); cleanup(); return false; }

	// create our socket
	int result = connect(	m_socket_handle, (struct sockaddr*) & server, sizeof(server));
	if(result==SOCKET_ERROR)
	{ print(-1,"- failed to connect to mocap server"); 
		printErrorCode(WSAGetLastError()); cleanup(); return false; }
	else
	{ print(-1,"- connected to mocap system [%s]",ip_address.c_str()); }

	//----------------------------------
	// Initial Channel Information
	//----------------------------------
	const int bufferSize = 2040;
	char buff[bufferSize];

	// build up our channel request buffer/message
	char* buffer = buff;
	*((long int *)buffer) = m_client_codes.EInfo;			buffer += sizeof(long int);
	*((long int *)buffer) = m_client_codes.ERequest;	buffer += sizeof(long int);

	// send our request for channel m_channel_info
	if(send(m_socket_handle, buff, buffer-buff, 0)==SOCKET_ERROR)
	{	print(-1,"- failed to request mocap channel info"); cleanup(); return false; }

	// variables for receiving packet/type m_channel_info
	long int info_packet;
	long int info_type;

	// grab packet
	if(!recieve(m_socket_handle, info_packet))
	{ print(-1,"- failed to receive mocap packet info"); cleanup(); return false; }
	if(info_packet!=m_client_codes.EInfo)
	{ print(-1,"- received bad reply type for mocap info"); cleanup(); return false; }

	// grab type
	if(!recieve(m_socket_handle, info_type))
	{ print(-1,"- failed to receive mocap type info"); cleanup(); return false; }
	if(info_type != m_client_codes.EReply)
	{ print(-1,"- received bad packet for mocap info"); cleanup(); return false; }

	// grab size of info 
	long int size;
	if(!recieve(m_socket_handle, size))
	{ print(-1,"- failed to receive size of mocap info"); cleanup(); return false; }
	m_channel_info.resize(size);

	// iterate through channels
	std::vector<std::string>::iterator itor;
	for(itor = m_channel_info.begin(); itor != m_channel_info.end(); itor++)
	{
		long int s;
		char c[255];
		char* p = c;

		// grab packets
		if((!recieve(m_socket_handle, s)) || (!recieve(m_socket_handle, c, s)))
		{ print(-1,"- failed to receive mocap channel info"); cleanup(); return false; }
		
		// do some point arithmatic
		p += s;		
		*p = 0;		
		*itor = std::string(c);
	}

	// call update once to get all initial markers and bodies
	this->update();

	// return success
	return true;
}

//========================================================================
void JMocap::cleanup(void) 
{
	if(!isConnected()) { return; }

	// close our socket
	if(closesocket(m_socket_handle)==SOCKET_ERROR)
	{ print(-1,"- failed to close mocap socket"); }
	m_socket_handle = INVALID_SOCKET;
}

//========================================================================
bool JMocap::isConnected(void) 
{ return (m_socket_handle==INVALID_SOCKET)?false:true; }

//========================================================================
void JMocap::update(void) 
{	
	if(!isConnected()) { return; }

	//--------------------------------
	// Parse Data
	//--------------------------------
	// iterate through channels
	std::vector<std::string>::iterator itor;
	for(itor = m_channel_info.begin(); itor != m_channel_info.end(); itor++)
	{
		// extract type
		int open_brace	= itor->find('<');
		int close_brace = itor->find('>');

		// make sure we're valid
		if((open_brace==itor->npos) || (close_brace==itor->npos))
		{ print(-1,"- invalid mocap channel id, closing connection!"); cleanup(); return; }

		// setup our brackets
		close_brace++;
		std::string type = itor->substr(open_brace, close_brace-open_brace);

		// grab the name
		std::string name = itor->substr(0, open_brace);
		int space = name.rfind(' ');
		if(space != name.npos) name.resize(space);

		// iterators for our lists
		std::vector<MarkerChannel>::iterator			marker_itor;
		std::vector<BodyChannel>::iterator				body_itor;
		std::vector<std::string>::const_iterator	type_itor;

		// make our tokens
		m_client_codes.MarkerTokens = m_client_codes.MakeMarkerTokens();
		m_client_codes.BodyTokens   = m_client_codes.MakeBodyTokens();

		// find marker and body
		marker_itor = std::find(m_marker_channels.begin(), m_marker_channels.end(), name);
		body_itor		= std::find(m_body_channels.begin(), m_body_channels.end(), name);

		//----------------------------
		// handle markers
		//----------------------------
		if(marker_itor != m_marker_channels.end())
		{
			//	The channel is for a marker we already have.
			type_itor = std::find(m_client_codes.MarkerTokens.begin(), m_client_codes.MarkerTokens.end(), type);
			if(type_itor != m_client_codes.MarkerTokens.end())
			{	marker_itor->operator[](type_itor - m_client_codes.MarkerTokens.begin()) = itor-m_channel_info.begin();	}
		}
		else
		{
			// we have new marker...
			type_itor = std::find(m_client_codes.MarkerTokens.begin(), m_client_codes.MarkerTokens.end(), type);
			if(type_itor != m_client_codes.MarkerTokens.end())
			{
				m_marker_channels.push_back(MarkerChannel(name));
				m_marker_channels.back()[type_itor - m_client_codes.MarkerTokens.begin()] = itor - m_channel_info.begin();
				//----> print(-1,"- creating new mocap marker [name: %s | type: %s]",name.c_str(),type.c_str());
			}
		}

		//----------------------------
		// handle bodies
		//----------------------------
		if(body_itor != m_body_channels.end())
		{
			//	The channel is for a marker we already have.
			type_itor = std::find(m_client_codes.BodyTokens.begin(), m_client_codes.BodyTokens.end(), type);
			if(type_itor != m_client_codes.BodyTokens.end())
			{	body_itor->operator[](type_itor - m_client_codes.BodyTokens.begin()) = itor-m_channel_info.begin();	}
		}
		else
		{
			// we have new body...
			type_itor = std::find(m_client_codes.BodyTokens.begin(), m_client_codes.BodyTokens.end(), type);
			if(type_itor != m_client_codes.BodyTokens.end())
			{
				m_body_channels.push_back(BodyChannel(name));
				m_body_channels.back()[type_itor - m_client_codes.BodyTokens.begin()] = itor - m_channel_info.begin();
				//----> print(-1,"- creating new mocap body [name: %s | type: %s]",name.c_str(),type.c_str());
			}
		}

		//----------------------------
		// handle other
		//----------------------------
		if(type=="<F>") { m_frame_channel = itor - m_channel_info.begin(); }
		// we have new channel type
		else
		{}// print(-1,"- unsupported mocap channel type [%s]",type.c_str()); }
	}

	//--------------------------------
	// Get Data
	//--------------------------------
	// resize our marker/body containers
	m_marker_data.resize(m_marker_channels.size());
	m_body_data.resize(m_body_channels.size());

	// buffer variables
	const int bufferSize = 2040;
	char buff[bufferSize];

	// build up our channel request buffer/message
	char* buffer = buff;
	*((long int *)buffer) = m_client_codes.EData;			buffer += sizeof(long int);
	*((long int *)buffer) = m_client_codes.ERequest;	buffer += sizeof(long int);

	// send our request for channel data
	if(send(m_socket_handle, buff, buffer-buff, 0)==SOCKET_ERROR)
	{	print(-1,"- failed to request mocap channel data"); cleanup(); return; }

	// variables for receiving packet/type data
	long int data_packet;
	long int data_type;

	// grab packet
	if(!recieve(m_socket_handle, data_packet))
	{ print(-1,"- failed to receive mocap packet info"); cleanup(); return; }
	if(data_packet!=m_client_codes.EData)
	{ print(-1,"- received bad reply type for mocap info"); cleanup(); return; }

	// grab type
	if(!recieve(m_socket_handle, data_type))
	{ print(-1,"- failed to receive mocap type info"); cleanup(); return; }
	if(data_type != m_client_codes.EReply)
	{ print(-1,"- received bad packet for mocap info"); cleanup(); return; }

	// grab size of data 
	long int size;
	if(!recieve(m_socket_handle, size))
	{ print(-1,"- failed to receive size of mocap data"); cleanup(); return; }

	// check size of data packet
	if(size != m_channel_info.size())
	{ print(-1,"- received bad mocap data packet"); return; }

	// build data vector
	std::vector<double> data;
	data.resize(m_channel_info.size());

	// grab actual data (finally)
	std::vector<double>::iterator data_itor;
	for(data_itor = data.begin(); data_itor != data.end(); data_itor++)
	{	
		if(!recieve(m_socket_handle, *data_itor)) 
		{ print(-1,"- received bad mocap data packet"); return; }
	}

	//----------------------------------
	// Marker Data
	//----------------------------------
	std::vector<MarkerData>::iterator					marker_data;
	std::vector<MarkerChannel>::iterator			marker_itor;
//	std::vector<osg::TransformPtr>::iterator	marker_xform;

	for(marker_itor=m_marker_channels.begin(), 
			marker_data = m_marker_data.begin();
//			marker_xform = m_marker_xforms.begin(); 
			marker_itor != m_marker_channels.end(); 
			marker_itor++, marker_data++)
	{
		// grab marker position and convert from mm to m
		marker_data->x =  data[marker_itor->X]/1000.0;
		marker_data->y =  data[marker_itor->Z]/1000.0;
		marker_data->z = -data[marker_itor->Y]/1000.0;
		marker_data->occlusion = data[marker_itor->O];
/*
--		// if we are drawing markers update their information
-		if(m_marker_subtree!=osg::NullFC) 
-		{ 
-			// test to see if lists are same, if not, rebuild
-			if(m_marker_xforms.size()!=m_marker_data.size())
-			{ print(-1,"- mocap lists out of sync, rebuilding bodies."); }
-
--			osg::TransformPtr osg_obj = osg::TransformPtr::dcast(*marker_xform);
-			osg::Matrix m = osg_obj->getMatrix();
-			m[3][0] = marker_data->x;
-			m[3][1] = marker_data->y;
-			m[3][2] = marker_data->z;
-			osg::beginEditCP(osg_obj); osg_obj->setMatrix(m);	osg::endEditCP(osg_obj);
--			marker_xform++;
		}

		// print marker data
		//print(-1,"- mocap marker: %s [%f %f %f | ",marker_itor->Name.c_str(),marker_data->X,marker_data->Y,marker_data->Z);
		//if(data[marker_itor->O] > 0.5) { printf("visible]"); } else { printf("occluded]"); }
*/
	}

	//----------------------------------
	// Body Data
	//----------------------------------
	std::vector<BodyData>::iterator						body_data;
	std::vector<BodyChannel>::iterator				body_itor;
//	std::vector<osg::TransformPtr>::iterator	body_xform;

	// cycle through body data
	for(body_itor  = m_body_channels.begin(), 
			body_data  = m_body_data.begin();
//			body_xform  = m_body_xforms.begin();
			body_itor != m_body_channels.end(); 
			body_itor++, body_data++)
	{
		// grab angle/axis
		double rx = data[body_itor->RX];
		double ry = data[body_itor->RY];
		double rz = data[body_itor->RZ];

		// convert orientation to quaternion
		double c, s, x, y, z;
		double len = sqrt(rx * rx + ry * ry + rz * rz);

		// if very small length, use identity
		if (len < 1e-15)
		{
			body_data->xform[0][0] = body_data->xform[1][1] = body_data->xform[2][2] = 1.0;
			body_data->xform[0][1] = body_data->xform[0][2] = body_data->xform[1][0] = 0.0;
			body_data->xform[1][2] = body_data->xform[2][0] = body_data->xform[2][1] = 0.0;
		}
		else
		{
			x = rx/len;
			y = ry/len;
			z = rz/len;

			c = cos(len);
			s = sin(len);

			body_data->xform[0][0] = c + (1-c)*x*x;
			body_data->xform[0][1] =     (1-c)*x*y + s*(-z);
			body_data->xform[0][2] =     (1-c)*x*z + s*y;
			body_data->xform[1][0] =     (1-c)*y*x + s*z;
			body_data->xform[1][1] = c + (1-c)*y*y;
			body_data->xform[1][2] =     (1-c)*y*z + s*(-x);
			body_data->xform[2][0] =     (1-c)*z*x + s*(-y);
			body_data->xform[2][1] =     (1-c)*z*y + s*x;
			body_data->xform[2][2] = c + (1-c)*z*z;
		}

		// store our position
		body_data->xform[0][3] = data[body_itor->TX]/1000.0;
		body_data->xform[1][3] = data[body_itor->TY]/1000.0;
		body_data->xform[2][3] = data[body_itor->TZ]/1000.0;

		// convert xform into OpenSG coordinate space
		gmtl::EulerAngleXYZf euler; euler.set(-90.f*DEGTORAD,0,0);
		gmtl::Matrix44f conv_mat =gmtl::make< gmtl::Matrix44f >(euler);
		gmtl::preMult(body_data->xform,conv_mat);

		// store name with data for conveinence
		body_data->name = body_itor->Name;

		// if we are drawing bodies update their information
		/*
--		if(m_body_subtree!=osg::NullFC) 
-		{ 
-			// test to see if lists are same, if not, rebuild
-			if(m_body_xforms.size()!=m_body_data.size())
-			{ print(-1,"- mocap lists out of sync, rebuilding bodies."); }
-
-			// assign our information to OpenSG
-			osg::TransformPtr osg_obj = osg::TransformPtr::dcast(*body_xform);
-			osg::Matrix m = osg_obj->getMatrix();
-			m.setValue(body_data->xform.getData());
-			osg::beginEditCP(osg_obj); osg_obj->setMatrix(m);	osg::endEditCP(osg_obj);
-			body_xform++;
--		}
*/
		// print body data
		//print(-1,"- mocap body: %s [%f %f %f]",body_itor->Name.c_str(),body_data->TX,body_data->TY,body_data->TZ);
	}

	//----------------------------------
	// Other Data
	//----------------------------------
	// get timestamp
  m_current_frame = data[m_frame_channel];
}

//========================================================================
bool JMocap::isBodyPresent(std::string name)
{
	std::vector<BodyData>::iterator itor;
	for(itor=m_body_data.begin(); itor!=m_body_data.end(); itor++)
	{	if(itor->name==name) { return true; }}
	return false;
}

//========================================================================
int JMocap::getNumMarkers(void) { return m_marker_channels.size(); }
int JMocap::getNumBodies(void)  { return m_body_channels.size();   }

//========================================================================
gmtl::Point3f JMocap::getBodyPosition(std::string name,bool print_errors)
{
	if(!isConnected()) { return gmtl::Point3f(); }

	std::vector<BodyData>::iterator itor;
	for(itor=m_body_data.begin(); itor!=m_body_data.end(); itor++)
	{	if(itor->name==name) { return gmtl::makeTrans<Point3f>(itor->xform); }}

	if(print_errors) { print(-1,"- mocap body not found [%s]",name.c_str()); }
	return gmtl::Point3f();
}

//========================================================================
gmtl::Point3f JMocap::getBodyPosition(unsigned int id)
{
	if(!isConnected()) { return gmtl::Point3f(); }

	// make sure we're in valid range
	if(id>=m_body_data.size()) 
	{ print(-1,"- mocap body id out of range, can't get position"); return gmtl::Point3f(); }
	return gmtl::makeTrans<Point3f>(m_body_data[id].xform);
}

//========================================================================
gmtl::Quatf JMocap::getBodyOrientation(std::string name,bool print_errors)
{
	if(!isConnected()) { return gmtl::Quatf(); }

	std::vector<BodyData>::iterator itor;
	for(itor=m_body_data.begin(); itor!=m_body_data.end(); itor++)
	{	if(itor->name==name) { return gmtl::makeRot<Quatf>(itor->xform); }}

	if(print_errors) { print(-1,"- mocap body not found [%s]",name.c_str()); }
	return gmtl::Quatf();
}

//========================================================================
gmtl::Quatf JMocap::getBodyOrientation(unsigned int id)
{
	if(!isConnected()) { return gmtl::Quatf(); }

	// make sure we're in valid range
	if(id>=m_body_data.size()) 
	{ print(-1,"- mocap body id out of range, can't get orientation"); return gmtl::Quatf(); }
	return gmtl::makeRot<Quatf>(m_body_data[id].xform);
}

//========================================================================
gmtl::Matrix44f JMocap::getBodyTM(std::string name,bool print_errors)
{
	if(!isConnected()) { return gmtl::Matrix44f(); }

	std::vector<BodyData>::iterator itor;
	for(itor=m_body_data.begin(); itor!=m_body_data.end(); itor++)
	{	if(itor->name==name) { return itor->xform; }}

	if(print_errors) { print(-1,"- mocap body not found [%s]",name.c_str()); }
	return gmtl::Matrix44f();
}

//========================================================================
gmtl::Matrix44f JMocap::getBodyTM(unsigned int id)
{
	if(!isConnected()) { return gmtl::Matrix44f(); }

	// make sure we're in valid range
	if(id>=m_body_data.size()) 
	{ print(-1,"- mocap body id out of range, can't get orientation"); return gmtl::Matrix44f(); }
	return m_body_data[id].xform;
}

//========================================================================
gmtl::Point3f JMocap::getMarkerPosition(unsigned int id)
{
	// make sure we're in valid range
	if(id>=m_marker_data.size()) 
	{ print(-1,"- mocap body id out of range, can't get position"); return gmtl::Point3f(); }
	return gmtl::Point3f(m_marker_data[id].x,m_marker_data[id].y,m_marker_data[id].z);
}

//========================================================================
double JMocap::getMarkerOcclusion(unsigned int id)
{
	// make sure we're in valid range
	if(id >= m_marker_data.size()) 
	{ print(-1,"- mocap body id out of range, can't get occlusion amount"); return 0.0; }
	return m_marker_data[id].occlusion; 
}

//========================================================================
/* void JMocap::drawBodies(const bool value,const float size) 
{
	// make sure we're connected to system
	if(!isConnected()) 
	{ print(-1,"- mocap system not connected, can't draw bodies"); return; }

	// make sure we actually have bodies to draw
	if(getNumBodies()<=0)
	{ print(-1,"- no mocap bodies to draw"); return; }

	// if we want to draw
	if(value)
	{
		// if we already are drawing, bail
		if(m_body_subtree!=osg::NullFC) { return; }

		// create subtree root
		osg::TransformPtr subtree_core = osg::Transform::create();
		m_body_subtree = osg::makeCoredNode<osg::Transform>(&subtree_core);
	
		// cycle through bodies
		std::vector<BodyData>::iterator itor;
		for(itor=m_body_data.begin(); itor!=m_body_data.end(); itor++)
		{
			// build transform
			osg::TransformPtr		body_core = osg::Transform::create();
			osg::NodePtr				body = osg::makeBox(size, size, size, 1, 1, 1);
			osg::NodePtr				body_xform = osg::makeCoredNode<osg::Transform>(&body_core);
	
			// assign to graph 
			osg::beginEditCP(body_xform);
				body_xform->addChild(body);
			osg::endEditCP(body_xform);
			m_body_subtree->addChild(body_xform);

			// update matrix
			osg::Matrix m = body_core->getMatrix();
			m.setValue(itor->xform.getData());			

			// store changes
			osg::beginEditCP(body_core);
				body_core->setMatrix(m);
			osg::endEditCP(body_core);
			m_body_xforms.push_back(body_core);
		}
			
		// add subtree to scenegraph
		osg::NodePtr root = m_app->getRootNode();
		beginEditCP(root, osg::Node::ChildrenFieldMask);
			root->addChild(m_body_subtree);
		endEditCP(root, osg::Node::ChildrenFieldMask);
	}
	else
	{
		// if we don't have subtree, bail
		if(m_body_subtree==osg::NullFC) { return; }

		// cleanup subtree
		osg::NodePtr root = m_app->getRootNode();
		beginEditCP(root, osg::Node::ChildrenFieldMask);
			root->subChild(m_body_subtree);
		endEditCP(root, osg::Node::ChildrenFieldMask);

		// reduce reference count and set to null
		osg::subRefCP(m_body_subtree);
		m_body_subtree = osg::NullFC;

		// cleanup vector
		m_body_xforms.clear();
	}
}
*/
//========================================================================
/* void JMocap::drawMarkers(const bool value,const float size)
{
	// make sure we're connected to system
	if(!isConnected()) 
	{ print(-1,"- mocap system not connected, can't draw markers"); return; }

	// make sure we actually have markers to draw
	if(getNumMarkers()<=0)
	{ print(-1,"- no mocap markers to draw"); return; }

	// if we want to draw
	if(value)
	{
		// if we already are drawing, bail
		if(m_marker_subtree!=osg::NullFC) { return; }

		// create subtree node for all mocap bodies to be under
		// use scene transform so it follows camera/navigator
		osg::TransformPtr subtree_core = osg::Transform::create();
		m_marker_subtree = osg::makeCoredNode<osg::Transform>(&subtree_core);
	
		// cycle through bodies
		std::vector<MarkerData>::iterator itor;
		for(itor=m_marker_data.begin(); itor!=m_marker_data.end(); itor++)
		{
			// build transform
			osg::TransformPtr		marker_core = osg::Transform::create();
			osg::NodePtr				marker = osg::makeSphere(0,size*0.5f);
			osg::NodePtr				marker_xform = osg::makeCoredNode<osg::Transform>(&marker_core);
	
			// assign to graph 
			osg::beginEditCP(marker_xform);
				marker_xform->addChild(marker);
			osg::endEditCP(marker_xform);
			m_marker_subtree->addChild(marker_xform);

			// update matrix
			osg::Matrix m = marker_core->getMatrix();
			m[3][0] = itor->x;
			m[3][1] = itor->y;
			m[3][2] = itor->z;
			
			// store changes
			osg::beginEditCP(marker_core);
				marker_core->setMatrix(m);
			osg::endEditCP(marker_core);
			m_marker_xforms.push_back(marker_core);
		}
			
		// add subtree to scenegraph
		osg::NodePtr root = m_app->getRootNode();
		beginEditCP(root, osg::Node::ChildrenFieldMask);
			root->addChild(m_marker_subtree);
		endEditCP(root, osg::Node::ChildrenFieldMask);
	}
	else
	{
		// if we don't have subtree, bail
		if(m_marker_subtree==osg::NullFC) { return; }

		// cleanup subtree
		osg::NodePtr root = m_app->getRootNode();
		beginEditCP(root, osg::Node::ChildrenFieldMask);
			root->subChild(m_marker_subtree);
		endEditCP(root, osg::Node::ChildrenFieldMask);

		// reduce reference count and set to null
		osg::subRefCP(m_marker_subtree);
		m_marker_subtree = osg::NullFC;

		// cleanup vector
		m_marker_xforms.clear();
	}
}
*/
//========================================================================
int JMocap::getCurrentFrame(void) { return m_current_frame; }

//========================================================================
bool JMocap::recieve(SOCKET Socket, char * pBuffer, int BufferSize)
{
	char * p = pBuffer;
	char * e = pBuffer + BufferSize;

	int result;
	while(p != e)
	{
		result = recv(Socket, p, e - p, 0 );
		if(result == SOCKET_ERROR) return false;
		p += result;
	}

	return true;
}

//========================================================================
bool JMocap::recieve(SOCKET Socket, long int & Val)
{ return recieve(Socket, (char*) & Val, sizeof(Val));	}

//========================================================================
bool JMocap::recieve(SOCKET Socket, unsigned long int & Val)
{	return recieve(Socket, (char*) & Val, sizeof(Val));	}

//========================================================================
bool JMocap::recieve(SOCKET Socket, double & Val)
{	return recieve(Socket, (char*) & Val, sizeof(Val));	}

//========================================================================
void JMocap::printErrorCode(int code)
{
	switch(code)
	{
	case 10060:	{ print(-2,"winsock: connection timeout"); } break;
	case 10061:	{ print(-2,"winsock: connection refused"); } break;
	default:    { print(-2,"winsock: %d",code); } break;
	}
}

//========================================================================
void JMocap::printStats(void)
{
	if(!isConnected()) { return; }

	printf("-------------------------------------------------------\n");
	printf(" Motion Capture Stats\n");
	printf("-------------------------------------------------------\n");
	printf("current frame:        %d\n",m_current_frame);
	
	// iterate through markers
	if(m_marker_channels.size()==0) { printf("markers:\n\t<none>\n"); }
	else
	{
		printf("markers:               \n");
		std::vector<MarkerChannel>::iterator itor;
		for(itor = m_marker_channels.begin(); itor != m_marker_channels.end(); itor++)
		{ printf("\t%s\n",itor->Name.c_str()); }
	}

	// iterate through markers
	if(m_body_channels.size()==0) 
	{ printf("bodies:\n\t<none>\n"); }
	else
	{
		printf("bodies:                \n");
		std::vector<BodyChannel>::iterator b_itor;
		for(b_itor = m_body_channels.begin(); b_itor != m_body_channels.end(); b_itor++)
		{ printf("\t%s\n",b_itor->Name.c_str()); }
	}
	printf("\n");
}

//========================================================================