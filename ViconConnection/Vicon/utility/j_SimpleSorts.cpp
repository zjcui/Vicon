//========================================================================
// University of Michigan 3D Lab
// um3d@umich.edu
// http:\\um3d.dc.umich.edu
//
// Copyright (C) 2001-2006, University of Michigan 3D Lab
//========================================================================
#include "j_SimpleSorts.h"

//======================================================================
void QuickSort(int array[], int left, int right)
{
	// check for early out
  if (left >= right) return;

	// couple variables
  int i, j;
  int num_left, num_right;
  int pivot, temp;

	// see if we have anything to do
	while (left < right)
  {
		i = left;
		j = right + 1;
		pivot = array[left];
                            
		while(true)
		{
			do { i++; } while (array[i] < pivot);
      do { j--; } while (array[j] > pivot);
      
      // check for break
      if (i >= j) break;

			// else swap values
      temp = array[i];
      array[i] = array[j];
      array[j] = temp;
    }
        
    array[left] = array[j];
    array[j] = pivot;
        
		num_left  = (j-1) - left;
    num_right = right - (j+1);
      
		// see if we nee to continue
    if(num_left <= num_right) {	QuickSort(array, left, j-1);  left  = j+1; }
    else									    { QuickSort(array, j+1, right); right = j-1; }
	}
}

//======================================================================
void QuickSort(float array[], int left, int right)
{
	// check for early out
  if (left >= right) return;

	// couple variables
  int i, j;
  int num_left, num_right;
  float pivot,temp;

	// see if we have anything to do
	while (left < right)
  {
		i = left;
		j = right + 1;
		pivot = array[left];
                            
		while(true)
		{
			do { i++; } while (array[i] < pivot);
      do { j--; } while (array[j] > pivot);
      
      // check for break
      if (i >= j) break;

			// else swap values
      temp = array[i];
      array[i] = array[j];
      array[j] = temp;
    }
        
    array[left] = array[j];
    array[j] = pivot;
        
		num_left  = (j-1) - left;
    num_right = right - (j+1);
      
		// see if we nee to continue
    if(num_left <= num_right) {	QuickSort(array, left, j-1);  left  = j+1; }
    else									    { QuickSort(array, j+1, right); right = j-1; }
	}
}

//======================================================================
void SelectionSort(int array[], int left, int right)
{
	// cycle through array
	for(int i=left; i<right; i++) 
	{
		// setup min
		int min = i;

		// find min
		for(int j=i+1; j<=right; j++)
		{	if(array[j] < array[min]) { min = j; }}

		// swap values
		int temp	 = array[min];
		array[min] = array[i];
		array[i]	 = temp;
	}
}

//======================================================================
void SelectionSort(float array[], int left, int right)
{
	// cycle through array
	for(int i=left; i<right; i++) 
	{
		// setup min
		int min = i;

		// find min
		for(int j=i+1; j<=right; j++)
		{	if(array[j] < array[min]) { min = j; }}

		// swap values
		float temp	= array[min];
		array[min]	= array[i];
		array[i]		= temp;
	}
}

//======================================================================
