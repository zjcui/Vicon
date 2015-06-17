//========================================================================
// University of Michigan 3D Lab
// um3d@umich.edu
// http:\\um3d.dc.umich.edu
//
// Copyright (C) 2001-2006, University of Michigan 3D Lab
//========================================================================
#ifndef J_SIMPLE_SORTS_H
#define J_SIMPLE_SORTS_H
	//======================================================================
	// a simple QuickSort implementation.	returned array is sorted.				//
	//======================================================================
	void QuickSort(int array[], int left, int right);
	void QuickSort(float array[], int left, int right);

	//======================================================================
	// a simple SelectionSort implementation. returned array is sorted.		//
	//======================================================================
	void SelectionSort(int array[], int left, int right);
	void SelectionSort(float array[], int left, int right);

	//======================================================================
#endif