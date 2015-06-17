//========================================================================
// University of Michigan 3D Lab
// um3d@umich.edu
// http:\\um3d.dc.umich.edu
//
// Copyright (C) 2001-2006, University of Michigan 3D Lab
//========================================================================
#ifndef J_MOCAP_TYPES_H
#define J_MOCAP_TYPES_H
	//======================================================================
	// System Includes
	//======================================================================
	#include <string>
	#include <vector>
	#include <functional>

	//======================================================================
	#include "Math/j_Math.h"

	//======================================================================
	// CLIENT CODES: Packet Types and Tokens
	//======================================================================
	class ClientCodes
	{
	public:
		//----------------------------------------------------------
		enum EType	 { ERequest, EReply };
		enum EPacket { EClose, EInfo, EData, EStreamOn, EStreamOff };

		//----------------------------------------------------------
		std::vector< std::string > MarkerTokens;
		std::vector< std::string > BodyTokens;

		//----------------------------------------------------------
		std::vector< std::string > MakeMarkerTokens()
		{
			std::vector< std::string > v;
			v.push_back("<P-X>");
			v.push_back("<P-Y>");
			v.push_back("<P-Z>");
			v.push_back("<P-O>");
			return v;
		}

		//----------------------------------------------------------
		std::vector< std::string > MakeBodyTokens()
		{
			std::vector< std::string > v;
			v.push_back("<A-X>");
			v.push_back("<A-Y>");
			v.push_back("<A-Z>");
			v.push_back("<T-X>");
			v.push_back("<T-Y>");
			v.push_back("<T-Z>");
			return v;
		}

		//----------------------------------------------------------
		struct CompareNames : std::binary_function<std::string, std::string, bool>
		{
			bool operator()(const std::string & a_S1, const std::string & a_S2) const
			{
				std::string::const_iterator iS1 = a_S1.begin();
				std::string::const_iterator iS2 = a_S2.begin();

				while(iS1 != a_S1.end() && iS2 != a_S2.end())
					if(toupper(*(iS1++)) != toupper(*(iS2++))) return false;

				return a_S1.size() == a_S2.size();
			}
		};
	};
	
	//======================================================================
	// MARKER CHANNEL
	//======================================================================
	class MarkerChannel
	{
	public:
		//----------------------------------------------------------
		std::string Name;

		int X;
		int Y;
		int Z;
		int O;

	public:
		//----------------------------------------------------------
		MarkerChannel(std::string & a_rName) : X(-1), Y(-1), Z(-1), O(-1), Name(a_rName) {}

		//----------------------------------------------------------
		int & operator[](int i)
		{
			switch(i)
			{
			case 0:		return X;
			case 1:		return Y;
			case 2:		return Z;
			case 3:		return O;
			default:	return O;
			}
		}

		//----------------------------------------------------------
		int operator[](int i) const
		{
			switch(i)
			{
			case 0:		return X;
			case 1:		return Y;
			case 2:		return Z;
			case 3:		return O;
			default:	return -1;
			}
		}

		//----------------------------------------------------------
		bool operator==(const std::string & a_rName) 
		{
			ClientCodes::CompareNames comparitor;
			return comparitor(Name, a_rName);
		}
	};

	//======================================================================
	// MARKER BODY
	//======================================================================
	class MarkerData
	{
	public:
		double	x,y,z;
		double	occlusion;
	};

	//======================================================================
	// BODY CHANNEL
	//======================================================================
	class BodyChannel
	{
	public:
		//----------------------------------------------------------
		std::string Name;
		int					TX,TY,TZ;
		int					RX,RY,RZ;

		//----------------------------------------------------------
		BodyChannel(std::string & a_rName) : RX(-1), RY(-1), RZ(-1), TX(-1), TY(-1), TZ(-1), Name(a_rName) {}

		//----------------------------------------------------------
		int & operator[](int i)
		{
			switch(i)
			{
			case 0:		return RX;
			case 1:		return RY;
			case 2:		return RZ;
			case 3:		return TX;
			case 4:		return TY;
			case 5:		return TZ;
			default:	return TZ;
			}
		}

		//----------------------------------------------------------
		int operator[](int i) const
		{
			switch(i)
			{
			case 0:		return RX;
			case 1:		return RY;
			case 2:		return RZ;
			case 3:		return TX;
			case 4:		return TY;
			case 5:		return TZ;
			default:	return -1;
			}
		}

		//----------------------------------------------------------
		bool operator==(const std::string & a_rName) 
		{
			ClientCodes::CompareNames comparitor;
			return comparitor(Name, a_rName);
		}
	};

	//======================================================================
	// BODY DATA
	//======================================================================
	class BodyData
	{
	public:
		// Representation of body translation
		gmtl::Point3f  pos;

		// Representation of body rotation
		// Quaternion
		//double	rx,ry,rz,rw;

		// full matrix
		gmtl::Matrix44f		xform;

		// name of body this data is related too
		std::string name;
	};

	//======================================================================
#endif