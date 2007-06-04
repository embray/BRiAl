/*
 *  nf.cc
 *  PolyBoRi
 *
 *  Created by Michael Brickenstein on 25.04.06.
 *  Copyright 2006 __MyCompanyName__. All rights reserved.
 *
 */

#include "nf.h"
#include "lexbuckets.h"
#include <LexOrder.h>
#include <iostream>
#include <COrderedIter.h>
#ifdef HAVE_NTL
#include <NTL/GF2.h>
#include <NTL/mat_GF2.h>
NTL_CLIENT
#endif
#ifdef HAVE_M4RI

#include "../../M4RI/m4ri.h"

#endif
using std::cout;
using std::endl;

BEGIN_NAMESPACE_PBORIGB
Polynomial add_up_monomials(const std::vector<Monomial>& res_vec);
static bool irreducible_lead(Monomial lm, const GroebnerStrategy& strat){

  return (!(strat.minimalLeadingTerms.hasTermOfVariables(lm)));//
  //        strat.minimalLeadingTerms.intersect(lm.divisors()).emptiness();
}
Polynomial nf1(GroebnerStrategy& strat, Polynomial p){
  //parameter by value, so I can modify it
  int index;
  while((index=select1(strat,p))>=0){
    p=spoly(p,strat.generators[index].p);
    
  }
  return p;
}
Polynomial nf2(GroebnerStrategy& strat, Polynomial p){
  //parameter by value, so I can modify it
  int index;
  while((index=select1(strat,p))>=0){
    assert(index<strat.generators.size());
    Polynomial* g=&strat.generators[index].p;
    if (g->nNodes()==1){
      idx_type v=*(g->navigation());
      if (g->length()==1)
      {
        p=Polynomial(p.diagram().subset0(v));
      } else {
        Polynomial p2=Polynomial(p.diagram().subset1(v));
        p=Polynomial(p.diagram().subset0(v))+p2;
      }
    } else {
      
      if (strat.generators[index].length==1){
        assert(strat.generators[index].p.length()==1);
        assert(strat.generators[index].lm==strat.generators[index].p.lead());
        //if (p!=strat.generators[index].lm)
          p=reduce_by_monom(p,strat.generators[index].lm);
        //else
        //  p=Polynomial(0);
      } else{
        assert(!(p.isZero()));
        assert(p.reducibleBy(*g));
        assert(!(g->isZero()));
        if (strat.generators[index].length==2)
          p=reduce_by_binom(p,strat.generators[index].p);
        else{
          if (strat.generators[index].deg==1){
            //implies lmDeg==1, ecart=0
            //cout<<"REDUCE_COMPLETE\n";
            assert(strat.generators[index].ecart()==0);
            assert(strat.generators[index].lmDeg==1);
            wlen_type dummy;
            p=reduce_complete(p,strat.generators[index],dummy);
          }
          else{
            p=spoly(p,*g);
          }
          
        }
      }
    }
  }
  return p;
}



Polynomial nf2_short(GroebnerStrategy& strat, Polynomial p){
  //parameter by value, so I can modify it
  int index;
  while((index=select_short(strat,p))>=0){
    assert(index<strat.generators.size());
    Polynomial* g=&strat.generators[index].p;
    if (g->nNodes()==1){
      idx_type v=*(g->navigation());
      if (g->length()==1)
      {
        p=Polynomial(p.diagram().subset0(v));
      } else {
        Polynomial p2=Polynomial(p.diagram().subset1(v));
        p=Polynomial(p.diagram().subset0(v))+p2;
      }
    } else {
      
      if (strat.generators[index].length==1){
        assert(strat.generators[index].p.length()==1);
        assert(strat.generators[index].lm==strat.generators[index].p.lead());
        //if (p!=strat.generators[index].lm)
        p=reduce_by_monom(p,strat.generators[index].lm);
        //else
        //  p=Polynomial(0);
      } else{
        assert(!(p.isZero()));
        assert(p.reducibleBy(*g));
        assert(!(g->isZero()));
        if (strat.generators[index].length==2)
          p=reduce_by_binom(p,strat.generators[index].p);
        else{
          if (strat.generators[index].deg==1){
            //implies lmDeg==1, ecart=0
            //cout<<"REDUCE_COMPLETE\n";
            assert(strat.generators[index].ecart()==0);
            assert(strat.generators[index].lmDeg==1);
            wlen_type dummy;
            p=reduce_complete(p,strat.generators[index], dummy);
          }
          else{
            p=spoly(p,*g);
          }
          
        }
      }
    }
  }
  return p;
}



Polynomial nf3(const GroebnerStrategy& strat, Polynomial p, Monomial rest_lead){
  int index;
  while((index=select1(strat,rest_lead))>=0){
    assert(index<strat.generators.size());
  
    const Polynomial* g=&strat.generators[index].p;
    
    if //((strat.generators[index].deg==1)&&(lm!=strat.generators[index].lm)){
    (((strat.optBrutalReductions) && (rest_lead!=strat.generators[index].lm))||((strat.generators[index].length<4) &&(strat.generators[index].ecart()==0) 
    && (rest_lead!=strat.generators[index].lm))){
      wlen_type dummy;
      p=reduce_complete(p,strat.generators[index], dummy);

    } else{
      //p=spoly(p,*g);
      Exponent exp=rest_lead.exp();
      p+=(exp-strat.generators[index].lmExp)*(*g);
    }
    if (p.isZero())
        return p;
    else
        rest_lead=p.lead();
  }
  return p;
}


Polynomial nf3_lexbuckets(const GroebnerStrategy& strat, Polynomial p, Monomial rest_lead){
  int index;
  LexBucket bucket(p);
  //cout<<"huhu";
  while((index=select1(strat,rest_lead))>=0){
    assert(index<strat.generators.size());
  
    const Polynomial* g=&strat.generators[index].p;
    
    if //((strat.generators[index].deg==1)&&(lm!=strat.generators[index].lm)){
    (((strat.optBrutalReductions) && (rest_lead!=strat.generators[index].lm))||((strat.generators[index].length<4) &&(strat.generators[index].ecart()==0) 
    && (rest_lead!=strat.generators[index].lm))){
      //wlen_type dummy;
      Polynomial front=bucket.getFront();
      front/=strat.generators[index].lmExp;
      front*=*g;
      bucket+=front;
      
      //p=reduce_complete(p,strat.generators[index], dummy);

    } else{
      //p=spoly(p,*g);
      Exponent exp=rest_lead.exp();
      //Monomial lm=strat.generators[index].lm
      bucket+=(exp-strat.generators[index].lmExp)*(*g);
    }
    if (bucket.isZero())
        return 0;
    else
        rest_lead=(Monomial) bucket.leadExp();
  }
  return bucket.value();
}

Polynomial nf3_no_deg_growth(const GroebnerStrategy& strat, Polynomial p, Monomial rest_lead){
  int index;
  while((index=select_no_deg_growth(strat,rest_lead))>=0){
    assert(index<strat.generators.size());
  
    const Polynomial* g=&strat.generators[index].p;
    
    if //((strat.generators[index].deg==1)&&(lm!=strat.generators[index].lm)){
    (((strat.optBrutalReductions) && (rest_lead!=strat.generators[index].lm))|| 
			((strat.generators[index].length<4) &&(strat.generators[index].ecart()==0) && 
			(rest_lead!=strat.generators[index].lm))){
			wlen_type dummy;
      p=reduce_complete(p,strat.generators[index],dummy);

    } else{
      //p=spoly(p,*g);
      Exponent exp=rest_lead.exp();
      p+=(exp-strat.generators[index].lmExp)*(*g);
    }
    if (p.isZero())
        return p;
    else
        rest_lead=p.lead();
  }
  return p;
}
Polynomial nf3_degree_order(const GroebnerStrategy& strat, Polynomial p, Monomial lead){
    int index;
    int deg=p.deg();
    //Monomial lead=p.boundedLead(deg);
    Exponent exp=lead.exp();
    while((index=select1(strat,lead))>=0){
    assert(index<strat.generators.size());
  
    const Polynomial* g=&strat.generators[index].p;
    
    if //((strat.generators[index].deg==1)&&(lm!=strat.generators[index].lm)){
    (((strat.optBrutalReductions) && (lead!=strat.generators[index].lm))||
			((strat.generators[index].length<4) &&(strat.generators[index].ecart()==0)
 			&& (lead!=strat.generators[index].lm)))

{     wlen_type dummy;
      p=reduce_complete(p,strat.generators[index],dummy);

    } else{
      p+=(exp-strat.generators[index].lmExp)*(*g);
      //p=spoly(p,*g);
    }
    if (!(p.isZero())){
        lead=p.boundedLead(deg);
        exp=lead.exp();
        deg=exp.deg();
    } else return p;
  }
  return p;
}
Polynomial nf3_db(GroebnerStrategy& strat, Polynomial p, int deg_bound){
  int index;
  while((index=select1(strat,p))>=0){
    assert(index<strat.generators.size());
    if((strat.generators[index].ecart()>0) && strat.generators[index].ecart()+p.lmDeg()-strat.generators[index].lm.deg()>deg_bound)
        return p;
    Polynomial* g=&strat.generators[index].p;
    
    if //((strat.generators[index].deg==1)&&(lm!=strat.generators[index].lm)){
    ((strat.generators[index].length<4) &&(strat.generators[index].ecart()==0) && (p.lead()!=strat.generators[index].lm)){
      wlen_type dummy;
      p=reduce_complete(p,strat.generators[index],dummy);

    } else{
      p=spoly(p,*g);
    }
  }
  return p;
}
Polynomial nf3_short(const GroebnerStrategy& strat, Polynomial p){
  int index;
  while((index=select_short(strat,p))>=0){
    assert(index<strat.generators.size());
    const Polynomial* g=&strat.generators[index].p;
    
    if //((strat.generators[index].deg==1)&&(lm!=strat.generators[index].lm)){
    ((strat.generators[index].length<4) &&(strat.generators[index].ecart()==0) && (p.lead()!=strat.generators[index].lm)){
      wlen_type dummy;
      p=reduce_complete(p,strat.generators[index].p,dummy);
      
    } else{
      p=spoly(p,*g);
    }
  }
  return p;
}

const int FARE_WORSE=10;
Polynomial nf_delaying(GroebnerStrategy& strat, Polynomial p){
  //parameter by value, so I can modify it
  wlen_type initial=p.eliminationLength();
  int index;
  bool first=true;
  while((index=select1(strat,p))>=0){
    Polynomial* g=&strat.generators[index].p;
    if (g->nNodes()==1){
      idx_type v=*(g->navigation());
      if (g->length()==1)
      {
        p=Polynomial(p.diagram().subset0(v));
      } else {
        Polynomial p2=Polynomial(p.diagram().subset1(v));
        p=Polynomial(p.diagram().subset0(v))+p2;
      }
    } else {
      if ((first==true) ||(strat.generators[index].weightedLength<= FARE_WORSE*initial))
        p=spoly(p,*g);
      else {
        strat.addGeneratorDelayed(p);
        strat.log("Delay");
        return Polynomial(false);
      }
    }
    first=false;
  }
  return p;
}

static Polynomial exchange(GroebnerStrategy& strat , int i, const Polynomial & p){
  assert(p.lead()==strat.generators[i].lm);
  PolyEntry e(p);
  e.vPairCalculated=strat.generators[i].vPairCalculated;
  Polynomial res=spoly(strat.generators[i].p,p);
  strat.generators[i]=e;
  return res;
}

static Polynomial exchange_with_promise(GroebnerStrategy& strat , int i, const Polynomial & p){
  assert(p.lead()==strat.generators[i].lm);
  //PolyEntry e(p);
  //e.vPairCalculated=strat.generators[i].vPairCalculated;
  bool minimal=strat.generators[i].minimal;
  Polynomial res=strat.generators[i].p;
  strat.generators[i].p=p;
  strat.generators[i].recomputeInformation();
  //strat.generators[i].minimal=false;
  
  if ((strat.generators[i].minimal)&&(strat.generators[i].length==2))
  //if ((strat.generators[i].length==2))
    strat.addNonTrivialImplicationsDelayed(strat.generators[i]);
  if (strat.generators[i].lmDeg==1)
    strat.propagate(strat.generators[i]);
  return res;
}

Polynomial nf_delaying_exchanging(GroebnerStrategy& strat, Polynomial p){
  //parameter by value, so I can modify it
  wlen_type initial=p.eliminationLength();
  int index;
  bool first=true;
  while((index=select1(strat,p))>=0){
    Polynomial* g=&strat.generators[index].p;
    if (g->nNodes()==1){
      idx_type v=*(g->navigation());
      if (g->length()==1)
      {
        p=Polynomial(p.diagram().subset0(v));
      } else {
        Polynomial p2=Polynomial(p.diagram().subset1(v));
        p=Polynomial(p.diagram().subset0(v))+p2;
      }
    } else {
      if ((p.lead()==strat.generators[index].lm) && (p.eliminationLength()<strat.generators[index].weightedLength)){
        p=exchange(strat,index,p);
        strat.log("Exchange");
      } else{
        if ((first==true) ||(strat.generators[index].weightedLength<= FARE_WORSE*initial))
          p=spoly(p,*g);
        else {
          strat.addGeneratorDelayed(p);
          strat.log("Delay");
          return Polynomial(false);
        }
      }
    }
    first=false;
  }
  return p;
}


template <> void SlimgbReduction<SLIMGB_SIMPLEST>::reduce(){
  while (!(to_reduce.empty())){
    //cout<<"looping"<<endl;
    std::vector<Polynomial> curr;
    curr.push_back(to_reduce.top());
    to_reduce.pop();
    //cout<<curr[0];
    Monomial lm=curr[0].lead();
    while ((!(to_reduce.empty())) && (to_reduce.top().lead()==lm)){
      curr.push_back(to_reduce.top());
      to_reduce.pop();
      //cout<<"same"<<endl;
      //cout.flush();
    }
    //cout<<lm;
    //cout.flush();
    int index=select1(*strat,lm);
    if (index>=0){
      Polynomial p_high=(lm/strat->generators[index].lm)*strat->generators[index].p;
      int i,s;
      s=curr.size();
      assert(p_high.lead()==lm);
      for(i=0;i<s;i++){
        curr[i]+=p_high;
        if (!(curr[i].isZero())){
          to_reduce.push(curr[i]);
        }
      }
    } else {
      //simly take the first, not so clever
      Polynomial reductor=curr.back();
      curr.pop_back();
      int i,s;
      s=curr.size();
      if (s>0){
        for(i=0;i<s;i++){
          curr[i]+=reductor;
          if (!(curr[i].isZero())){
            assert(curr[i].lead()<lm);
            to_reduce.push(curr[i]);
          }
          
        }
        result.push_back(reductor);
      } else{
        assert(s==0);
        result.push_back(curr[0]);
      }
    }
  
  }
  
}



class PolynomialSugar{
public:
  PolynomialSugar(const Polynomial& p){
    this->p=p;
    sugar=p.deg();
    this->lm=p.boundedLead(sugar);
    length=p.length();
    this->exp=lm.exp();
    assert(lm==p.lead());
    assert(exp==p.leadExp());
  }
  PolynomialSugar(const Polynomial& p, int sugar, int length){
    this->p=p;
    assert(length>=0);
    
    //sugar=p.deg();
    this->sugar=sugar;
    this->length=length;
    assert(sugar>=p.deg());
    assert(length>=p.length());
    this->lm=p.boundedLead(sugar);
    this->exp=lm.exp();
    assert(lm==p.lead());
    assert(exp==p.leadExp());
  }
  const BooleMonomial& lead() const{
    return this->lm;
  }
  const Exponent& leadExp() const{
    return this->exp;
  }
  deg_type getSugar() const{
    return sugar;
  }
  wlen_type getLengthEstimation() const {
    return length;
  }
  bool isZero() const{
    return p.isZero();
  }
  void add(const Polynomial p2, deg_type sugar2, wlen_type length){
    assert(p2.leadExp()==exp);
    assert(length>=0);
    assert(length>=p2.length());
    this->p=p+p2;
    this->sugar=std::max(sugar2,this->sugar);
    this->lm=this->p.boundedLead(sugar);
    this->exp=this->lm.exp();
    this->length+=length;
    this->length-=2;
    if (BoolePolyRing::isTotalDegreeOrder()) this->sugar=this->lm.deg();
    assert(lm==p.lead());
    assert(exp==p.leadExp());
  }
  void adjustSugar(){
    sugar=p.deg();
  }
  bool isOne(){
    return p.isOne();
  }
  Polynomial value() const{
    return p;
  }
  wlen_type eliminationLength() const{
    ///@TODO optimize that using length optimization
    wlen_type res=1;
    if (isZero()) return 0;
    res=res+(sugar-exp.deg()+1)*(length-1);
    assert(res>=p.eliminationLengthWithDegBound(sugar));
    return res;
    //return p.eliminationLengthWithDegBound(sugar);
  }
  void adjustLm(){
    this->lm=this->p.lead();
    exp=lm.exp();
    assert(lm==p.lead());
    assert(exp==p.leadExp());
  }
protected:
  Monomial lm;
  wlen_type length;
  deg_type sugar;
  Polynomial p;
  Exponent exp;
};

class LMLessComparePS{
public:
  bool operator() (const PolynomialSugar& p1, const PolynomialSugar& p2){
    return p1.leadExp()<p2.leadExp();
  }
};


static void step_S(std::vector<PolynomialSugar>& curr, std::vector<Polynomial>& result, const BooleMonomial& lm, int index, GroebnerStrategy& strat){
  
  int s=curr.size();
  
  
  
  if ((strat.generators[index].length>2)||(lm==strat.generators[index].lm)){
    
    
    
    if //((strat.generators[index].deg==1)&&(lm!=strat.generators[index].lm)){
	  (((strat.optBrutalReductions) && (lm!=strat.generators[index].lm)) ||
    ((strat.generators[index].length<4) &&(strat.generators[index].ecart()==0) 
		&& (lm!=strat.generators[index].lm))){
      //implies lmDeg==1, ecart=0
      //cout<<"REDUCE_COMPLETE\n";
      //assert(strat.generators[index].ecart()==0);
      //assert(strat.generators[index].lmDeg==1);
      //p=reduce_complete(p,strat.generators[index].p);
      
      for(int i=0;i<s;i++){
        
        //curr[i].add(p_high, deg_high);
        Polynomial to_red=curr[i].value();
        wlen_type new_len=curr[i].getLengthEstimation();
        to_red=reduce_complete(to_red,strat.generators[index],new_len);
        if (BoolePolyRing::isTotalDegreeOrder())
            curr[i]=PolynomialSugar(to_red,curr[i].getSugar(),new_len);
        else
            curr[i]=PolynomialSugar(to_red);
      }
      
      
    }
    else{
      Polynomial p_high=(lm/strat.generators[index].lm)*strat.generators[index].p;
      
      wlen_type len_high=strat.generators[index].length;
      if (lm!=strat.generators[index].lm) len_high=p_high.length();
      if ((strat.reduceByTailReduced) && (p_high!=strat.generators[index].p)){
          p_high=red_tail(strat, p_high);
          len_high=p_high.length();
        }
  deg_type deg_high=strat.generators[index].ecart()+lm.deg();
  assert(p_high.lead()==lm);
      for(int i=0;i<s;i++){
        
        curr[i].add(p_high, deg_high, len_high);
      }
    }
  } else {
    assert(strat.generators[index].length<=2);
    if (strat.generators[index].length==2){
      assert(strat.generators[index].p.length()==2);
      for(int i=0;i<s;i++){
        
        //curr[i].add(p_high, deg_high);
        Polynomial to_red=curr[i].value();
        wlen_type new_len=curr[i].getLengthEstimation();
        to_red=reduce_complete(to_red,strat.generators[index],new_len);
        //curr[i]=PolynomialSugar(to_red);
        if (BoolePolyRing::isTotalDegreeOrder())
            curr[i]=PolynomialSugar(to_red,curr[i].getSugar(), new_len);
        else
            curr[i]=PolynomialSugar(to_red,to_red.deg(),new_len);
      }
    } else {
      ///@todo: check for sugar garanties
      assert(strat.generators[index].length==1);
      assert(strat.generators[index].p.length()==1);
    
      for(int i=0;i<s;i++){
        Polynomial to_red=curr[i].value();
        wlen_type new_len=curr[i].getLengthEstimation();
        to_red=reduce_complete(to_red,strat.generators[index],new_len);//BooleSet(to_red).diff(strat.generators[index].lm.multiples(to_red.usedVariables()));
        //curr[i]=PolynomialSugar(to_red);
        if (BoolePolyRing::isTotalDegreeOrder())
            curr[i]=PolynomialSugar(to_red,curr[i].getSugar(),new_len);
        else
            curr[i]=PolynomialSugar(to_red,to_red.deg(),new_len);
      }
      
    }
  }
}

static void step_S_T(std::vector<PolynomialSugar>& curr, std::vector<Polynomial>& result,  const BooleMonomial& lm, int index, GroebnerStrategy& strat){
  int s=curr.size();
  
  int found;
  wlen_type pivot_el;
  found=0;
  pivot_el=//curr[0].value().nNodes();
    curr[0].eliminationLength();
  
  for (int i=1;i<s;i++){
    wlen_type comp=//curr[i].value().nNodes();
      curr[0].eliminationLength();
    if (comp<pivot_el){
      found=i;
      pivot_el=comp;
    }
  }
  
  Polynomial pivot;
  if (pivot_el<strat.generators[index].weightedLength){
    
    pivot_el=curr[found].eliminationLength();
    if (pivot_el<strat.generators[index].weightedLength){
        if (strat.optRedTail)
            curr[found]=PolynomialSugar(red_tail(strat,curr[found].value()));
        pivot_el=curr[found].eliminationLength();
    }
  }
  /*if (pivot_el<strat.generators[index].weightedLength){
      pivot=redTail(strat,curr[found].value());
      pivot_el=pivot.eliminationLength();
      curr[found]=pivot;
      }
    */  
  if (pivot_el<strat.generators[index].weightedLength){
    wlen_type pivot_len=curr[found].value().length();
    for(int i=0;i<s;i++){
      if(i==found) continue;
      curr[i].add(curr[found].value(), curr[found].getSugar(),pivot_len);
      ///@todo different prototpye
    }
    #if 1
    if ((pivot.deg()<=strat.generators[index].deg) &&(lm.deg()==strat.generators[index].lmDeg)){
      assert(lm==strat.generators[index].lm);
      assert(curr[found].getSugar()>=curr[found].value().deg());
      assert(curr[found].value().lead()==lm);
      wlen_type old_length=strat.generators[index].length;
      deg_type old_deg=strat.generators[index].deg;
      curr[found]=PolynomialSugar(exchange_with_promise(strat, index, curr[found].value()),old_deg,old_length);
      strat.log("Exchange");
    }
    #endif
    
    deg_type deg_high=strat.generators[index].ecart()+lm.deg();
    curr[found].add((lm/strat.generators[index].lm)*strat.generators[index].p, deg_high,strat.generators[index].length);
    //assert(!(curr[found].value().isZero()));
  } else 
    step_S(curr,result,lm, index,strat);
  
}


static void step_T_simple(std::vector<PolynomialSugar>& curr, std::vector<Polynomial>& result,  const BooleMonomial& lm,GroebnerStrategy& strat){
  int s=curr.size();
  Polynomial reductor;
  int found;
  wlen_type pivot_el;
  found=0;
  pivot_el=curr[0].eliminationLength();
  int i;
  for (i=1;i<s;i++){
    wlen_type comp=curr[i].eliminationLength();
    if (comp<pivot_el){
      found=i;
      pivot_el=comp;
    }
  }
  reductor=curr[found].value();
  assert(reductor.lead()==lm);
  wlen_type length=reductor.length();//curr[found].getLengthEstimation();
  curr.erase(curr.begin()+found);
  
  s=s-1;
  
  //will calculate elimLength later so does not increase assymptotic complexity
  deg_type deg_high=reductor.deg();
  for(i=0;i<s;i++){
    //if (i==found) continue;
    assert(curr[i].lead()==lm);
    assert(curr[i].lead()==curr[i].value().lead());
    curr[i].add(reductor, deg_high,length);
    if (!(curr[i].isZero())){
      //if (!(curr[i].lead()<lm)){
    //    cout<<curr[i].lead()<<endl<<lm<<endl;
     //}
      assert(curr[i].lead()<lm);
    }
    
  }
  result.push_back(reductor);
  

}


class PSCompareByEl{
public:
  bool operator() (const PolynomialSugar& p1, const PolynomialSugar& p2){
    return ((p1.getSugar()<p2.getSugar()) ||((p1.getSugar()<=p2.getSugar()) && (p1.eliminationLength()<p2.eliminationLength())));
  }
};

int sum_size(const MonomialSet& s1, const MonomialSet& s2){
  MonomialSet m1=s1;
  MonomialSet m2=s2;
  Monomial lm=Polynomial(m1).lead();
  int d=lm.deg()/2;
  int i;
  Monomial::const_iterator iter=lm.begin();
  for(i=0;i<d;i++){
    assert(iter!=lm.end());
    m1=m1.subset1(*iter);
    m2=m2.subset1(*iter);
    iter++;
    
  }
  return m1.length()+m2.length()-2*m1.intersect(m2).length();
}


static void step_T_complex(std::vector<PolynomialSugar>& curr, std::vector<Polynomial>& result,  const BooleMonomial& lm,GroebnerStrategy& strat){
  std::sort(curr.begin(), curr.end(), PSCompareByEl());
  const int max_cans=5;
  int s=curr.size();
  Polynomial reductor;
  int found;
  wlen_type pivot_el;
  
  pivot_el=curr[0].eliminationLength();
  
  int i,j;
  for (i=s-1;i>0;i--){
    int found=0;
    MonomialSet as_set(curr[i].value());
    int c_size=sum_size(as_set, MonomialSet(curr[0].value()));
    for (j=1;j<std::min(i,max_cans);j++){ 
      int size2=sum_size(as_set, MonomialSet(curr[j].value()));
      if (size2<c_size){
        found=j;
        c_size=size2;
      }
    }
    curr[i].add(curr[found].value(), curr[found].getSugar(), curr[found].getLengthEstimation());
  }
  reductor=curr[0].value();
  curr.erase(curr.begin());
  result.push_back(reductor);
  
  
}

static void step_T(std::vector<PolynomialSugar>& curr, std::vector<Polynomial>& result,  const BooleMonomial& lm,GroebnerStrategy& strat){
  int s=curr.size();

  if (s>2) return step_T_complex(curr,result, lm, strat);
  else
    step_T_complex(curr,result, lm, strat);


  
  
}


std::vector<Polynomial> parallel_reduce(std::vector<Polynomial> inp, GroebnerStrategy& strat, int average_steps, double delay_f){

  
  std::vector<Polynomial> result;
  int i,s;
  s=inp.size();
  int max_steps=average_steps*s;
  int steps=0;
  std::priority_queue<PolynomialSugar, std::vector<PolynomialSugar>, LMLessComparePS> to_reduce;
  deg_type max_sugar=0;
  unsigned int max_length=0;
  unsigned int max_nodes=0;
  for(i=0;i<s;i++){
    if (inp[i].isOne()){
      result.push_back(inp[i]);
      return result;
    }
		if (inp[i].isZero()) continue;
    PolynomialSugar to_push=PolynomialSugar(inp[i]);
    //max_length=std::max(max_length,inp[i].length());
    max_nodes=std::max(max_nodes,inp[i].length());
    max_sugar=std::max(max_sugar,to_push.getSugar());
    
    to_reduce.push(to_push);
  }
  
  while (!(to_reduce.empty())){

    std::vector<PolynomialSugar> curr;
    curr.push_back(to_reduce.top());
    to_reduce.pop();

    Monomial lm=curr[0].lead();
    while ((!(to_reduce.empty())) && (to_reduce.top().lead()==lm)){
      curr.push_back(to_reduce.top());
      to_reduce.pop();

    }
 
    int index=select1(strat,lm);
    if (index>=0){
      steps=steps+curr.size();
      if ((strat.optExchange) && (curr.size()>1)){
        if (strat.generators[index].lmDeg==lm.deg())
          step_S_T(curr,result,lm, index,strat);
        else
          step_S(curr,result,lm, index,strat);
      } else{
        step_S(curr,result,lm, index,strat);
      }
    } else {
      //simly take the first, not so clever
     
      int i,s;
      s=curr.size();
      if (s>1){
        steps+=curr.size()-1;
        step_T_simple(curr,result,lm,strat);
      } else{
        assert(s==1);
				if (!(curr[0].isZero()))
        	result.push_back(curr[0].value());
        curr.clear();
      }
      
    }
    
    //after each loop push back
    s=curr.size();
    for(i=0;i<s;i++){
      if (!(curr[i].isZero())){
        if (((!strat.optLazy) ||((curr[i].getSugar()<=max_sugar)
        /*&&(curr[i].value().nNodes()<=delay_f*max_nodes)*/))||(curr[i].isOne())){
          if (curr[i].isOne()){
            result.clear();
            result.push_back(curr[i].value());
            return result;
          }
          to_reduce.push(curr[i]);
        } else {
          strat.log("Delaying");
          cout.flush();
          strat.addGeneratorDelayed(curr[i].value());
        }
      }
    }
    curr.clear();
    if ((strat.optStepBounded) &&(steps>max_steps)){
        strat.log("Too many steps\n");
        while (!(to_reduce.empty())){
            assert(!(to_reduce.top().isZero()));
            Monomial lm=to_reduce.top().lead();
            if (select1(strat,lm)>=0){
                while((!(to_reduce.empty()))&&(to_reduce.top().lead()==lm)){
                    strat.addGeneratorDelayed(to_reduce.top().value());
                    to_reduce.pop();
                } 
            }else {
                std::vector<PolynomialSugar> this_lm;
                while((!(to_reduce.empty()))&&(to_reduce.top().lead()==lm)){
                    this_lm.push_back(to_reduce.top());
                    to_reduce.pop();
                }
                std::vector<PolynomialSugar>::iterator for_basis=std::min_element(this_lm.begin(),this_lm.end(),PSCompareByEl());
                int i;
                for(i=0;i<this_lm.size();i++){
                    if (this_lm[i].value()!=for_basis->value()){
                        strat.addGeneratorDelayed(this_lm[i].value());
                    }
                }
                result.push_back((*for_basis).value());
            }
            
            
            
 
        }
        return result;
    }
  }
  return result;
  
}
typedef LessWeightedLengthInStratModified StratComparerForSelect;
static int select_short(const GroebnerStrategy& strat, const Polynomial& p){
  MonomialSet ms=strat.leadingTerms.intersect(p.lmDivisors());
  //Polynomial workaround =Polynomial(ms);
  
  if (ms.emptiness())
    return -1;
  else {
    
    //Monomial min=*(std::min_element(ms.begin(),ms.end(), LessWeightedLengthInStrat(strat)));
    Monomial min=*(std::min_element(ms.begin(),ms.end(), LessWeightedLengthInStrat(strat)));
    
    int res=strat.lm2Index.find(min)->second;
    if ((strat.generators[res].weightedLength<=2)/*||(strat.generators[res].ecart()==0)*/) return res;
    else return -1;
  }
  
}
static int select_short(const GroebnerStrategy& strat, const Monomial& m){
  MonomialSet ms=strat.leadingTerms.intersect(m.divisors());
  if (ms.emptiness())
    return -1;
  else {
    //Monomial min=*(std::min_element(ms.begin(),ms.end(), LessWeightedLengthInStrat(strat)));
    Monomial min=*(std::min_element(ms.begin(),ms.end(), LessWeightedLengthInStrat(strat)));
    int res=strat.lm2Index.find(min)->second;
    if ((strat.generators[res].weightedLength<=2)/*||(strat.generators[res].ecart()==0)*/) return res;
    else return -1;

  }
}



int select1(const GroebnerStrategy& strat, const Polynomial& p){
  MonomialSet ms=strat.leadingTerms.divisorsOf(p.lead());//strat.leadingTerms.intersect(p.lmDivisors());
  //Polynomial workaround =Polynomial(ms);
  
  if (ms.emptiness())
    return -1;
  else {
#ifdef LEX_LEAD_RED_STRAT
    if (BoolePolyRing::isLexicographical()){
      Exponent min=*(ms.expBegin());
      return strat.exp2Index.find(min)->second;
    }
#endif
    //Monomial min=*(std::min_element(ms.begin(),ms.end(), LessWeightedLengthInStrat(strat)));
    Exponent min=*(std::min_element(ms.expBegin(),ms.expEnd(), StratComparerForSelect(strat)));

    return strat.exp2Index.find(min)->second;
     
  }
  
}
int select1(const GroebnerStrategy& strat, const Monomial& m){
  MonomialSet ms=strat.leadingTerms.divisorsOf(m);
  if (ms.emptiness())
    return -1;
  else {
    //Monomial min=*(std::min_element(ms.begin(),ms.end(), LessWeightedLengthInStrat(strat)));
    Exponent min=*(std::min_element(ms.expBegin(),ms.expEnd(), StratComparerForSelect(strat)));
    return strat.exp2Index.find(min)->second;
  }
}
class IsEcart0Predicate{
public:
IsEcart0Predicate(const GroebnerStrategy& strat){
    this->strat=&strat;
}
bool operator() (const Exponent& e){
    return strat->generators[strat->exp2Index.find(e)->second].ecart()==0;
}


private:
    const GroebnerStrategy* strat;
};



class LexHelper{
    public:
    static bool irreducible_lead(const Monomial& m, const GroebnerStrategy& strat){
        if (strat.optRedTailDegGrowth) return PBORINAME::groebner::irreducible_lead(m,strat);
        else{
            BooleSet ms=strat.leadingTerms.intersect(m.divisors());
            if (ms.emptiness())
                return true;
            else {
                return std::find_if(ms.expBegin(),ms.expEnd(),IsEcart0Predicate(strat))==ms.expEnd();
            }
        }
        
    }
    static Polynomial::const_iterator begin(const Polynomial & p){
        return p.begin();
    }
    static Polynomial::const_iterator end(const Polynomial & p){
        return p.end();
    }
    static Polynomial nf(const GroebnerStrategy& strat, const Polynomial& p, const Monomial& m){
        //return nf3_lexbuckets(strat,p,m);
        if (strat.optRedTailDegGrowth) return nf3(strat,p,m);
        else return nf3_no_deg_growth(strat,p,m);
    }
    typedef Polynomial::const_iterator iterator_type;
    const static bool isDegreeOrder=false;
    const static bool isLexicographicalOrder=true;
    static bool knowRestIsIrreducible(const iterator_type& it, const GroebnerStrategy & strat){
      if ( (it.deg()>0) && (it.firstIndex()>strat.reducibleUntil))
        return true;
      else return false;
      
    }
    static Polynomial sum_range(std::vector<Monomial>& vec,const iterator_type& it, const iterator_type& end){
        if (vec.size()==1) return vec[0];
        if (it!=end)
            return term_accumulate(it,end,Polynomial(0));
        else return 0;
    }
};

class DegOrderHelper{
    public:
    static bool irreducible_lead(const Monomial& m, const GroebnerStrategy& strat){
      return PBORINAME::groebner::irreducible_lead(m,strat);
          }
    static Polynomial::ordered_iterator begin(const Polynomial & p){
        return p.orderedBegin();
    }
    static Polynomial::ordered_iterator end(const Polynomial & p){
        return p.orderedEnd();
    }
    static Polynomial nf(const GroebnerStrategy& strat, const Polynomial& p, const Monomial& m){
        return nf3_degree_order(strat,p,m);
    }
    typedef Polynomial::ordered_iterator iterator_type;
    const static bool isDegreeOrder=true;
    const static bool isLexicographicalOrder=false;
    static bool knowRestIsIrreducible(const iterator_type& it, const GroebnerStrategy & strat){
      return false;
    }
    static Polynomial sum_range(std::vector<Monomial>& vec,iterator_type it, iterator_type end){
          return add_up_monomials(vec);
      }
    
};
class BlockOrderHelper{
    public:
    static bool irreducible_lead(const Monomial& m, const GroebnerStrategy& strat){
      return PBORINAME::groebner::irreducible_lead(m,strat);
          }
    static Polynomial::ordered_iterator begin(const Polynomial & p){
        return p.orderedBegin();
    }
    static Polynomial::ordered_iterator end(const Polynomial & p){
        return p.orderedEnd();
    }
    static Polynomial nf(const GroebnerStrategy& strat, const Polynomial& p, const Monomial& m){
        return nf3(strat,p,m);
    }
    typedef Polynomial::ordered_iterator iterator_type;
    const static bool isDegreeOrder=false;
    const static bool isLexicographicalOrder=false;
    static bool knowRestIsIrreducible(const iterator_type& it, const GroebnerStrategy & strat){
      return false;
    }
    static Polynomial  sum_range(std::vector<Monomial>& vec,iterator_type it, iterator_type end){
            return add_up_monomials(vec);
        }
};
int select_no_deg_growth(const GroebnerStrategy& strat, const Monomial& m){
  MonomialSet ms=strat.leadingTerms.divisorsOf(m);
  if (ms.emptiness())
    return -1;
  else {
    //Monomial min=*(std::min_element(ms.begin(),ms.end(), LessWeightedLengthInStrat(strat)));
    //Exponent min=*(std::min_element(ms.expBegin(),ms.expEnd(), StratComparerForSelect(strat)));
    int selected=-1;
    wlen_type selected_wlen=-1;
    MonomialSet::exp_iterator it=ms.expBegin();
    MonomialSet::exp_iterator end=ms.expEnd();
    while(it!=end){
        Exponent curr=*it;
        int index=strat.exp2Index.find(curr)->second;
        if (strat.generators[index].ecart()==0){
            if (selected<0){
                selected=index;
                selected_wlen=wlen_literal_exceptioned(strat.generators[index]);
            } else {
                if (wlen_literal_exceptioned(strat.generators[index])<selected_wlen){
                    selected=index;
                    selected_wlen=wlen_literal_exceptioned(strat.generators[index]);
                }
            }
        }
        
        it++;
    }
    if ((selected<0)&&(!(LexHelper::irreducible_lead(m,strat)))) cerr<<"select_no_Deg_growth buggy";
  return selected; 
  }
  
}

static Polynomial nf4(GroebnerStrategy& strat, Polynomial p){
  int index;
  while((index=select1(strat,p))>=0){
    assert(index<strat.generators.size());
    Polynomial* g=&strat.generators[index].p;
    
    if((strat.generators[index].ecart()==0) && (strat.generators[index].length<=4) &&(strat.generators[index].lm!=p.lead())){
      wlen_type dummy;
      p=reduce_complete(p,strat.generators[index],dummy);
      
    } else{
      p=spoly(p,*g);
    }
  }
  return p;
  
}

template <class T> Polynomial add_up_generic(const std::vector<T>& res_vec, int start, int end){
    //we assume the polynomials to be pairwise different
    int s=end-start;
    if (s==0) return Polynomial();
    if (s==1) return Polynomial(res_vec[start]);
    int h=s/2;
    return add_up_generic(res_vec,start,start+h)+add_up_generic(res_vec,start+h,end);
    //return add_up_monomials(res_vec,start,start+h)+add_up_monomials(res_vec,start+h,end);
}
template <class T> Polynomial add_up_generic(const std::vector<T>& res_vec){
    //we assume the polynomials to be pairwise different
    int s=res_vec.size();
    if (s==0) return Polynomial();
    if (s==1) return (Polynomial) res_vec[0];
    int h=s/2;
    
    return add_up_generic(res_vec,0,h)+add_up_generic(res_vec,h,s);
}

class LexOrderGreaterComparer{
    LexOrder o;
public:
    bool operator() (const Monomial& m1, const Monomial& m2){
        return o.compare(m1,m2)==BoolePolyRing::greater_than;
    }
    bool operator() (const Exponent& m1, const Exponent& m2){
        return o.compare(m1,m2)==BoolePolyRing::greater_than;
    }
};
static MonomialSet add_up_lex_sorted_monomials(std::vector<Monomial>& vec, int start, int end){
    assert(end<=vec.size());
    assert(start>=0);
    int d=end-start;
    assert(d>=0);
    if (d<=2){
        switch(d){
            case 0:return MonomialSet();
            case 1:return vec[start].diagram();
            case 2: 
              return (vec[start]+vec[start+1]).diagram();
        }

    
    }
    
    //more than two monomial, lex sorted, so if first is  constant, all are constant
    if (vec[start].isOne()) return Polynomial(end-start).diagram();
    assert (!(vec[start].isOne()));
    idx_type idx=*vec[start].begin();
    int limes=end;
    vec[start].popFirst();
    for(limes=start+1;limes<end;limes++){
        if (vec[limes].isOne()||(*vec[limes].begin()!=idx)){
            assert((vec[limes].isOne())||(*vec[limes].begin()>idx));
            break;
        } else 
           vec[limes].popFirst();
            //vec[limes].changeAssign(idx);
    }
    
    return MonomialSet(idx,add_up_lex_sorted_monomials(vec,start,limes),add_up_lex_sorted_monomials(vec,limes,end));
}


static MonomialSet add_up_lex_sorted_exponents(std::vector<Exponent>& vec, int start, int end){
    assert(end<=vec.size());
    assert(start>=0);
    int d=end-start;
    assert(d>=0);
    if (d<=2){
        switch(d){
            case 0:return MonomialSet();
            case 1:return Monomial(vec[start]).diagram();
            case 2: 
              Polynomial res=Monomial(vec[start])+Monomial(vec[start+1]);
              return MonomialSet(res.diagram());
        }

    
    }
    
    //more than two monomial, lex sorted, so if first is  constant, all are constant
    if (vec[start].deg()==0) return Polynomial(end-start).diagram();
    assert (!(vec[start].deg()==0));
    idx_type idx=*vec[start].begin();
    int limes=end;
    vec[start].popFirst();
    for(limes=start+1;limes<end;limes++){
        if ((vec[limes].deg()==0)||(*vec[limes].begin()!=idx)){
            assert((vec[limes].deg()==0)||(*vec[limes].begin()>idx));
            break;
        } else 
           vec[limes].popFirst();
            //vec[limes].changeAssign(idx);
    }
    
    return MonomialSet(idx,add_up_lex_sorted_exponents(vec,start,limes),add_up_lex_sorted_exponents(vec,limes,end));
}

static MonomialSet add_up_lex_sorted_monomial_navs(std::vector<Monomial::const_iterator>& vec, int start, int end){
    assert(end<=vec.size());
    assert(start>=0);
    int d=end-start;
    assert(d>=0);
    if (d<=2){
        switch(d){
            case 0:return MonomialSet();
            case 1:return vec[start];
            case 2: 
              Polynomial res=Polynomial(vec[start])+Polynomial(vec[start+1]);
              return MonomialSet(res.diagram());
        }

    
    }
    
    //more than two monomial, lex sorted, so if first is  constant, all are constant
    if (vec[start].isConstant()) return Polynomial(end-start).diagram();
    assert (!(vec[start].isConstant()));
    idx_type idx=*vec[start];
    int limes=end;
    vec[start]++;
    for(limes=start+1;limes<end;limes++){
        if (vec[limes].isConstant()||(*vec[limes]!=idx)){
            assert((vec[limes].isTerminated())||(*vec[limes]>idx));
            break;
        } else 
           vec[limes]++;
            //vec[limes].changeAssign(idx);
    }
    
    return MonomialSet(idx,add_up_lex_sorted_monomial_navs(vec,start,limes),add_up_lex_sorted_monomial_navs(vec,limes,end));
}

Polynomial add_up_monomials(const std::vector<Monomial>& vec){
    return add_up_generic(vec);

}
Polynomial add_up_polynomials(const std::vector<Polynomial>& vec){
    return add_up_generic(vec);

}
Polynomial add_up_exponents(const std::vector<Exponent>& vec){
    //return add_up_generic(vec);
    std::vector<Exponent> vec_sorted=vec;
    std::sort(vec_sorted.begin(),vec_sorted.end(),LexOrderGreaterComparer());
    
   
    return add_up_lex_sorted_exponents(vec_sorted,0,vec_sorted.size());
}



static Polynomial unite_polynomials(const std::vector<Polynomial>& res_vec, int start, int end){
    //we assume the polynomials to be pairwise different
    int s=end-start;
    if (s==0) return Polynomial();
    if (s==1) return res_vec[start];
    int h=s/2;
    return Polynomial(unite_polynomials(res_vec,start,start+h).diagram().unite(unite_polynomials(res_vec,start+h,end).diagram()));
    //return add_up_monomials(res_vec,start,start+h)+add_up_monomials(res_vec,start+h,end);
}
static Polynomial unite_polynomials(const std::vector<Polynomial>& res_vec){
    //we assume the polynomials to be pairwise different
    int s=res_vec.size();
    if (s==0) return Polynomial();
    if (s==1) return res_vec[0];
    int h=s/2;
    
    return Polynomial(unite_polynomials(res_vec,0,h).diagram().unite(unite_polynomials(res_vec,h,s).diagram()));
}


#if 0
Polynomial red_tail(const GroebnerStrategy& strat, Polynomial p){
  Polynomial res;
  int deg_bound=p.deg();
  std::vector<Monomial> res_vec;
  Polynomial orig_p=p;
  bool changed=false;
  if (!(p.isZero())){
    Monomial lm=p.lead();
    res_vec.push_back(lm);
    p=Polynomial(p.diagram().diff(lm.diagram()));
  }
  while(!(p.isZero())){
    
    //res+=lm;

    
    //p-=lm;
    std::vector<Monomial> irr;
    Polynomial::const_iterator it=p.begin();
    Polynomial::const_iterator end=p.end();
    while((it!=end)&& (irreducible_lead(*it,strat))){
        irr.push_back(*it);
        it++;
    }
    if ((!(changed))&& (it==end)) return orig_p;
    Polynomial irr_p=add_up_monomials(irr);
    int s,i;
    s=irr.size();
    assert(s==irr_p.length());
    //if (s!=irr_p.length()) cout<<"ADDUP FAILED!!!!!!!!!!!!!!!!!!!!!!!!\n";
    for(i=0;i<s;i++){
        res_vec.push_back(irr[i]);
    }
    
    //p=p-irr_p;
    p=Polynomial(p.diagram().diff(irr_p.diagram()));
    if(p.isZero()) break;
    //Monomial lm=p.lead();
    //res_vec.push_back(lm);
    
    
    //p=Polynomial(p.diagram().diff(lm.diagram()));
    p=nf3(strat,p);
    changed=true;
  }
  
  //should use already added irr_p's
  res=add_up_monomials(res_vec);
  return res;
}
#else
Polynomial red_tail_general(const GroebnerStrategy& strat, Polynomial p){
  Polynomial res;
  int deg_bound=p.deg();
  std::vector<Polynomial> res_vec;
  Polynomial orig_p=p;
  bool changed=false;
  if (!(p.isZero())){
    Monomial lm=p.lead();
    res_vec.push_back(lm);
    p=Polynomial(p.diagram().diff(lm.diagram()));
  }
  while(!(p.isZero())){
    
    //res+=lm;

    
    //p-=lm;
    std::vector<Monomial> irr;
    Polynomial::ordered_iterator it=p.orderedBegin();
    Polynomial::ordered_iterator end=p.orderedEnd();
    while((it!=end)&& (irreducible_lead(*it,strat))){
        irr.push_back(*it);
        it++;
    }
    Monomial rest_lead;
    
    if ((!(changed))&& (it==end)) return orig_p;
    //@todo: if it==end irr_p=p, p=Polnomial(0)
    Polynomial irr_p;
    if (it!=end) {
        irr_p=add_up_monomials(irr);
        rest_lead=*it;
        }
    else irr_p=p;
    int s,i;
    s=irr.size();
    assert(s==irr_p.length());
    //if (s!=irr_p.length()) cout<<"ADDUP FAILED!!!!!!!!!!!!!!!!!!!!!!!!\n";
    //for(i=0;i<s;i++){
    //    res_vec.push_back(irr[i]);
    //}
    res_vec.push_back(irr_p);
    //p=p-irr_p;
    p=Polynomial(p.diagram().diff(irr_p.diagram()));
    if(p.isZero()) break;
    //Monomial lm=p.lead();
    //res_vec.push_back(lm);
    
    
    //p=Polynomial(p.diagram().diff(lm.diagram()));
    if (!(BoolePolyRing::isDegreeOrder()))
        p=nf3(strat,p, rest_lead);
    else{
        p=nf3_degree_order(strat,p,rest_lead);
    }
    changed=true;
  }
  
  //should use already added irr_p's
  res=unite_polynomials(res_vec);
  return res;
}

template <class Helper> Polynomial red_tail_generic(const GroebnerStrategy& strat, Polynomial p){
  Polynomial res;
  int deg_bound=p.deg();
  std::vector<Polynomial> res_vec;
  Polynomial orig_p=p;
  bool changed=false;
  if (!(p.isZero())){
    Monomial lm=p.lead();
    res_vec.push_back(lm);
    p=Polynomial(p.diagram().diff(lm.diagram()));
  }
  while(!(p.isZero())){
    
    //res+=lm;
     {
       Polynomial p_bak=p;
       p=mod_mon_set(p.diagram(),strat.monomials);
       
       //p=plug_1(p,strat.monomials_plus_one);
       if (strat.optLL){
         Polynomial p_bak2=p;
         p=ll_red_nf(p,strat.llReductor);
         if (p_bak2!=p){
             p=mod_mon_set(p.diagram(),strat.monomials);
             //p=plug_1(p,strat.monomials_plus_one);
         }
       }
       if (p_bak!=p) changed=true;
     if (p.isZero()) break;
     }
    //p-=lm;
    std::vector<Monomial> irr;
    typename Helper::iterator_type it=Helper::begin(p);
    typename Helper::iterator_type it_orig=it;
    typename Helper::iterator_type end=Helper::end(p);
    bool rest_is_irreducible=false;
    //typedef  (typename Helper::iterator_type) it_type;
    //typedef  (typename it_type::value_type) mon_type;
    //Monomial mymon;
    while((it!=end)&& (Helper::irreducible_lead(*it,strat))){
      if (Helper::knowRestIsIrreducible(it,strat)){
       rest_is_irreducible=true;
       break;
      } else{
        irr.push_back(*it);
        it++;
        
      }
    }
    Monomial rest_lead;
    
    if ((!(changed))&& (it==end)) return orig_p;
    //@todo: if it==end irr_p=p, p=Polnomial(0)
    Polynomial irr_p;
    if ((it!=end) &&(!(rest_is_irreducible))) {
        irr_p=Helper::sum_range(irr,it_orig,it);//add_up_monomials(irr);
        rest_lead=*it;
        
        }
    else irr_p=p;
    int s,i;
    s=irr.size();

    assert((s==irr_p.length())||(rest_is_irreducible));

    res_vec.push_back(irr_p);

    p=Polynomial(p.diagram().diff(irr_p.diagram()));
    if(p.isZero()) break;
    p=Helper::nf(strat,p,rest_lead);
    changed=true;
  }
  
  //should use already added irr_p's
  res=unite_polynomials(res_vec);
  return res;
}


/*
class LexHelper{
    public:
    static bool irreducible_lead(const Monomial& m, const GroebnerStrategy& strat){
        if (strat.optRedTailDegGrowth) return PBORINAME::groebner::irreducible_lead(m,strat);
        else{
            BooleSet ms=strat.leadingTerms.intersect(m.divisors());
            if (ms.emptiness())
                return true;
            else {
                return std::find_if(ms.expBegin(),ms.expEnd(),IsEcart0Predicate(strat))==ms.expEnd();
            }
        }
        
    }
    static Polynomial::const_iterator begin(const Polynomial & p){
        return p.begin();
    }
    static Polynomial::const_iterator end(const Polynomial & p){
        return p.end();
    }
    static Polynomial nf(const GroebnerStrategy& strat, const Polynomial& p, const Monomial& m){
        if (strat.optRedTailDegGrowth) return nf3(strat,p,m);
        else return nf3_no_deg_growth(strat,p,m);
    }
    typedef Polynomial::const_iterator iterator_type;
    const static bool isDegreeOrder=false;
};

class DegOrderHelper{
    public:
    static bool irreducible_lead(const Monomial& m, const GroebnerStrategy& strat){
      return PBORINAME::groebner::irreducible_lead(m,strat);
          }
    static Polynomial::ordered_iterator begin(const Polynomial & p){
        return p.orderedBegin();
    }
    static Polynomial::ordered_iterator end(const Polynomial & p){
        return p.orderedEnd();
    }
    static Polynomial nf(const GroebnerStrategy& strat, const Polynomial& p, const Monomial& m){
        return nf3_degree_order(strat,p,m);
    }
    typedef Polynomial::ordered_iterator iterator_type;
    const static bool isDegreeOrder=true;
};*/

Polynomial red_tail(const GroebnerStrategy& strat, Polynomial p){
    if (BoolePolyRing::isLexicographical())
        return red_tail_generic<LexHelper>(strat,p);
    if (BoolePolyRing::isDegreeOrder())
        return red_tail_generic<DegOrderHelper>(strat,p);
    if (BoolePolyRing::isBlockOrder())
        return red_tail_generic<BlockOrderHelper>(strat,p);
    return red_tail_general(strat,p);
}
#endif
Polynomial red_tail_short(const GroebnerStrategy& strat, Polynomial p){
  Polynomial res;
  while(!(p.isZero())){
    Polynomial lm=p.lead();
    res+=lm;
    p-=lm;
    p=nf3_short(strat,p);
  }
  return res;
}
Polynomial red_tail_self_tuning(const GroebnerStrategy& strat, Polynomial p){
  Polynomial res;
  int orig_length=p.length();
  bool short_mode=false;
  while(!(p.isZero())){
    Polynomial lm=p.lead();
    res+=lm;
    p-=lm;
    if (short_mode)
      p=nf3_short(strat,p);
    else
      p=nf3(strat,p, p.lead());
    if ((!short_mode)&&(p.length()+res.length()>2*orig_length+5))
      short_mode=true;
  }
  return res;
}


template <bool have_redsb> Polynomial ll_red_nf_generic(const Polynomial& p,const BooleSet& reductors){
    
    if (p.isConstant()) return p;
    //if (reductors.emptiness()) return p;
    
  MonomialSet::navigator p_nav=p.navigation();
  idx_type p_index=*p_nav;
  MonomialSet::navigator r_nav=reductors.navigation();

  
  while((*r_nav)<p_index){
      
      r_nav.incrementThen();
  }
  if (r_nav.isConstant())
      return p;
  typedef PBORI::CacheManager<CCacheTypes::ll_red_nf>
    cache_mgr_type;
  cache_mgr_type cache_mgr;
  MonomialSet::navigator cached =
    cache_mgr.find(p_nav,r_nav);
  if (cached.isValid()) return MonomialSet(cached);
  Polynomial res;
  if ((*r_nav)==p_index){
    if (have_redsb){  
    res=ll_red_nf_generic<have_redsb>(MonomialSet(p_nav.elseBranch()),r_nav.thenBranch())
      +Polynomial(MonomialSet(r_nav.elseBranch()))*ll_red_nf_generic<have_redsb>(MonomialSet(p_nav.thenBranch()),r_nav.thenBranch());
   }else{
    res=ll_red_nf_generic<have_redsb>(MonomialSet(p_nav.elseBranch()),r_nav.thenBranch())
         +ll_red_nf_generic<have_redsb>(Polynomial(MonomialSet(r_nav.elseBranch())),r_nav.thenBranch())*ll_red_nf_generic<have_redsb>(MonomialSet(p_nav.thenBranch()),r_nav.thenBranch());
   }
  } else{
      assert((*r_nav)>p_index);
      
      res=
      MonomialSet(
        p_index,
        ll_red_nf_generic<have_redsb>(MonomialSet(p_nav.thenBranch()),r_nav).diagram(),
        ll_red_nf_generic<have_redsb>(MonomialSet(p_nav.elseBranch()),r_nav).diagram());
      
  }
  cache_mgr.insert(p_nav,r_nav,res.navigation());
  return res;

    
}
Polynomial ll_red_nf(const Polynomial& p,const BooleSet& reductors){
    return ll_red_nf_generic<true>(p,reductors);
}
Polynomial ll_red_nf_noredsb(const Polynomial& p,const BooleSet& reductors){
    return ll_red_nf_generic<false>(p,reductors);
}

Polynomial do_plug_1(const Polynomial& p, const MonomialSet& m_plus_ones){
    MonomialSet::navigator m_nav=m_plus_ones.navigation();
    
    if (m_nav.isConstant()){
        return p;
    }
    Polynomial::navigator p_nav=p.navigation();
    if (p_nav.isConstant()) return p;
    idx_type p_index=*p_nav;
    while(p_index>*m_nav){
        assert(!(m_nav.isConstant()));
        m_nav.incrementElse();
    }
    assert (p_index=*p_nav);
    typedef PBORI::CacheManager<CCacheTypes::plug_1>
      cache_mgr_type;
    cache_mgr_type cache_mgr;
    MonomialSet::navigator cached =
      cache_mgr.find(p_nav,m_nav);
    if (cached.isValid()) return MonomialSet(cached);
    MonomialSet res;
    if (p_index==*m_nav){  
    MonomialSet m1(m_nav.thenBranch());
    MonomialSet m0(m_nav.elseBranch());
    MonomialSet p1=p_nav.thenBranch();
    MonomialSet p1_irr_s1=mod_mon_set(p1,m1);
    MonomialSet p1_red_s1=p1.diff(p1_irr_s1);
    MonomialSet p0=p_nav.elseBranch();
    Polynomial res0=do_plug_1(p1_red_s1,m1)+do_plug_1(p0,m0);
    Polynomial res1=do_plug_1(p1_irr_s1,m0);
    res=MonomialSet(p_index,res1.diagram(),res0.diagram());
    } else {
        assert(p_index<*m_nav);
        res=MonomialSet(p_index,do_plug_1(p_nav.thenBranch(),m_plus_ones).diagram(),do_plug_1(p_nav.elseBranch(),m_plus_ones).diagram());
    }
    cache_mgr.insert(p_nav,m_nav,res.navigation());
    
    return res;
}

Polynomial plug_1_top(const Polynomial& p, const MonomialSet& m_plus_ones){
    Polynomial  irr=mod_mon_set(p.diagram(),m_plus_ones);
    Polynomial red=p.diagram().diff(irr);
    return irr+do_plug_1(red,m_plus_ones);
}
Polynomial plug_1(const Polynomial& p, const MonomialSet& m_plus_ones){
    Polynomial p1,p2;
    p1=p;
    p2=plug_1_top(p1,m_plus_ones);
    while(p1!=p2){
        Polynomial h=p2;
        p2=plug_1_top(p1,m_plus_ones);
        p1=h;
    }
    return p2;
}
#ifdef HAVE_NTL
using std::vector;
vector<Polynomial> GroebnerStrategy::noroStep(const vector<Polynomial>& orig_system){
    vector<Polynomial> polys;
    int i;
    MonomialSet terms;
    for(i=0;i<orig_system.size();i++){
        Polynomial p=orig_system[i];
        if (!(p.isZero())){
            p=ll_red_nf(p,llReductor);
            if (!(p.isZero())){
                p=nf(p);
                if (!(p.isZero())){
                    p=red_tail(*this,p);
                    terms=terms.unite(p.diagram());
                    polys.push_back(p);
                }
            }
        }
    }
    if (polys.size()==0) return vector<Polynomial>();
    typedef std::map<int,Exponent> to_term_map_type;
    typedef Exponent::idx_map_type from_term_map_type;
    
    int rows=polys.size();
    int cols=terms.size();
    if (this->enabledLog){
        std::cout<<"ROWS:"<<rows<<"COLUMNS:"<<cols<<std::endl;
    }

    mat_GF2 mat(INIT_SIZE,rows,cols);

    std::vector<Exponent> terms_as_exp(terms.size());
    std::copy(terms.expBegin(),terms.expEnd(),terms_as_exp.begin());
    std::sort(terms_as_exp.begin(),terms_as_exp.end(),std::greater<Exponent>());
    from_term_map_type from_term_map;
    //to_term_map_type to_term_map;
    for (i=0;i<terms_as_exp.size();i++){
        from_term_map[terms_as_exp[i]]=i;
        //to_term_map[i]=terms_as_exp[i]);
    }
    for(i=0;i<polys.size();i++){
        Polynomial::exp_iterator it=polys[i].expBegin();//not order dependend
        Polynomial::exp_iterator end=polys[i].expEnd();
        while(it!=end){
            mat[i][from_term_map[*it]]=1;
            it++;
        }
    }
    polys.clear();
    int rank=gauss(mat);
    for(i=0;i<rank;i++){
        int j;
        vector<Exponent> p_t;
        for(j=0;j<cols;j++){
            if (mat[i][j]==1){
                p_t.push_back(terms_as_exp[j]);
            }
        }
        polys.push_back(add_up_exponents(p_t));//,0,p_t.size()));
    }
    return polys;
}
#endif
#if  defined(HAVE_NTL) || defined(HAVE_M4RI)
using std::vector;
vector<Polynomial> GroebnerStrategy::faugereStepDense(const vector<Polynomial>& orig_system){
    vector<Polynomial> extendable_system=orig_system;
    vector<Polynomial> polys;
    vector<Monomial> leads_from_strat_vec;
    int i;
    MonomialSet terms;
    MonomialSet leads_from_strat;
    for(i=0;i<extendable_system.size();i++){
        Polynomial p_orig=extendable_system[i];
        
        
        if (p_orig.isZero()) continue;
        Polynomial p=mod_mon_set(p_orig.diagram(),monomials);
        if (optLL){
            Polynomial p_bak2=p;
            p=ll_red_nf(p,llReductor);
            if (p!=p_bak2) p=mod_mon_set(p.diagram(),monomials);
        }
        MonomialSet new_terms=p.diagram().diff(terms);
        MonomialSet::const_iterator it=new_terms.begin();
        MonomialSet::const_iterator end=new_terms.end();
        
        //bool from_strat=(i>=orig_system.size());
        //if ((from_strat)&&(p_orig!=p) &&(p_orig.leadExp()!=p.leadExp())) from_strat=false;
        while(it!=end){
            Monomial m=*it;
            
            int index=select1(*this,m);
            if (index>=0){
                    //leads_from_strat_vec.push_back(m);
                    //leads_from_strat=leads_from_strat.unite(m.diagram());
                    Monomial m2=m/generators[index].lm;
                    Polynomial p2=m2*generators[index].p;
                    extendable_system.push_back(p2);
            }
            it++;
        }
        terms=terms.unite(new_terms);
        polys.push_back(p);
    }
    
    leads_from_strat=terms.diff(mod_mon_set(terms,minimalLeadingTerms));//add_up_monomials(leads_from_strat_vec).diagram().unite(this->monomials);
    if (polys.size()==0) return vector<Polynomial>();
    typedef std::map<int,Monomial> to_term_map_type;
    typedef Exponent::idx_map_type from_term_map_type;
    
    int rows=polys.size();
    int cols=terms.size();
    if (this->enabledLog){
        std::cout<<"ROWS:"<<rows<<"COLUMNS:"<<cols<<std::endl;
    }
    #ifndef HAVE_M4RI
    mat_GF2 mat(INIT_SIZE,rows,cols);
    #else
    packedmatrix* mat=createPackedMatrix(rows,cols);
    #endif
    std::vector<Exponent> terms_as_exp(terms.size());
    std::copy(terms.expBegin(),terms.expEnd(),terms_as_exp.begin());
    std::vector<Exponent> terms_as_exp_lex(terms_as_exp);
    std::sort(terms_as_exp.begin(),terms_as_exp.end(),std::greater<Exponent>());
    std::vector<int> ring_order2lex(terms_as_exp.size());
    std::vector<int> lex_order2ring(terms_as_exp.size());
    from_term_map_type from_term_map;
    //to_term_map_type to_term_map;
    for (i=0;i<terms_as_exp.size();i++){
        from_term_map[terms_as_exp[i]]=i;
        //to_term_map[i]=Monomial(terms_as_exp[i]);
    }
    for (i=0;i<terms_as_exp_lex.size();i++){
        int ring_pos=from_term_map[terms_as_exp_lex[i]];
        ring_order2lex[ring_pos]=i;
        lex_order2ring[i]=ring_pos;
        //to_term_map[i]=Monomial(terms_as_exp[i]);
    }
    for(i=0;i<polys.size();i++){
        Polynomial::exp_iterator it=polys[i].expBegin();//not order dependend
        Polynomial::exp_iterator end=polys[i].expEnd();
        while(it!=end){
            #ifndef HAVE_M4RI
            mat[i][from_term_map[*it]]=1;
            #else
            writePackedCell(mat,i,from_term_map[*it],1);
            #endif
            it++;
        }
    }
    polys.clear();
    #ifndef HAVE_M4RI
    int rank=gauss(mat);
    #else
    //int rank=gaussianPacked(mat, YES);
    int rank=simpleFourRussiansPackedFlex(mat, YES, 16);
    #endif
    //std::cout<<"rank:"<<rank<<std::endl;
    if (this->enabledLog){
        std::cout<<"finished gauss"<<std::endl;
    }
    for(i=0;i<rank;i++){
        int j;
        vector<int> p_t_i;
        
        bool from_strat=false;
        for(j=0;j<cols;j++){
            #ifndef HAVE_M4RI
            if (mat[i][j]==1){
            #else
            if(readPackedCell(mat,i,j)==1){
            #endif
                if (p_t_i.size()==0){
                    if (leads_from_strat.owns(terms_as_exp[j])) {
                        from_strat=true;break;
                    }
                }
                p_t_i.push_back(ring_order2lex[j]);
            }
        }
        if (!(from_strat)){
            vector<Exponent> p_t(p_t_i.size());
            std::sort(p_t_i.begin(),p_t_i.end(),std::less<int>());            
            for(j=0;j<p_t_i.size();j++){
                p_t[j]=terms_as_exp_lex[p_t_i[j]];
            }
            polys.push_back(add_up_lex_sorted_exponents(p_t,0,p_t.size()));
            assert(!(polys[polys.size()-1].isZero()));
        }
    }
    #ifdef HAVE_M4RI
    destroyPackedMatrix(mat);
    #endif
    return polys;
}
#endif

MonomialSet mod_mon_set(const MonomialSet& as, const MonomialSet &vs){
  MonomialSet::navigator a=as.navigation();
  MonomialSet::navigator v=vs.navigation();
  idx_type a_index=*a;
  idx_type v_index=*v;
  if (vs.ownsOne()) return MonomialSet();
  if (a.isConstant()) {
     // if (!(vs.ownsOne()))
          return as;
      //else return MonomialSet();
  }
  
  
  while((v_index=*v)<(a_index=*a)){
        v.incrementElse();
    }
  if (v.isConstant()) {
      //if (v.isTerminated()) return MonomialSet();
      //else
          return as;
  }
  if (v==a) return MonomialSet(); 
  /*else 
  {
      if (MonomialSet(v).ownsOne()) return MonomialSet();
  }*/
  typedef PBORI::CacheManager<CCacheTypes::mod_mon_set>
    cache_mgr_type;
  cache_mgr_type cache_mgr;
  MonomialSet::navigator cached =
    cache_mgr.find(a, v);
  if (cached.isValid()) return cached;
  MonomialSet result;
  if (a_index==v_index){
    result=MonomialSet(a_index,
    mod_mon_set(mod_mon_set(a.thenBranch(), v.thenBranch()),v.elseBranch()),
    mod_mon_set(a.elseBranch(),v.elseBranch())
    );
    
  } else {
    assert(v_index>a_index);
    result=MonomialSet(a_index,
      mod_mon_set(a.thenBranch(),v),
      mod_mon_set(a.elseBranch(), v));
  }
  cache_mgr.insert(a,v,result.navigation());
  return result;
}
Polynomial GroebnerStrategy::nf(Polynomial p){
    if (p.isZero()) return p;
    if (BoolePolyRing::isDegreeOrder()) return nf3_degree_order(*this,p,p.lead());
    else return nf3(*this,p,p.lead());
}
END_NAMESPACE_PBORIGB
