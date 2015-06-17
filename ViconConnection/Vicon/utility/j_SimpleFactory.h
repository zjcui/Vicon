//========================================================================
// University of Michigan 3D Lab
// um3d@umich.edu
// http:\\um3d.dc.umich.edu
//
// Copyright (C) 2001-2006, University of Michigan 3D Lab
//========================================================================
#ifndef J_SIMPLE_FACTORY_H
#define J_SIMPLE_FACTORY_H
	//======================================================================	
	#include <vector>
	#include <direct.h>

	//======================================================================	
	#include "..\System\j_JugularDefines.h"

	//======================================================================
	// very simple factory, no loading, no paths, no assumptions except
	// for a name, and the update routine is self contained.
	//======================================================================
	template <class T> class JSimpleFactory
	{
	public:
		//------------------------------------------------------
		JSimpleFactory(void);
		~JSimpleFactory(void);

		//------------------------------------------------------
		void clearList(void);
		void updateList(void);

		//------------------------------------------------------
		T*	create(const char* const name);

		//------------------------------------------------------
		void destroy(T* ptr);
		void destroy(const char* const name);
		void destroy(const unsigned int id);
		
		//------------------------------------------------------
		T* get(const char* const name);
		T* get(const unsigned int id);

		//------------------------------------------------------
		unsigned int getID(const char* const name);
		unsigned int getID(T* ptr);

		//------------------------------------------------------
		char*	getName(void);
		void  setName(const char* const name);

		//------------------------------------------------------
		unsigned int getNumEntries(void);

	private:
		//------------------------------------------------------
		unsigned int getNextID(void);

	private:
		//------------------------------------------------------
		// main storage queue
		std::vector<typename T*>	m_list;
		std::vector<unsigned int>	m_free_list;

		//------------------------------------------------------
		char					m_name[256];
		unsigned int	m_capacity;
		unsigned int	m_num_resizes;
	};

	//======================================================================
	// CLASS BODY
	//======================================================================
	template <class T> JSimpleFactory<T>::JSimpleFactory(void) 
	{
		m_capacity = 0;
		m_num_resizes = 0;
		strcpy(m_name,"undefined factory");
	}

	//======================================================================
	template <class T> JSimpleFactory<T>::~JSimpleFactory(void) 
	{ clearList(); }

	//======================================================================
	template <class T> char* JSimpleFactory<T>::getName(void) { return m_name; }
	template <class T> void JSimpleFactory<T>::setName(const char* const name) { strcpy(m_name,name); }

	//======================================================================
	template <class T> void JSimpleFactory<T>::clearList(void) 
	{
		// cycle through entries
		std::vector<T*>::iterator itor;
		for(itor=m_list.begin(); itor<m_list.end(); ++itor)	
		{ if(*itor) { delete (*itor); }}	m_list.clear();

		// clear the librarys
		m_list.clear();
		m_free_list.clear();
	}

	//======================================================================
	template <class T> void JSimpleFactory<T>::updateList(void) 
	{
		// cycle through entries
		std::vector<T*>::iterator itor;
		for(itor=m_list.begin(); itor<m_list.end(); ++itor)	
		{ if(*itor) { (*itor)->update(); }}
	}

	//======================================================================
	template <class T> T* JSimpleFactory<T>::create(const char* const name) 
	{
		// make sure we have a valid name
		if(!name) { return 0; }

		// if sharing, check to see if it already exists
		if(do_share) { T* obj = get(name); if(obj) { return obj; }}

		// if it is, clone the reference object
		T* clone = new T(name);

		// find the next id
		unsigned int id = getNextID();
		if(id==INVALID_ID) 
		{
			// push asset onto list
			id = (unsigned int)m_list.size();
			m_list.push_back(clone);			

			// grab the capacity again and compare (debug)
			if(m_list.capacity() > m_capacity) 
			{ 
				++m_num_resizes; 
				m_capacity = (unsigned int)m_list.capacity();
			}
		}
		else	{ m_list[id] = clone; }
		
		// return our id
		clone->setID(id);
		return clone; 
	}

	//======================================================================
	template <class T> void JSimpleFactory<T>::destroy(const char* const name) 
	{	
		// grab our asset
		unsigned int id = getID(name);
		if(id==INVALID_ID) { return; }

		//delete it
		delete m_list[id]; m_list[id]=0;
		m_free_list.push_back(id);
	}

	//======================================================================
	template <class T> void JSimpleFactory<T>::destroy(const unsigned int id) 
	{	
		// make sure our id is within range
		if(id >= m_list.size()) { return; }

		// make sure id isn't already in free list
		if(m_list[id]==0) { return; }

		// delete it
		delete m_list[id]; m_list[id]=0;
		m_free_list.push_back(id);
	}

	//======================================================================
	template <class T> void JSimpleFactory<T>::destroy(T* ptr) 
	{	
		// grab our asset
		unsigned int id = getID(ptr);
		if(id==INVALID_ID) { return; }

		//delete it
		delete m_list[id]; m_list[id]=0;
		m_free_list.push_back(id);
	}

	//======================================================================
	template <class T> T* JSimpleFactory<T>::get(const char* const name) 
	{		
		std::vector<T*>::iterator itor;
		for(itor=m_list.begin(); itor<m_list.end(); ++itor)	
		{ 
			if(*itor) 
			{ if(strcmp((*itor)->getName(),name)==0) { return (*itor); }}
		}
		return 0; 
	}

	//======================================================================
	template <class T> T* JSimpleFactory<T>::get(const unsigned int id) 
	{	
		// make sure our id is within range
		if(id >= m_list.size()) { return 0; }
		return m_list[id];
	}

	//======================================================================
	template <class T> unsigned int JSimpleFactory<T>::getID(const char* const name) 
	{	
		const unsigned int size = (unsigned int)m_list.size();
		for(unsigned int i=0; i<size; ++i)
		{
			if(m_list[i]!=0)
			{ if(strcmp(name,m_list[i]->getName())==0) { return i; }}
		}

		return INVALID_ID;
	}

	//======================================================================
	template <class T> unsigned int JSimpleFactory<T>::getID(T* ptr) 
	{	
		const unsigned int size = (unsigned int)m_list.size();
		for(unsigned int i=0; i<size; ++i)
		{ if(m_list[i]!=0) { if(m_list[i]==ptr){ return i; }}}

		return INVALID_ID;
	}

	//======================================================================
	template <class T> unsigned int JSimpleFactory<T>::getNextID(void) 
	{
		// if our free list is empty, return
		if(m_free_list.begin()==m_free_list.end()) 
		{ return INVALID_ID; }

		// else grab the last id
		const unsigned int ret = m_free_list[m_free_list.size()-1];

		// pop it off the vector
		m_free_list.pop_back();

		// return our id
		return ret;
	}

	//======================================================================
	template <class T> unsigned int JSimpleFactory<T>::getNumEntries(void)
	{ return (unsigned int)m_list.size(); }

	//======================================================================
#endif