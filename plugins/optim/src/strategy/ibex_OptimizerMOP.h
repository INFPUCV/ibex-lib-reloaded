//============================================================================
//                                  I B E X
// File        : ibex_Optimizer.h
// Author      : Matias Campusano, Damir Aliquintui, Ignacio Araya
// Copyright   : IMT Atlantique (France)
// License     : See the LICENSE file
// Created     : Sep 24, 2017
// Last Update : Sep 24, 2017
//============================================================================

#ifndef __IBEX_OPTIMIZERMOP_H__
#define __IBEX_OPTIMIZERMOP_H__

#include "ibex_Ctc.h"
#include "ibex_Bsc.h"
#include "ibex_LoupFinderMOP.h"
#include "ibex_CellBufferOptim.h"
#include "ibex_CellSet.h"
//#include "ibex_EntailedCtr.h"
#include "ibex_CtcKhunTucker.h"

#include <set>
#include <map>
#include <list>

using namespace std;
namespace ibex {

class point2{
public:
	Interval x;
	Interval y;

	point2()  : x(0), y(0)  {}
	point2(double x, double y) : x(Interval(x)), y(Interval(y)) { }
	point2(Interval x, Interval y) : x(x), y(y) { }

	point2 operator+(const point2& p2) const {
		return point2( x+p2.x, y+p2.y );
	}

	point2 operator-(const point2& p2) const {
		return point2( x-p2.x, y-p2.y );
	}

	Interval operator*(const point2& p2) const {
		return ( x*p2.y - y*p2.x );
	}

	bool operator<(const point2& p2) const {
		if(x.mid()!=p2.x.mid()) return x.mid() < p2.x.mid();
		if(y.mid()!=p2.y.mid()) return y.mid() > p2.y.mid();
		return false;
	}

};

struct sorty{
	bool operator()(const pair<double,double> p1, const pair<double,double> p2){
		return p1.second>p2.second;
	}
};


/**
 * \defgroup optim IbexOpt
 */

/**
 * \ingroup optim
 *
 * \brief Global MOP Optimizer.
 *
 */
class OptimizerMOP {

public:

	/**
	 * \brief Return status of the optimizer
	 *
	 * See comments for optimize(...) below.
	 */
	typedef enum {SUCCESS, INFEASIBLE, NO_FEASIBLE_FOUND, UNBOUNDED_OBJ, TIME_OUT, UNREACHED_PREC} Status;

	/**
	 *  \brief Create an optimizer.
	 *
	 * Inputs:
	 *   \param n        - number of variables or the <b>original system</b>
	 *   \param f1	     - the objective function f1
     *	 \param f2       - the objective function f2
	 *   \param ctc      - contractor for <b>extended<b> boxes (of size n+2)
	 *   \param bsc      - bisector for <b>extended<b> boxes (of size n+2)
	 *   \param buffer   - buffer for <b>extended<b> boxes (of size n+2)
	 *   \param finder   - the finder of ub solutions
	 *   \param eps	     - the required precision
	 *

	 *
	 * \warning The optimizer relies on the contractor \a ctc to contract the domain of the goal variable
	 *          and increase the uplo. If this contractor never contracts this goal variable,
	 *          the optimizer will only rely on the evaluation of f and will be very slow.
	 *
	 * We are assuming that the objective variables are n and n+1
	 *
	 */
	OptimizerMOP(int n, const Array<NumConstraint>& ctcs, const Function &f1,  const Function &f2,
			Ctc& ctc, Bsc& bsc, CellBufferOptim& buffer, LoupFinderMOP& finder,  double eps=default_eps);

	/**
	 * \brief Delete *this.
	 */
	virtual ~OptimizerMOP();

	/**
	 * \brief Run the optimization.
	 *
	 * \param init_box             The initial box
	 * \param obj_init_bound       (optional) can be set when an initial upper bound of the objective minimum is known a priori.
	 *                             This bound can be obtained, e.g., by a local solver. This is equivalent to (but more practical
	 *                             than) adding a constraint f(x)<=obj_init_bound.
	 *
	 * \return SUCCESS             if the global minimum (with respect to the precision required) has been found.
	 *                             In particular, at least one feasible point has been found, less than obj_init_bound, and in the
	 *                             time limit.
	 *
	 *         INFEASIBLE          if no feasible point exist less than obj_init_bound. In particular, the function returns INFEASIBLE
	 *                             if the initial bound "obj_init_bound" is LESS than the true minimum (this case is only possible if
	 *                             goal_abs_prec and goal_rel_prec are 0). In the latter case, there may exist feasible points.
	 *
	 *         NO_FEASIBLE_FOUND   if no feasible point could be found less than obj_init_bound. Contrary to INFEASIBLE,
	 *                             infeasibility is not proven here. Warning: this return value is sensitive to the abs_eps_f and
	 *                             rel_eps_f parameters. The upperbounding makes the optimizer only looking for points less than
	 *                             min { (1-rel_eps_f)*obj_init_bound, obj_init_bound - abs_eps_f }.
	 *
	 *         UNBOUNDED_OBJ       if the objective function seems unbounded (tends to -oo).
	 *
	 *         TIMEOUT             if time is out.
	 *
	 *         UNREACHED_PREC      if the search is over but the resulting interval [uplo,loup] does not satisfy the precision
	 *                             requirements. There are several possible reasons: the goal function may be too pessimistic
	 *                             or the constraints function may be too pessimistic with respect to the precision requirement
	 *                             (which can be too stringent). This results in tiny boxes that can neither be contracted nor
	 *                             used as new loup candidates. Finally, the eps_x parameter may be too large.
	 *
	 */
	Status optimize(const IntervalVector& init_box);

	/* =========================== Output ============================= */

	/**
	 * \brief Displays on standard output a report of the last call to optimize(...).
	 *
	 * Information provided:
	 * <ul><li> interval of the cost  [uplo,loup]
	 *     <li> the best feasible point found
	 *     <li> total running time
	 *     <li> total number of cells (~boxes) created during the exploration
	 * </ul>
	 */
	void report(bool verbose=true);

	void plot(Cell* current);

	/**
	 * \brief Get the status.
	 *
	 * \return the status of last call to optimize(...).
	 */
	Status get_status() const;

	/**
	 * \brief Get the "UB" set of the pareto front.
	 *
	 * \return the UB of the last call to optimize(...).
	 */
	map< pair <double, double>, IntervalVector >& get_UB()  { return UB; }

	std::set< point2 >& get_LB()  { return LB; }

	/**
	 * \brief Get the time spent.
	 *
	 * \return the total CPU time of last call to optimize(...)
	 */
	double get_time() const;

	/**
	 * \brief Get the number of cells.
	 *
	 * \return the number of cells generated by the last call to optimize(...).
	 */
	double get_nb_cells() const;


	int get_nb_sols() const {return nb_sols;}
	/* =========================== Settings ============================= */

	/**
	 * \brief Number of variables.
	 */
	const int n;

	/**
	 * \brief Objective functions
	 * Functions have the form: f1 - z1  and f2 - z2. Thus, in order to
	 * evaluate them we have to set z1 and z2 to [0,0].
	 */
	const Function& goal1;
	const Function& goal2;

	/**
	 * \brief Constraints
	 */
	const Array<NumConstraint>& ctrs;

	/**
	 * \brief Contractor for the extended system.
	 *
	 * The extended system:
	 * (y=f(x), g_1(x)<=0,...,g_m(x)<=0).
	 */
	Ctc& ctc;

	/**
	 * \brief Bisector.
	 *
	 * Must work on extended boxes.
	 */
	Bsc& bsc;

	/**
	 * Cell buffer.
	 */
	CellBuffer& buffer;

	/**
	 * \brief LoupFinder
	 */
	LoupFinderMOP& finder;

	/** Precision of the pareto frontier */
	double eps;

	double top_dist;

	/** Default precision: 0.01 */
	static const double default_eps;

	/**
	 * \brief Trace activation flag.
	 *
	 * The value can be fixed by the user.
	 * - 0 (by default): nothing is printed
	 * - 1:              prints every loup/uplo update.
	 * - 2:              prints also each handled node (warning: can generate very
	 *                   long trace).
	 */
	int trace;

	/**
	 * \brief Time limit.
	 *
	 * Maximum CPU time used by the strategy.
	 * This parameter allows to bound time consumption.
	 * The value can be fixed by the user.
	 */
	double timeout;


	/**
	 * \brief returns the distance from the box to the non-dominated set of points
	 */
	static double distance2(const Cell* c);


	void updateLB(Cell* c, int dist){
		if(LB.empty()) {
			//cout << "dist: " << dist << ":" << abs_eps << endl;
			if(_plot  && dist>=0) {plot(c);  getchar();}
			y2_max=y1_ub.second;
			y1_max=y2_ub.first;
		}

        //for obtaining the nadir point (y1max, y2max) used to compute the hypervolume
		if(y1_max < c->box[n].ub() &&  c->box[n+1].lb()<y2_ub.second)
			y1_max=c->box[n].ub();
		if(y2_max < c->box[n+1].ub() &&  c->box[n].lb()<y1_ub.first)
			y2_max=c->box[n+1].ub();

		if(cy_contract_var){
			double ya2 = ((c)->get<CellBS>().w_lb-c->box[n].lb())/(c)->get<CellBS>().a;
			if(trace) cout << "ya2:" << ya2 << endl;
			if(ya2 > c->box[n+1].lb() && ya2 < c->box[n+1].ub()){
				double yb1 = (c)->get<CellBS>().w_lb-((c)->get<CellBS>().a*c->box[n+1].lb());
				if(trace) cout << "yb1:" << yb1 << endl;
				if(yb1 > c->box[n].lb() && yb1 < c->box[n].ub()){
						insert_lb_segment( point2(c->box[n].lb(),ya2),
						point2(yb1 ,  c->box[n+1].lb()) );
				}else{
					double yb2=((c)->get<CellBS>().w_lb-c->box[n].ub())/(c)->get<CellBS>().a;
					insert_lb_segment( point2(c->box[n].lb(),ya2),
							point2(c->box[n].ub() ,  yb2 ) );
				}

			}else if(ya2 >= c->box[n+1].ub()){
				double ya1=(c)->get<CellBS>().w_lb-((c)->get<CellBS>().a*c->box[n+1].ub());
				double yb1 = (c)->get<CellBS>().w_lb-((c)->get<CellBS>().a*c->box[n+1].lb());
				if(trace) cout << "ya1:" << ya1 << endl;
				if(trace) cout << "yb1:" << yb1 << endl;
				if(yb1 > c->box[n].lb() && yb1 < c->box[n].ub()){

					insert_lb_segment( point2(ya1, c->box[n+1].ub()),
								point2(yb1 ,  c->box[n+1].lb() ) );
				}else{

					double yb2=((c)->get<CellBS>().w_lb-c->box[n].ub())/(c)->get<CellBS>().a;
					insert_lb_segment( point2(ya1, c->box[n+1].ub()),
								point2(c->box[n].ub() ,  yb2) );
				}


			}else
			  insert_lb_segment(point2(c->box[n].lb(),c->box[n+1].lb()),point2(c->box[n].lb(),c->box[n+1].lb()));
		}else
			insert_lb_segment(point2(c->box[n].lb(),c->box[n+1].lb()),point2(c->box[n].lb(),c->box[n+1].lb()));
	}

  //TODO: make it conservative!
	void insert_lb_segment(point2 p1, point2 p2){
      //trace=1;
		  if(trace) cout << "p1-p2: (" << p1.x.mid() << "," << p1.y.mid() << ") --> (" << p2.x.mid() << "," << p2.y.mid() << ")" << endl;
	    if(LB.size()==0){
			LB.insert(point2(CellBS::y1_init.lb(),CellBS::y2_init.ub()));
	    	LB.insert(point2(CellBS::y1_init.ub(),CellBS::y2_init.ub()));
	    	LB.insert(point2(CellBS::y1_init.ub(),CellBS::y2_init.lb()));

	    }

		point2 p1_p = point2(p1.x,CellBS::y2_init.ub());
		point2 p2_p = point2(CellBS::y1_init.ub(),p2.y);

		//point2 p1_p = point2(p1.x,1e10);
		//point2 p2_p = point2(1e10,p2.y);

		std::set< point2 > new_points;
		std::set< point2 >::iterator it=LB.upper_bound(p1);
		it--;

		point2 v1(it->x, it->y);
		it++;
		point2 v2(it->x, it->y);

		bool in = false;


		point2 s;
		if (intersect(v1, v2, p1_p,  p1, s)) {
			 if(trace) cout << "s1: (" << s.x << "," << s.y << ")" << endl;
			 if(s.x.lb()==NEG_INFINITY){
				 cout << "s1: (" << s.x << "," << s.y << ")" << endl;
				 return; exit(0);
			 }

			 if(( (s.x==p1.x && s.y==p1.y) && ((p2-p1)*(v2-v1)).lb() <= 0 )){
				new_points.insert(s);
				new_points.insert(p1);
				in = true;  if(trace) cout << "in"  << endl;
				//it--; LB.erase(v2); it++; v1 = v2; v2=*it;
			}


			else if(s.x!=p1.x || s.y!=p1.y){
				new_points.insert(s);
				new_points.insert(p1);
				in = true;  if(trace) cout << "in"  << endl;
				//it--; LB.erase(v2); //it++; v1 = v2; v2=*it;
			}
			//it++; v1 = v2; v2=*it;
		}



	    while(v1.y.mid() > p2.y.mid()){

	        if (intersect(v1,v2, p1, p2, s)){
	           if(trace) cout << "s2: (" << s.x << "," << s.y << ")" << endl;
				if(s.x.lb()==NEG_INFINITY){
					cout << "s2: (" << s.x << "," << s.y << ")" << endl;
					return; exit(0);
				}

	          in=!in;
              if(trace) cout << ((in)? "in":"out")  << endl;
	          new_points.insert(point2(s.x.lb(),s.y.lb()));
	        }

	        if ( v2.y.mid() <= p2.y.mid() && intersect(v1,v2, p2, p2_p,s)){
	           if(trace) cout << "s3: (" << s.x << "," << s.y << ")" << endl;
						if(s.x.lb()==NEG_INFINITY) exit(0);
	          in = false;
						 if(trace) cout << "out" << endl;
	          new_points.insert(point2(s.x.lb(),s.y.lb()));
	          new_points.insert(p2);
	          break;
	        }

	        if(in){ it--; LB.erase(v2); }

	        v1 = v2;
	        it++;
	        v2 = *it;
	    }

	    LB.insert(new_points.begin(), new_points.end());

	}

	bool static _plot;
	int static _nb_ub_sols;
	double static _min_ub_dist;
	static bool _cy_upper;
	static bool cy_contract_var;
	static bool _hv;
	static bool _eps_contract;


protected:

	/**
	 * The contraction using y+cy
	 */
	void cy_contract(Cell& c);

	/**
	 * \brief Contract and bound procedure for processing a box.
	 *
	 * <ul>
	 * <li> contract the cell's box w.r.t the "loup",
	 * <li> contract with the contractor ctc,
	 * <li> search for a new loup,
	 * <li> (optional) call the first order contractor
	 * </ul>
	 *
	 */
	void contract_and_bound(Cell& c, const IntervalVector& init_box);

	/**
	 * \brief The box is reduced using the set of nondominated points as shown in EJOR2016, page 13
	 */
	void dominance_peeler(IntervalVector& box);

    bool is_inner_facet(IntervalVector box, int i, Interval bound){
    	box.resize(n);
    	box[i]=bound;
    	//TODO: should be strict inner!
    	return finder.norm_sys.is_inner(box);
    }


	void discard_generalized_monotonicty_test(IntervalVector& box, const IntervalVector& initbox){
		IntervalVector grad_f1= goal1.gradient(box);
		IntervalVector grad_f2= goal2.gradient(box);

		IntervalVector new_box(box);

		//bool discard=false;

		for(int i=0;i<n;i++){
			for(int j=0;j<n;j++){
				if(grad_f1[i].lb() > 0.0 && grad_f2[j].lb()>0.0 &&
					    (i==j || (Interval(grad_f2[i].lb()) / grad_f2[j].lb() -  Interval(grad_f1[i].lb()) / grad_f1[j].lb()).lb()  > 0.0) ){

					    if(i==j) new_box[i] = box[i].lb(); //simple monotonicity test

						if( box[i].lb() != initbox[i].lb() && box[j].lb() != initbox[j].lb() ){
							if(is_inner_facet(box,i,box[i].lb()) && is_inner_facet(box,j,box[j].lb())){
								box.set_empty();
								return;
							}
						}
				}else if(grad_f1[i].ub() < 0.0 && grad_f2[j].lb()>0.0 &&
						(Interval(grad_f1[i].ub()) / grad_f1[j].lb() -  Interval(grad_f2[i].ub()) / grad_f2[j].lb()).lb()  > 0.0) {

					    if( box[i].ub() == initbox[i].ub() && box[j].lb() != initbox[j].lb() ) {
							if(is_inner_facet(box,i,box[i].ub()) && is_inner_facet(box,j,box[j].lb())){
								box.set_empty();
								return;
							}
						}

				}else if(grad_f1[i].lb() > 0.0 && grad_f2[j].ub()<0.0 &&
						(Interval(grad_f1[i].lb()) / grad_f1[j].ub() -  Interval(grad_f2[i].lb()) / grad_f2[j].ub()).lb()  > 0.0) {

					    if( box[i].lb() != initbox[i].lb() && box[j].ub() != initbox[j].ub() )
					    {
							if(is_inner_facet(box,i,box[i].lb()) && is_inner_facet(box,j,box[j].ub())){
								box.set_empty();
								return;
							}
						}

				 }else if(grad_f1[i].ub() < 0.0 && grad_f2[j].ub() < 0.0 &&
						(i==j || (Interval(grad_f2[i].ub()) / grad_f2[j].ub() -  Interval(grad_f1[i].ub()) / grad_f1[j].ub()).lb()  > 0.0) ){

						if(i==j) new_box[i] = box[i].ub(); //simple monotonicity test

						if( box[i].ub() != initbox[i].ub() && box[j].ub() != initbox[j].ub() )
						{
							if(is_inner_facet(box,i,box[i].ub()) && is_inner_facet(box,j,box[j].ub())){
								box.set_empty();
								return;
							}
						}
				}
			}
		}

		IntervalVector bb=new_box;
		bb.resize(n);

		if(finder.norm_sys.is_inner(bb))
			box=new_box;

  }

  /**
  * \brief intersect two segments and return the intersection res
  * https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
  */
  bool intersect(point2 p, point2 p2,
		point2 q,  point2 q2, point2& res){

	  	  if(trace) cout << "p-p2: (" << p.x.mid() << "," << p.y.mid() << ") --> (" << p2.x.mid() << "," << p2.y.mid() << ")" << endl;
	  	  if(trace) cout << "q-q2: (" << q.x.mid() << "," << q.y.mid() << ") --> (" << q2.x.mid() << "," << q2.y.mid() << ")" << endl;

        //orthogonal segments
	  	  if(p.x.mid()==p2.x.mid() && q.y.mid()==q2.y.mid()) {
	  		  if(q.x.mid()>p.x.mid() || q2.x.mid() < p.x.mid() ||
					q.y.mid() < p2.y.mid() || q.y.mid() > p.y.mid()) return false;
	  		  res.x=p.x;
	  		  res.y=q.y;
	  		  return true;
	  	  }

        //orthogonal segments
	  	  if(p.y.mid()==p2.y.mid() && q.x.mid()==q2.x.mid()){
	  		  if(p.x.mid() > q.x.mid() || p2.x.mid() < q.x.mid() ||
					p.y.mid() < q2.y.mid() || p.y.mid() > q.y.mid()) return false;
	  		  res.x=q.x;
	  		  res.y=p.y;
	  		  return true;
	  	  }

	  	  if( (p.x.mid()==p2.x.mid() && p.y.mid()==p2.y.mid()) ||
				 (q.x.mid()==q2.x.mid() && q.y.mid()==q2.y.mid())) return false;

			point2 r = p2-p;
			point2 s = q2-q;

			//now we find a solution for the equation p+tr = q+us,
			if( (r * s).lb() > -1e-8 && (r * s).ub() < 1e-8) {
				//segments are collinear
				if((q - p) * r == 0){
					//segments are intersecting
					if(p2.y.mid() > q2.y.mid() || (p2.y.mid() == q2.y.mid() && p2.x.mid()<q2.x.mid())) res=p2;
					else res=q2;
					return true;
				}else //segments have no intersection
				  return false;

			}

			Interval t = ((q - p) * s) / (r * s);
			Interval u = ((p - q) * r) / (s * r);


			if (t.ub()>=0 && t.lb() <=1  && u.ub()>=0 && u.lb() <=1){
				res = p + point2(t*r.x,t*r.y);
				//cout << "res::(" << res.x << "," << res.y << ")"  << endl;
				return true;
			}

			return false;
		}

	/**
	 * \brief Main procedure for updating the loup.
	 */
	bool update_UB(const IntervalVector& box, int n);

	Interval compute_lb_hypervolume(){

    Interval volume(0.0);
    point2 lb1 = *LB.begin();

    std::set< point2 >::iterator lb2=LB.begin();

    cout << y1_max << "," << y2_max << endl;

		for(lb2++;lb2!=LB.end();lb2++){
          if( (lb1.y.mid() <= y2_max) && ( lb2->x.mid() <= y1_max ) )
        	  volume += (( y2_max - lb1.y ) + (lb1.y - lb2->y)/2.0) * ( lb2->x - lb1.x );

          else if( lb2->x.mid() > y1_max ){
        	  volume += ( y2_max - lb1.y )  * ( y1_max - lb1.x );
        	  break;
          }

          lb1=*lb2;
		}

		return volume;
	}

	Interval compute_ub_hypervolume(){

		//cout << y1_max << ";" <<y2_max << endl;

        Interval volume(0.0);
        pair <double, double> ub1 = UB.begin()->first;
        map< pair <double, double>, IntervalVector >::iterator _ub2= UB.begin();

		for(;_ub2!=UB.end();_ub2++){

			pair <double, double> ub2=_ub2->first;
			pair <double, double> ubx=make_pair(ub2.first, ub1.second);

          if( (ub1.second <= y2_max) && ( ub2.first <= y1_max ) )
        	  volume += (Interval(y2_max) - ub1.second ) * ( Interval(ubx.first) - ub1.first );

          else if(ub1.second <= y2_max && ub2.first > y1_max ){
        	  volume += ( Interval(y2_max) - ub1.second )  * ( Interval(y1_max) - ub1.first );
        	  break;
          }

          ub1=ub2;
          //cout << volume << endl;
          //cout << endl;
		}

		return volume;
	}



private:

	/**
	 * \brief Evaluate the goal in the point x
	 */
	Interval eval_goal(const Function& goal, IntervalVector& x);

	/**
	 * min feasible value found for the objectives
	 */
    pair <double, double> y1_ub, y2_ub;

    /**
     * the max possible value for the objectives s.t. the other objective is minimized
     */
    double y1_max, y2_max;

	/** Currently entailed constraints */
	//EntailedCtr* entailed;

	//!! warning: sys.box should be properly set before call to constructor !!
	//CtcKhunTucker kkt;

	/* Remember return status of the last optimization. */
	Status status;

  /** The cells in the buffer for plotting
	 * the set should be updated each time the real buffer is popped
	 * and pushed.
	 */
	set< Cell* > buffer_cells;

	//TODO: arreglar esto (static)
	/** The current upper bounds (f1(x), f2(x)) of the pareto front associated
	 * to its corresponding  point x
	 */
	static map< pair <double, double>, IntervalVector > UB;

	map< pair <double, double>, IntervalVector, sorty > UBy;

	/**
	 * A set of points denoting the segments related to the lowerbound of the
	 * pareto front.
	 */
	std::set< point2 > LB;

	int nb_sols;


	/** True if loup has changed in the last call to handle_cell(..) */
	//bool loup_changed;

	/* CPU running time of the current optimization. */
	double time;

	/** Number of cells pushed into the heap (which passed through the contractors) */
	int nb_cells;
};

inline OptimizerMOP::Status OptimizerMOP::get_status() const { return status; }

inline double OptimizerMOP::get_time() const { return time; }

inline double OptimizerMOP::get_nb_cells() const { return nb_cells; }




} // end namespace ibex

#endif // __IBEX_OPTIMIZER_H__
