//========================================================================
// University of Michigan 3D Lab
// um3d@umich.edu
// http:\\um3d.dc.umich.edu
//
// Copyright (C) 2001-2006, University of Michigan 3D Lab
//========================================================================
#ifndef J_MANAGER_H
#define J_MANAGER_H
	//======================================================================	
	#include <vector>
	#include <direct.h>

	//======================================================================
	template <class T> class JManager
	{
		//------------------------------------------------------
		struct JManagerEntry
		{
			T*		ptr;
			int		ref;
			char	name[256];

			JManagerEntry(void) { ptr=0; ref=-1; strcpy(name,"undefined"); }
			~JManagerEntry(void) { if(ptr) delete ptr; }
		};

	public:
		//------------------------------------------------------
		JManager(void);
		~JManager(void);

		//------------------------------------------------------
		void ClearList(void);

		//------------------------------------------------------
		unsigned int	Add(const char* const name, T* ptr,
											const bool do_share=true);

		//------------------------------------------------------
		void Remove(T* ptr);
		void Remove(const char* const name);
		void Remove(const unsigned int id);
		
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
		unsigned int  GetSize(void);
		unsigned int  GetCapacity(void);
		unsigned int  GetNumResizes(void);

	private:
		//------------------------------------------------------
		unsigned int GetNextID(void);

	private:
		//------------------------------------------------------
		// main storage queue
		std::vector<JManagerEntry*>	m_list;
		std::vector<unsigned int>			m_free_list;

		//------------------------------------------------------
		char					m_path[256];
		char					m_name[256];
		unsigned int	m_capacity;
		unsigned int	m_num_resizes;
	};

	//======================================================================
	// CLASS BODY
	//======================================================================
	template <class T> JManager<T>::JManager(void) 
	{
		m_capacity = 0;
		m_num_resizes = 0;
		strcpy(m_path,".");
		strcpy(m_name,"undefined manager");
	}

	//======================================================================
	template <class T> JManager<T>::~JManager(void) 
	{ ClearList(); }

	//======================================================================
	template <class T> char* JManager<T>::GetName(void) { return m_name; }
	template <class T> void JManager<T>::SetName(const char* const name) { strcpy(m_name,name); }

	//======================================================================
	template <class T> unsigned int JManager<T>::GetSize(void)			 { return (unsigned int)m_list.size(); }
	template <class T> unsigned int JManager<T>::GetCapacity(void)   { return m_capacity;    } 
	template <class T> unsigned int JManager<T>::GetNumResizes(void) { return m_num_resizes; }

	//======================================================================
	template <class T> void JManager<T>::ClearList(void) 
	{
		// cycle through entries
		std::vector<JManagerEntry*>::iterator itor;
		for(itor=m_list.begin(); itor<m_list.end(); ++itor)	
		{ if(*itor) { delete (*itor); }}	m_list.clear();

		// clear the libraries
		m_free_list.clear();
	}

	//======================================================================
	template <class T> unsigned int JManager<T>::Add(const char* const name, T* ptr,
																										 const bool do_share) 
	{
		// make sure we have a valid name
		if(!name) 
		{ printf("JManager<%s> - need to specify a name",m_name); return 0; }

		// if sharing, check to see if it already exists
		if(do_share) 
		{ 
			// find object
			for(unsigned int i=0; i<m_list.size(); ++i)
			{
				if(m_list[i])
				{	
					if(strcmp(m_list[i]->name,name)==0)
					{ m_list[i]->ref++; return i; }
				}
			}
		}

		// create new entry
		JManagerEntry* entry = new JManagerEntry();
		entry->ref = 1;
		entry->ptr = ptr;
		strcpy(entry->name,name);

		// find the next id
		unsigned int id = GetNextID();
		if(id==INVALID_ID) 
		{
			// push asset onto list
			id = (unsigned int)m_list.size();
			m_list.push_back(entry);			

			// grab the capacity again and compare (debug)
			if(m_list.capacity() > m_capacity) 
			{ 
				++m_num_resizes; 
				m_capacity = (unsigned int)m_list.capacity();
			}
		}
		else	{ m_list[id] = entry; }
		
		// return our id
		return id; 
	}

	//======================================================================
	template <class T> void JManager<T>::Remove(const char* const name) 
	{	
		// grab our asset
		unsigned int id = GetID(name);
		if(id==INVALID_ID) { return; }

		// reduce ref count
		m_list[id]->ref--;

		//delete it
		if(m_list[id]->ref<=0)
		{	delete m_list[id]; m_list[id]=0; 
			m_free_list.push_back(id); }
	}

	//======================================================================
	template <class T> void JManager<T>::Remove(const unsigned int id) 
	{	
		// make sure our id is within range
		if(id >= m_list.size()) { return; }

		// make sure id isn't already in free list
		if(m_list[id]==0) { return; }

		// reduce ref count
		m_list[id]->ref--;

		//delete it
		if(m_list[id]->ref<=0)
		{	delete m_list[id]; m_list[id]=0; 
			m_free_list.push_back(id); }
	}

	//======================================================================
	template <class T> void JManager<T>::Remove(T* ptr) 
	{	
		// grab our asset
		unsigned int id = GetID(ptr);
		if(id==INVALID_ID) { return; }

		// reduce ref count
		m_list[id]->ref--;

		//delete it
		if(m_list[id]->ref<=0)
		{	delete m_list[id]; m_list[id]=0; 
			m_free_list.push_back(id); }
	}

	//======================================================================
	template <class T> T* JManager<T>::Get(const char* const name) 
	{		
		std::vector<JManagerEntry*>::iterator itor;
		for(itor=m_list.begin(); itor<m_list.end(); ++itor)	
		{ 
			if(*itor) 
			{ if(strcmp((*itor)->name,name)==0) { return (*itor)->ptr; }}
		}
		return 0; 
	}

	//======================================================================
	template <class T> T* JManager<T>::Get(const unsigned int id) 
	{	
		// make sure our id is within range
		if(id >= m_list.size()) { return 0; }
		return m_list[id]->ptr;
	}

	//======================================================================
	template <class T> unsigned int JManager<T>::GetID(const char* const name) 
	{	
		const unsigned int size = (unsigned int)m_list.size();
		for(unsigned int i=0; i<size; ++i)
		{
			if(m_list[i]!=0)
			{ if(strcmp(name,m_list[i]->name)==0) { return i; }}
		}

		return INVALID_ID;
	}

	//======================================================================
	template <class T> unsigned int JManager<T>::GetID(T* ptr) 
	{	
		const unsigned int size = (unsigned int)m_list.size();
		for(unsigned int i=0; i<size; ++i)
		{ if(m_list[i]!=0) { if(m_list[i]->ptr==ptr){ return i; }}}

		return INVALID_ID;
	}

	//======================================================================
	template <class T> void JManager<T>::SetDirectory(const char* const path) 
	{	
		// grab current directory
		char* c_dir = getcwd(NULL,256);

		// composite new path
		char n_dir[256];
		sprintf(n_dir,"%s\\%s",c_dir,path);

		// check it
		if(chdir(n_dir)==-1) 
		{ printf("JManager<%s> - couldn't set directory. [directory: %s]\n",m_name,n_dir); }
		else			
		{ strcpy(m_path,n_dir); }

		// restore old directory
		chdir(c_dir);
	}

	//======================================================================
	template <class T> char* JManager<T>::GetDirectory(void) {	return m_path; }

	//======================================================================
	template <class T> unsigned int JManager<T>::GetNextID(void) 
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