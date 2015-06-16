//========================================================================
// University of Michigan 3D Lab
// um3d@umich.edu
// http:\\um3d.dc.umich.edu
//
// Copyright (C) 2001-2006, University of Michigan 3D Lab
//========================================================================
#include "j_RadixSort.h"

//======================================================================
RadixSort::RadixSort()
{
	// initialize some variables
	m_indices					= 0;
	m_indices_2				= 0;
	m_current_size		= 0;
	m_previous_size		= 0;
	m_num_hits				= 0;
	m_current_counter	= 0;
	m_unique_value		= 0;

	// allocate input-independent ram
	m_histogram	= new unsigned int[256*4];
	m_offset		= new unsigned int[256];

	// reset our indices
	ResetIndices();
}

//======================================================================
RadixSort::~RadixSort()
{
	// delete our arrays
	if(m_histogram!=0) { delete [] m_histogram; }
	if(m_offset!=0)		 { delete [] m_offset;		}
	if(m_indices!=0)	 { delete [] m_indices;		}
	if(m_indices_2!=0) { delete [] m_indices_2;	}
}

//======================================================================
RadixSort& RadixSort::Sort(unsigned int* input,const unsigned int	 count)
{
	// check validity
	if((!input) || (count==0)) { return *this; }

	// resize the lists if needed
	CheckResize(count);

	// build our histograms
	if(!CreateHistograms(input,count)) { return *this; }

	// Compute # negative values involved. (just zero)
	unsigned int num_neg_values = 0;

	// sort (i = pass number)
	for(unsigned int i=0; i<4; ++i)
	{
		// see if we should perform the pass
		if(CheckPassValidity(i,input,count))
		{
			// create offsets
			m_offset[0] = 0;
			for(unsigned int j=1; j<256; ++j)		
			{	m_offset[j] = m_offset[j-1] + m_current_counter[j-1];	}

			// init our indices
			unsigned char* input_bytes	= (unsigned char*)input;
			unsigned int*	 indices			= m_indices;
			unsigned int*  indices_end	= &m_indices[count];

			// do radix sort
			input_bytes += i;
			while(indices!=indices_end)
			{
				unsigned int id = *indices++;
				m_indices_2[m_offset[input_bytes[id<<2]]++] = id;
			}

			// Swap pointers for next pass.
			unsigned int* tmp	= m_indices;	
			m_indices		= m_indices_2; 
			m_indices_2 = tmp;
		}
	}

	// return this
	return *this;
}

//======================================================================
RadixSort&	RadixSort::Sort(int* input,const unsigned int	 count)
{
	// check validity
	if((!input) || (count==0)) { return *this; }

	// resize the lists if needed
	CheckResize(count);

	// build our histograms
	if(!CreateHistograms(input,count)) { return *this; }

	// Compute # negative values involved. (just zero)
	unsigned int num_neg_values = 0;

	// An efficient way to compute the number of negative 
	// values we'll have to deal with is simply to sum the 128
	// last values of the last histogram. Last histogram because 
	// that's the one for the Most Significant Byte, responsible for the sign.
	unsigned int* h3= &m_histogram[768];
	for(unsigned int i=128;i<256;++i)	
	{ num_neg_values += h3[i]; }

	// sort (i = pass number)
	for(unsigned int i=0; i<4; ++i)
	{
		// see if we should perform the pass
		if(CheckPassValidity(i,input,count))
		{
			if(i!=3)
			{
				// create offsets
				m_offset[0] = 0;
				for(unsigned int j=1;j<256;++j)	
				{ m_offset[j] = m_offset[j-1] + m_current_counter[j-1]; }
			}
			else
			{
				// create biased offsets, in order for negative numbers to be sorted as well
				m_offset[0] = num_neg_values;		
				for(unsigned int j=1; j<128; ++j)		
				{	m_offset[j] = m_offset[j-1] + m_current_counter[j-1]; }

				// Fixing the wrong place for negative values
				m_offset[128] = 0;
				for(unsigned int j=129;j<256;++j)
				{ m_offset[j] = m_offset[j-1] + m_current_counter[j-1]; }
			}

			// init our indices
			unsigned char* input_bytes	= (unsigned char*)input;
			unsigned int*	 indices			= m_indices;
			unsigned int*  indices_end	= &m_indices[count];

			// do radix sort
			input_bytes += i;
			while(indices!=indices_end)
			{
				unsigned int id = *indices++;
				m_indices_2[m_offset[input_bytes[id<<2]]++] = id;
			}

			// Swap pointers for next pass.
			unsigned int* tmp	= m_indices;	
			m_indices		= m_indices_2; 
			m_indices_2 = tmp;
		}

		m_current_counter = 0;
		m_unique_value		= 0;
	}

	// return this
	return *this;
}

//======================================================================
RadixSort& RadixSort::Sort(float* input2,const unsigned int count)
{ 
	// check validity
	if((!input2) || (count==0)) { return *this; }

	// resize the lists if needed
	CheckResize(count);

	// create reference to our passed buffer
	unsigned int* input = (unsigned int*)input2;

	// build our histograms
	if(!CreateHistograms(input2,count)) { return *this; }
	
	// figure out number of negative values
	unsigned int  num_neg_values = 0;
	unsigned int* h3= &m_histogram[768];
	for(unsigned int i=128;i<256;i++)	num_neg_values += h3[i];

	// sort (i = pass number)
	for(unsigned int i=0; i<4; ++i)
	{
		// if we're not in our last pass (i.e. dealing with the sign)
		if(i!=3)
		{
			// see if we should perform the pass
			if(CheckPassValidity(i,input,count))
			{
				// Create offsets
				m_offset[0] = 0;
				for(unsigned int j=1;j<256;++j)		
				{ m_offset[j] = m_offset[j-1] + m_current_counter[j-1]; }

				// setup our indices
				unsigned char* input_bytes	= (unsigned char*)input;
				unsigned int*  indices			= m_indices;
				unsigned int*  indices_end	= &m_indices[count];

				// do our sort
				input_bytes += i;
				while(indices!=indices_end)
				{
					unsigned int id = *indices++;
					m_indices_2[m_offset[input_bytes[id<<2]]++] = id;
				}

				// Swap pointers for next pass.
				unsigned int* tmp	= m_indices;	
				m_indices		= m_indices_2; 
				m_indices_2 = tmp;
			}
		}
		else
		{
			// see if we should perform the pass
			if(CheckPassValidity(i,input,count))
			{
				// Create biased offsets, in order for negative numbers to be sorted as well
				m_offset[0] = num_neg_values;	
				for(unsigned int j=1;j<128;++j)		
				{ m_offset[j] = m_offset[j-1] + m_current_counter[j-1]; }

				// We must reverse the sorting order for negative numbers!
				m_offset[255] = 0;
				for(unsigned int j=0;j<127;++j)	{ m_offset[254-j] = m_offset[255-j] + m_current_counter[255-j]; }
				for(unsigned int j=128;j<256;++j){ m_offset[j] += m_current_counter[j]; }

				// do the sort
				for(unsigned int j=0;j<count;++j)
				{
					unsigned int Radix = input[m_indices[j]]>>24;
					if(Radix<128)	{ m_indices_2[m_offset[Radix]++] = m_indices[j]; }
					else					{ m_indices_2[--m_offset[Radix]] = m_indices[j]; }
				}

				// Swap pointers for next pass.
				unsigned int* tmp	= m_indices;	
				m_indices		= m_indices_2; 
				m_indices_2 = tmp;
			}
			else
			{
				// The pass is useless, yet we still have to reverse the order of current list if all values are negative.
				if(m_unique_value >= 128)
				{
					for(unsigned int j=0;j<count;++j)	m_indices_2[j] = m_indices[count-j-1];
					
					// Swap pointers for next pass.
					unsigned int* tmp	= m_indices;	
					m_indices		= m_indices_2; 
					m_indices_2 = tmp;
				}
			}
		}
	}

	// return this
	return *this; 
}

//======================================================================
void RadixSort::ResetIndices()
{
	// cycle through reseting indices
	if(m_indices!=0) 
	{ 
		for(unsigned int i=0;i<m_current_size;i++)	
		m_indices[i] = i; 
	}
}

//======================================================================
//												PRIVATE ROUTINES															//
//======================================================================
bool RadixSort::Resize(const unsigned int size)
{ 
	// Free previously used ram
	if(m_indices!=0)	 { delete [] m_indices;		}
	if(m_indices_2!=0) { delete [] m_indices_2;	}

	// Get some fresh ones
	m_indices				= new unsigned int[size];
	m_indices_2			= new unsigned int[size];
	m_current_size	= size;

	// make sure we got arrays
	if((!m_indices)||(!m_indices_2))
	{
		if(m_indices!=0)	 { delete [] m_indices;		}
		if(m_indices_2!=0) { delete [] m_indices_2;	}
		return false;
	}

	// initialize indices so that the input buffer is read in sequential order
	ResetIndices();

	// return success
	return true;
}

//======================================================================
void RadixSort::CheckResize(const unsigned int size)
{
	// the passed size doesn't equal our previous
	if(size!=m_previous_size)
	{		
		// if greater than current size, resize it
		// else reset the indices
		if(size>m_current_size)	{ Resize(size);		}
		else										{ ResetIndices();	}

		// set our previous size
		m_previous_size = size;							
	}
}

//======================================================================
bool RadixSort::CreateHistograms(unsigned int* input,const unsigned int size)
{
	// clear existing counters
	memset(m_histogram,0,256*4*sizeof(unsigned int));

	// prepare for temporal coherence
	unsigned int prev_value = (unsigned int)input[m_indices[0]];		
	bool already_sorted = true;
	unsigned int* indices = m_indices;		

	// prepare to count
	unsigned char* p  = (unsigned char*)input;
	unsigned char* pe = &p[size*4];
	unsigned int*	 h0 = &m_histogram[0];		// Histogram for first pass (LSB)
	unsigned int*  h1 = &m_histogram[256];	// Histogram for second pass
	unsigned int*  h2 = &m_histogram[512];	// Histogram for third pass
	unsigned int*  h3 = &m_histogram[768];	// Histogram for last pass (MSB)

	// create a histogram
	while(p!=pe)
	{						
		// read input buffer in previously sorted order
		unsigned int value = (unsigned int)input[*indices++];
		
		// see if we're already sorted
		if(value < prev_value)	{ already_sorted = false; break; }

		// update for next iteration
		prev_value = value;	
							
		// create the histograms
		h0[*p++]++;
		h1[*p++]++;
		h2[*p++]++;
		h3[*p++]++;
	}

	// if we're already sorted, we just return
	if(already_sorted)	{ m_num_hits++; return false;	}	

	// else we need to continue figuring out our histograms
	while(p!=pe)
	{
		// create the histograms without the previous overhead
		h0[*p++]++;	
		h1[*p++]++;	
		h2[*p++]++;	
		h3[*p++]++;	
	}

	// need to sort
	return true;
}

//======================================================================
bool RadixSort::CreateHistograms(int* input,const unsigned int size)
{
	// clear existing counters
	memset(m_histogram,0,256*4*sizeof(unsigned int));

	// prepare for temporal coherence
	int prev_value = (int)input[m_indices[0]];		
	bool already_sorted = true;
	unsigned int* indices = m_indices;		

	// prepare to count
	unsigned char* p  = (unsigned char*)input;
	unsigned char* pe = &p[size*4];
	unsigned int*	 h0 = &m_histogram[0];		// Histogram for first pass (LSB)
	unsigned int*  h1 = &m_histogram[256];	// Histogram for second pass
	unsigned int*  h2 = &m_histogram[512];	// Histogram for third pass
	unsigned int*  h3 = &m_histogram[768];	// Histogram for last pass (MSB)

	// create a histogram
	while(p!=pe)
	{						
		// read input buffer in previously sorted order
		int value = (int)input[*indices++];
		
		// see if we're already sorted
		if(value < prev_value)	{ already_sorted = false; break; }

		// update for next iteration
		prev_value = value;	
							
		// create the histograms
		h0[*p++]++;
		h1[*p++]++;
		h2[*p++]++;
		h3[*p++]++;
	}

	// if we're already sorted, we just return
	if(already_sorted)	{ m_num_hits++; return false;	}	

	// else we need to continue figuring out our histograms
	while(p!=pe)
	{
		// create the histograms without the previous overhead
		h0[*p++]++;	
		h1[*p++]++;	
		h2[*p++]++;	
		h3[*p++]++;	
	}

	// need to sort
	return true;
}

//======================================================================
bool RadixSort::CreateHistograms(float* input,const unsigned int size)
{
	// clear existing counters
	memset(m_histogram,0,256*4*sizeof(unsigned int));

	// prepare for temporal coherence
	float prev_value = (float)input[m_indices[0]];		
	bool already_sorted = true;
	unsigned int* indices = m_indices;		

	// prepare to count
	unsigned char* p  = (unsigned char*)input;
	unsigned char* pe = &p[size*4];
	unsigned int*	 h0 = &m_histogram[0];		// Histogram for first pass (LSB)
	unsigned int*  h1 = &m_histogram[256];	// Histogram for second pass
	unsigned int*  h2 = &m_histogram[512];	// Histogram for third pass
	unsigned int*  h3 = &m_histogram[768];	// Histogram for last pass (MSB)

	// create a histogram
	while(p!=pe)
	{						
		// read input buffer in previously sorted order
		float value = (float)input[*indices++];
		
		// see if we're already sorted
		if(value < prev_value)	{ already_sorted = false; break; }

		// update for next iteration
		prev_value = value;	
							
		// create the histograms
		h0[*p++]++;
		h1[*p++]++;
		h2[*p++]++;
		h3[*p++]++;
	}

	// if we're already sorted, we just return
	if(already_sorted)	{ m_num_hits++; return false;	}	

	// else we need to continue figuring out our histograms
	while(p!=pe)
	{
		// create the histograms without the previous overhead
		h0[*p++]++;	
		h1[*p++]++;	
		h2[*p++]++;	
		h3[*p++]++;	
	}

	// need to sort
	return true;
}

//======================================================================
bool RadixSort::CheckPassValidity(unsigned int pass,void* input,const unsigned int size)
{
	// shortcut to get current counters
	m_current_counter = &m_histogram[pass<<8];

	// get the first byte
	m_unique_value = *(((unsigned char*)input)+pass);	

	// check the byte count
	if(m_current_counter[m_unique_value]==size)	{ return false; }

	// return
	return true;
}

//======================================================================
