//========================================================================
// University of Michigan 3D Lab
// um3d@umich.edu
// http:\\um3d.dc.umich.edu
//
// Copyright (C) 2001-2006, University of Michigan 3D Lab
//========================================================================
#ifndef J_BUCKET_H
#define J_BUCKET_H
	//======================================================================	
	#include <vector>
	#include <direct.h>

	//======================================================================
	template <class T> class JBucket
	{
	public:
		//------------------------------------------------------
		JBucket(void);
		~JBucket(void);

		//------------------------------------------------------
		void ClearList(void);

		//------------------------------------------------------
		T*	Create(void);

		//------------------------------------------------------
		void Destroy(T* ptr);
		void Destroy(const unsigned int id);
		
		//------------------------------------------------------
		T* Get(const unsigned int id);

		//------------------------------------------------------
		unsigned int GetID(T* ptr);

		//------------------------------------------------------
		char*	GetName(void);
		void  SetName(const char* const name);

		//------------------------------------------------------
		void	SetDirectory(const char* const path);
		char*	GetDirectory(void);

		//------------------------------------------------------
		void  SetVersion(const float ver);
		float GetVersion(void);

	private:
		//------------------------------------------------------
		unsigned int GetNextID(void);

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
	template <class T> JBucket<T>::JBucket(void) 
	{
		m_version = 1.f;
		m_capacity = 0;
		m_num_resizes = 0;
		strcpy(m_path,".");
		strcpy(m_name,"undefined factory");
	}

	//======================================================================
	template <class T> JBucket<T>::~JBucket(void) 
	{ ClearList(); }

	//======================================================================
	template <class T> char* JBucket<T>::GetName(void) { return m_name; }
	template <class T> void JBucket<T>::SetName(const char* const name) { strcpy(m_name,name); }

	//======================================================================
	template <class T> void JBucket<T>::ClearList(void) 
	{
		// cycle through entries
		std::vector<T*>::iterator itor;
		for(itor=m_list.begin(); itor<m_list.end(); ++itor)	
		{ delete (*itor); }	m_list.clear();

		// clear the librarys
		m_list.clear();
		m_free_list.clear();
	}

	//======================================================================
	template <class T> T* JBucket<T>::Create(void) 
	{
		// if it is, clone the reference object
		T* clone = new T();

		// find the next id
		unsigned int id = GetNextID();
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
		clone->SetID(id);
		return clone; 
	}

	//======================================================================
	template <class T> void JBucket<T>::Destroy(const unsigned int id) 
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
	template <class T> void JBucket<T>::Destroy(T* ptr) 
	{	
		// grab our asset
		unsigned int id = GetID(ptr);
		if(id==INVALID_ID) { return; }

		//delete it
		delete m_list[id]; m_list[id]=0;
		m_free_list.push_back(id);
	}

	//======================================================================
	template <class T> T* JBucket<T>::Get(const unsigned int id) 
	{	
		// make sure our id is within range
		if(id >= m_list.size()) { return 0; }
		return m_list[id];
	}

	//======================================================================
	template <class T> unsigned int JBucket<T>::GetID(T* ptr) 
	{	
		const unsigned int size = (unsigned int)m_list.size();
		for(unsigned int i=0; i<size; ++i)
		{ if(m_list[i]!=0) { if(m_list[i]==ptr){ return i; }}}

		return INVALID_ID;
	}

	//======================================================================
	template <class T> void JBucket<T>::SetDirectory(const char* const path) 
	{	
		// grab current directory
		char* c_dir = getcwd(NULL,256);

		// composite new path
		char n_dir[256];
		sprintf(n_dir,"%s\\%s",c_dir,path);

		// check it
		if(chdir(n_dir)==-1) 
		{ printf("JBucket<%s> - couldn't set directory. [directory: %s]\n",m_name,n_dir); }
		else			
		{ strcpy(m_path,n_dir); }

		// restore old directory
		chdir(c_dir);
	}

	//======================================================================
	template <class T> char* JBucket<T>::GetDirectory(void) {	return m_path; }

	//======================================================================
	template <class T> void JBucket<T>::SetVersion(const float ver) { m_version = ver; }
	template <class T> float JBucket<T>::GetVersion(void)					 { return m_version; }

	//======================================================================
	template <class T> unsigned int JBucket<T>::GetNextID(void) 
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
#endif