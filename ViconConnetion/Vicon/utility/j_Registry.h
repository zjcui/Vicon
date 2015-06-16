//========================================================================
// University of Michigan 3D Lab
// um3d@umich.edu
// http:\\um3d.dc.umich.edu
//
// Copyright (C) 2001-2006, University of Michigan 3D Lab
//========================================================================
#ifndef J_REGISTRY_H
#define J_REGISTRY_H
	//======================================================================	
	#include <map>
	#include <vector>
	#include <direct.h>

	//======================================================================
	// todo: separate shared into their own list for quicker queries.
	// todo: additional create routine for "file defined" type
	// todo: proper sharing behavior. (i.e. reference counting)
	//======================================================================
	template <class T> class JRegistry
	{
	public:
		//------------------------------------------------------
		JRegistry(void);
		~JRegistry(void);

		//------------------------------------------------------
		void ClearList(void);
		void ClearRegistry(void);

		//------------------------------------------------------
		T*	Create(const char* const type,
							 const char* const name,
							 const bool do_share=true);

		//------------------------------------------------------
		void Destroy(T* ptr);
		void Destroy(const char* const name);
		void Destroy(const unsigned int id);
		
		//------------------------------------------------------
		T* Get(const char* const name);
		T* Get(const unsigned int id);

		//------------------------------------------------------
		unsigned int GetID(const char* const name);
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

		//------------------------------------------------------
		bool Register(const char* const type,T* ptr);
		bool IsRegistered(const char* const type);

	private:
		//------------------------------------------------------
		unsigned int GetNextID(void);

	private:
		//------------------------------------------------------
		// registry structures
		typedef std::map<const char*,typename T*>	RegMap;
		typedef typename RegMap::value_type				RegValueType;
		typedef	typename RegMap::iterator					RegItor;

		//------------------------------------------------------
		// main storage queue
		std::vector<typename T*>	m_list;
		std::vector<unsigned int>	m_free_list;

		//------------------------------------------------------
		RegMap				m_registry;
		char					m_path[256];
		char					m_name[256];
		float					m_version;
		unsigned int	m_capacity;
		unsigned int	m_num_resizes;
	};

	//======================================================================
	// CLASS BODY
	//======================================================================
	template <class T> JRegistry<T>::JRegistry(void) 
	{
		m_version = 1.f;
		m_capacity = 0;
		m_num_resizes = 0;
		strcpy(m_path,".");
		strcpy(m_name,"undefined factory");
	}

	//======================================================================
	template <class T> JRegistry<T>::~JRegistry(void) 
	{ ClearList(); ClearRegistry(); }

	//======================================================================
	template <class T> char* JRegistry<T>::GetName(void) { return m_name; }
	template <class T> void JRegistry<T>::SetName(const char* const name) { strcpy(m_name,name); }

	//======================================================================
	template <class T> void JRegistry<T>::ClearList(void) 
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
	template <class T> void JRegistry<T>::ClearRegistry(void) 
	{
		// delete entries
		for(RegItor itor = m_registry.begin(); itor != m_registry.end(); ++itor)
		{	if((*itor).second) { delete (*itor).second; }}

		// clear map
		m_registry.clear();
	}

	//======================================================================
	template <class T> T* JRegistry<T>::Create(const char* const type,
																							 const char* const name,
																							 const bool do_share) 
	{
		// make sure we have a valid name
		if(!name) 
		{ printf("JRegistry<%s> - need to specify a name. [type: %s]",m_name,type); return 0; }

		// if sharing, check to see if it already exists
		if(do_share) { T* obj = Get(name); if(obj) { return obj; }}
    
		// see if type is registered
		RegItor itor = m_registry.find(type);
		if(itor==m_registry.end())
		{ printf("JRegistry<%s> - desired type isn't registered with the factory. [type: %s]\n",m_name,type); return 0; }
		
		// if it is, clone the reference object
		T* clone = (*itor).second->Clone(name);

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
	template <class T> void JRegistry<T>::Destroy(const char* const name) 
	{	
		// grab our asset
		unsigned int id = GetID(name);
		if(id==INVALID_ID) { return; }

		//delete it
		delete m_list[id]; m_list[id]=0;
		m_free_list.push_back(id);
	}

	//======================================================================
	template <class T> void JRegistry<T>::Destroy(const unsigned int id) 
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
	template <class T> void JRegistry<T>::Destroy(T* ptr) 
	{	
		// grab our asset
		unsigned int id = GetID(ptr);
		if(id==INVALID_ID) { return; }

		//delete it
		delete m_list[id]; m_list[id]=0;
		m_free_list.push_back(id);
	}

	//======================================================================
	template <class T> T* JRegistry<T>::Get(const char* const name) 
	{		
		std::vector<T*>::iterator itor;
		for(itor=m_list.begin(); itor<m_list.end(); ++itor)	
		{ 
			if(*itor) 
			{ if(strcmp((*itor)->GetName(),name)==0) { return (*itor); }}
		}
		return 0; 
	}

	//======================================================================
	template <class T> T* JRegistry<T>::Get(const unsigned int id) 
	{	
		// make sure our id is within range
		if(id >= m_list.size()) { return 0; }
		return m_list[id];
	}

	//======================================================================
	template <class T> unsigned int JRegistry<T>::GetID(const char* const name) 
	{	
		const unsigned int size = (unsigned int)m_list.size();
		for(unsigned int i=0; i<size; ++i)
		{
			if(m_list[i]!=0)
			{ if(strcmp(name,m_list[i]->GetName())==0) { return i; }}
		}

		return INVALID_ID;
	}

	//======================================================================
	template <class T> unsigned int JRegistry<T>::GetID(T* ptr) 
	{	
		const unsigned int size = (unsigned int)m_list.size();
		for(unsigned int i=0; i<size; ++i)
		{ if(m_list[i]!=0) { if(m_list[i]==ptr){ return i; }}}

		return INVALID_ID;
	}

	//======================================================================
	template <class T> void JRegistry<T>::SetDirectory(const char* const path) 
	{	
		// grab current directory
		char* c_dir = getcwd(NULL,256);

		// composite new path
		char n_dir[256];
		sprintf(n_dir,"%s\\%s",c_dir,path);

		// check it
		if(chdir(n_dir)==-1) 
		{ printf("JRegistry<%s> - couldn't set directory. [directory: %s]\n",m_name,n_dir); }
		else			
		{ strcpy(m_path,n_dir); }

		// restore old directory
		chdir(c_dir);
	}

	//======================================================================
	template <class T> char* JRegistry<T>::GetDirectory(void) {	return m_path; }

	//======================================================================
	template <class T> void JRegistry<T>::SetVersion(const float ver) { m_version = ver; }
	template <class T> float JRegistry<T>::GetVersion(void)					 { return m_version; }

	//======================================================================
	template <class T> bool JRegistry<T>::Register(const char* const type,T* class_ref) 
	{
		RegItor itor = m_registry.find(type);
		if(itor!=m_registry.end()) 
		{ printf("JRegistry<%s> - type already registered. [type: %s]\n",m_name,type); return true; }
		
		// if not add it to the registry
		m_registry.insert(RegValueType(type,class_ref));
		return true;
	}

	//======================================================================
	template <class T> bool JRegistry<T>::IsRegistered(const char* const type) 
	{
		RegItor itor = m_registry.find(type);
		return (itor!=m_registry.end())?true:false; 
	}

	//======================================================================
	template <class T> unsigned int JRegistry<T>::GetNextID(void) 
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