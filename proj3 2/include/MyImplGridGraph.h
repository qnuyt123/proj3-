#ifndef MT_IMPL_GRID_GRAPH_H
#define MY_IMPL_GRID_GRAPH_H

#include "GriGRaph.h"
#include <memory>

class CMyImplGridGraph : public virtual CMutableGridGraph{
	private:
		struct SImplementation;
		std::unique_ptr<SImplementation> DImplementation;
	public:
		CMyImplGridGraph(TGridCoordinate width, TGridCoordinate height, const SGridConnectionPattern &pattern);
		
