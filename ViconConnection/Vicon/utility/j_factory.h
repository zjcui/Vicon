//========================================================================
// University of Michigan 3D Lab
// um3d@umich.edu
// http:\\um3d.dc.umich.edu
//
// Copyright (C) 2001-2006, University of Michigan 3D Lab
//========================================================================
#ifndef J_FACTORY_H
#define J_FACTORY_H
	//======================================================================	
	#include <vector>
	#include <direct.h>

	//======================================================================	


	//======================================================================
	// todo: separate shared into their own list for quicker queries.
	// todo: get mechanism correct so we can store pointers to the objects
	//       instead of the objects themselves.
	//======================================================================
	template <class T> class JFactory
	{
	public:
		//------------------------------------------------------
		JFactory(void);
		~JFactory(void);

		//------------------------------------------------------
		void clearList(void);
		void updateList(const float delta=0.f);

		//------------------------------------------------------
		T*	create(const char* const name,
							 const bool do_share=true);

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
		void	setDirectory(const char* const path);
		char*	getDirectory(void);

		//------------------------------------------------------
		void  setVersion(const float ver);
		float getVersion(void);

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
		char					m_path[256];
		char					m_name[256];
		float					m_version;
		unsigned int	m_capacity;
		unsigned int	m_num_resizes;
	};

	//======================================================================
	// CLASS BODY
	//======================================================================
	template <class T> JFactory<T>::JFactory(void) 
	{
		m_version = 1.f;
		m_capacity = 0;
		m_num_resizes = 0;
		strcpy(m_path,".");
		strcpy(m_name,"undefined factory");
	}

	//======================================================================
	template <class T> JFactory<T>::~JFactory(void) 
	{ clearList(); }

	//======================================================================
	template <class T> char* JFactory<T>::getName(void) { return m_name; }
	template <class T> void JFactory<T>::setName(const char* const name) { strcpy(m_name,name); }

	//======================================================================
	template <class T> void JFactory<T>::clearList(void) 
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
	template <class T> void JFactory<T>::updateList(const float delta) 
	{
		// cycle through entries
		std::vector<T*>::iterator itor;
		for(itor=m_list.begin(); itor<m_list.end(); ++itor)	
		{ if(*itor) { (*itor)->update(delta); }}
	}

	//======================================================================
	template <class T> T* JFactory<T>::create(const char* const name,
																							const bool do_share) 
	{
		// make sure we have a valid name
		if(!name) 
		{ printf("JFactory<%s> - need to specify a name\n",m_name); return 0; }

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
	template <class T> void JFactory<T>::destroy(const char* const name) 
	{	
		// grab our asset
		unsigned int id = getID(name);
		if(id==INVALID_ID) { return; }

		//delete it
		delete m_list[id]; m_list[id]=0;
		m_free_list.push_back(id);
	}

	//======================================================================
	template <class T> void JFactory<T>::destroy(const unsigned int id) 
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
	template <class T> void JFactory<T>::destroy(T* ptr) 
	{	
		// grab our asset
		unsigned int id = getID(ptr);
		if(id==INVALID_ID) { return; }

		//delete it
		delete m_list[id]; m_list[id]=0;
		m_free_list.push_back(id);
	}

	//======================================================================
	template <class T> T* JFactory<T>::get(const char* const name) 
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
	template <class T> T* JFactory<T>::get(const unsigned int id) 
	{	
		// make sure our id is within range
		if(id >= m_list.size()) { return 0; }
		return m_list[id];
	}

	//======================================================================
	template <class T> unsigned int JFactory<T>::getID(const char* const name) 
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
	template <class T> unsigned int JFactory<T>::getID(T* ptr) 
	{	
		const unsigned int size = (unsigned int)m_list.size();
		for(unsigned int i=0; i<size; ++i)
		{ if(m_list[i]!=0) { if(m_list[i]==ptr){ return i; }}}

		return INVALID_ID;
	}

	//======================================================================
	template <class T> void JFactory<T>::setDirectory(const char* const path) 
	{	
		// grab current directory
		char* c_dir = getcwd(NULL,256);

		// composite new path
		char n_dir[256];
		sprintf(n_dir,"%s\\%s",c_dir,path);

		// check it
		if(chdir(n_dir)==-1) 
		{ printf("JFactory<%s> - couldn't set directory. [directory: %s]\n",m_name,n_dir); }
		else			
		{ strcpy(m_path,n_dir); }

		// restore old directory
		chdir(c_dir);
	}

	//======================================================================
	template <class T> char* JFactory<T>::getDirectory(void) {	return m_path; }

	//======================================================================
	template <class T> void JFactory<T>::setVersion(const float ver) { m_version = ver; }
	template <class T> float JFactory<T>::getVersion(void)					 { return m_version; }

	//======================================================================
	template <class T> unsigned int JFactory<T>::getNextID(void) 
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
	template <class T> unsigned int JFactory<T>::getNumEntries(void)
	{ return (unsigned int)m_list.size(); }

	//======================================================================
#endif