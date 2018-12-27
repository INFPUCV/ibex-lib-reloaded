/*
 * ibex_BeamSearchBufferMOP.cpp
 *
 *  Created on: 20 oct. 2017
 *      Author: iaraya
 */

#include "ibex_BeamSearchBufferMOP.h"
#include "ibex_OptimizerMOP.h"
#include <algorithm>    // std::min_element, std::max_element

namespace ibex {

	void BeamSearchBufferMOP::flush() {
		while (!globalBuffer.empty()) {
			delete pop();
		}
	}

	unsigned int BeamSearchBufferMOP::size() const {
		return globalBuffer.size();
	}

	bool BeamSearchBufferMOP::empty() const {
		return globalBuffer.empty();
	}

	void BeamSearchBufferMOP::push(Cell* cell) {
        double dist=nds->distance(cell);
		if(dist < cell->get<CellMOP>().ub_distance && !OptimizerMOP::_hv)
			cell->get<CellMOP>().ub_distance=dist;
        
        
        if(globalBuffer.empty() && currentBuffer.empty() && nextBuffer.empty()){
			globalBuffer.push(cell);
		}else if(nextBuffer.size()+1>4){
			
        }       nextBuffer.push(cell);

		

	}

	Cell* BeamSearchBufferMOP::pop() {

        Cell* c = top();
        globalBuffer.pop();

		return c;
	}

  int counter=0;
	Cell* BeamSearchBufferMOP::top() const {

		Cell* c = globalBuffer.top();
		if(!c) return NULL;

		if (OptimizerMOP::_hv) return c;

		double dist=nds->distance(c);

		//we update the distance and reinsert the element
		while(dist < c->get<CellMOP>().ub_distance){
			globalBuffer.pop();
			c->get<CellMOP>().ub_distance=dist;
			globalBuffer.push(c);
			c = globalBuffer.top();
			dist=nds->distance(c);
		}

    counter ++;
		//cout << counter  <<":" <<  c->get<CellMOP>().ub_distance << endl;

		return c;
	}

} // end namespace ibex