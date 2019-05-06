/*
 * ibex_CrowdingDistanceBSMOP.cpp
 *
 *  Created on: 20 oct. 2017
 *     Authors: matias y pablo
 */

#include "ibex_CrowdingDistanceBSMOP.h"
#include "ibex_OptimizerMOP.h"
#include <algorithm>    // std::min_element, std::max_element


namespace ibex { 

	int CrowdingDistanceBSMOP::nextBufferSize = 4;
	int CrowdingDistanceBSMOP::currentBufferSize = 4;
	int CrowdingDistanceBSMOP::nn = 0;

	void CrowdingDistanceBSMOP::flush() {
		while (!globalBuffer.empty()) {
			delete pop();
		}
		while (!currentBuffer.empty()) {
			delete pop();
		}
		while (!nextBuffer.empty()) {
			nextBuffer.erase(nextBuffer.begin());
		}
	}

	unsigned int CrowdingDistanceBSMOP::size() const {
		return (globalBuffer.size()+currentBuffer.size()+nextBuffer.size());
	}

	bool CrowdingDistanceBSMOP::empty() const {
		return (globalBuffer.empty() && currentBuffer.empty() && nextBuffer.empty());
	}

	void CrowdingDistanceBSMOP::push(Cell* cell) {
		
        double dist=nds->distance(cell);
		std::multiset <Cell*>::iterator it;

		int delta=0,i=0;
		if(dist < cell->get<CellMOP>().ub_distance)
		{	

			cell->get<CellMOP>().ub_distance=dist;
		}
 
        //primera iteracion
        if(globalBuffer.empty() && nextBuffer.empty() && cont==0){
			
			globalBuffer.push(cell);
			//se cambia el valor del flag cont para no entrar nuevamente
			cont=1;
		
		}else{
			
			nextBuffer.insert(cell);
			Cell* c=NULL;
			//Si el NextBuffer sobrepasa la capacidad maxima, se borran y se mueven al global
			//cout << "size next antes while: " << nextBuffer.size() << endl;
			if(nextBuffer.size() > nextBufferSize){
				ofstream myfile;
				myfile.open ("cajasDescartadas.txt", std::ios_base::app);
				while(nextBuffer.size()> nextBufferSize && currentBuffer.empty()){
					
					c=*nextBuffer.begin();
					globalBuffer.push(*nextBuffer.begin());

					// Guardando archivo 
					myfile << c->box[nn-1] << "\n" << c->box[nn-2] << "\n";
					nextBuffer.erase(nextBuffer.begin());

					// cout << "size next en while: " << nextBuffer.size() << endl;
					// cout << "size global en while: " << globalBuffer.size() << endl;
				}
				myfile.close();	
			}
		}  	

		// cout << "tamaño global: " << globalBuffer.size() << endl;
		// cout << "tamaño current: " << currentBuffer.size() << endl;
		// cout << "tamaño next: " << nextBuffer.size() << endl;
		//getchar();	
	}

	Cell* CrowdingDistanceBSMOP::pop() {
		Cell *c = NULL, *c2 = NULL;
		std::multiset <Cell*>::iterator it;
		
		//SI el current esta vacio y el next tiene elementos, se pasan del next al current
		if(currentBuffer.empty() && !nextBuffer.empty()){
		//	getchar();
			ofstream myfile;
			myfile.open ("cajasCurrent.txt");
			// Reset de archivo
			ofstream myfile2;
			myfile2.open("puntos.txt");
			myfile2.close();
			// Reset de archivo
			ofstream myfile3;
			myfile3.open("cajasDescartadas.txt");
			myfile3.close();
			// Reset de archivo
			ofstream myfile5;
			myfile5.open("global.txt");
			myfile5.close();

			c=*nextBuffer.begin();
			++c;
			c2=c;

			while(!nextBuffer.empty()){

				//it = nextBuffer.begin();
				//double distNextBegin = nds->distance(*nextBuffer.begin());
				//cout << "distancia primero: " << distNextBegin << endl;
				/*if(nextBuffer.size()>1){
					it++;
					double distNextEnd = nds->distance(*it);
					//cout << "distancia siguiente: " << distNextEnd << endl;
				}*/
				c=*nextBuffer.begin();
				currentBuffer.push(*nextBuffer.begin());

				myfile << c->box[nn-1] << "\n" << c->box[nn-2] << "\n";
	
				nextBuffer.erase(nextBuffer.begin());

			}
			myfile.close();	
		}
		
		//Si current y next estan vacios, se popea del global
		if(currentBuffer.empty() && nextBuffer.empty() && !globalBuffer.empty()){
			
			// Reset de archivo
			ofstream myfile4;
			myfile4.open("global.txt");

			c = globalBuffer.top();

			myfile4 << c->box[nn-1] << "\n" << c->box[nn-2] << "\n";
			myfile4.close();

			
			globalBuffer.pop();
			//cantBeam++;
			//cout << "BeamSearch: " << cantBeam << endl;
			//int p = c->get<CellMOP>().depth;
			//cout << "Profundidad: " << p << endl;
			//getchar();
					
		}else if(!currentBuffer.empty()){
			//si current tiene elementos, siempre se sacan de current
			c = currentBuffer.top();
			currentBuffer.pop();

		}else{
			cout << "error" << endl;
		 	exit;
		} 
		// cout << "tamaño next 2: " << nextBuffer.size() << endl;
		// cout << "tamaño current 2: " << currentBuffer.size() << endl;
		return c;
	}

  int counter2=0;
	Cell* CrowdingDistanceBSMOP::top() const {

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

    	counter2 ++;
		//cout << counter2  <<":" <<  c->get<CellMOP>().ub_distance << endl;

		return c;
	}

} // end namespace ibex