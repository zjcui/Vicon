//========================================================================
// University of Michigan 3D Lab
// um3d@umich.edu
// http:\\um3d.dc.umich.edu
//
// Copyright (C) 2001-2006, University of Michigan 3D Lab
//========================================================================
#ifndef J_RADIX_SORT_H
#define J_RADIX_SORT_H
	//======================================================================
	#include <memory.h>

	//======================================================================
	// a radix sort class that is based on the work of Pierre Terdiman  	//
	// (www.codercorner.com) & Michael Herf (www.stereopsis.com).					//
	// ------------------------------------------------------------------	//
	// example usage:																											//
	//		eRadixSort radix = new eRadixSort();														//
	//		float* input = new float[count];																//
	//		for(int i=0;i<count;++i) { input[i] = rand(); }									//
	//		radix.Sort(input,count);																				//
	// ------------------------------------------------------------------	//
	// when using multiple sort keys, the results are sorted from left to	//
	// right:																															//
	//																																		//
	//		radix.Sort(mat,4).Sort(dist,4).Sort(vb,4);											//
	//																																		//
	// would yield indices sorted by material, with each group sorted by	//
	// the distance, with each group sorted by the vertex buffer.					//
	//======================================================================
	class RadixSort
	{
	public:
		//------------------------------------------------------
		RadixSort(void);
		~RadixSort(void);

		//------------------------------------------------------
		// sorts a set of unsigned ints.											//
		//------------------------------------------------------
		RadixSort&	Sort(unsigned int* input,const unsigned int	count); 

		//------------------------------------------------------
		// sorts a set of ints.	if all values are positive		//
		// use the other sort routine. (cast as a uint)				//
		//------------------------------------------------------
		RadixSort&	Sort(int*	input,const unsigned int	count); 

		//------------------------------------------------------
		// sorts a set of floats. pass in the array and count	//
		//------------------------------------------------------
		RadixSort& Sort(float* input,const unsigned int count);

		//------------------------------------------------------
		// number of early outs due to temporal coherences.		//
		//------------------------------------------------------
		inline unsigned int	GetNumHits(void) const	
		{ return m_num_hits;	}

		//------------------------------------------------------
		// returns the sorted result.													//
		//------------------------------------------------------
		inline unsigned int* GetIndices() const	
		{ return m_indices;		}

		//------------------------------------------------------
		// clears the stored indices.													//
		//------------------------------------------------------
		void	ResetIndices();

	private:
		//------------------------------------------------------
		// resizes the indice arrays. deletes any existing.		//
		//------------------------------------------------------
		bool	Resize(const unsigned int size);

		//------------------------------------------------------
		// checks to see if we need to resize our indice list	//
		// and will call resize itself if it needs to.				//
		//------------------------------------------------------
		void	CheckResize(const unsigned int size);

		//------------------------------------------------------
		// builds our histogram values. returns false if the	//
		// histograms are valid and no need to sort. (e.g.		//
		// should we sort) returns true if we should continue	//
		// to sort.																						//
		//------------------------------------------------------
		bool	CreateHistograms(unsigned int* input,const unsigned int size);
		bool	CreateHistograms(int* input,const unsigned int size);
		bool	CreateHistograms(float* input,const unsigned int size);

		//------------------------------------------------------
		// checks to see if a pass needs to be sorted or not.	//
		// if all values have the same byte, then there is no	//
		// need to sort.this could happen with sorting bytes,	//
		// or with ints.																			//
		// -------------------------------------------------- //
		//		unsigned int = O(4*n)														//
		//						 int = O(2*n)														//
		//						byte = O(n)															//
		//					 float = O(?)															//
		//------------------------------------------------------
		bool	CheckPassValidity(unsigned int pass,void* input,const unsigned int size);

	private:
		//------------------------------------------------------
		unsigned int*			m_histogram;			// counters for each byte
		unsigned int*			m_offset;					// offset table
		unsigned int			m_current_size;		// current size of indice list
		unsigned int			m_previous_size;	// size of indice list last call
		unsigned int*			m_indices;				// two lists of indices used
		unsigned int*			m_indices_2;			//  as a double buffering system
		unsigned int			m_num_hits;				// number of hits
		unsigned int*			m_current_counter;// the current counter value
		unsigned char			m_unique_value;		// unique value used for pass checking
		void*							m_current_input;	// a pointer to the current input
	};
	//======================================================================

#endif