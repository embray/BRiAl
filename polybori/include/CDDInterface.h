// -*- c++ -*-
//*****************************************************************************
/** @file 
 *
 * @author Alexander Dreyer
 * @date  2006-03-20
 *
 * This file define specializations for the CDDInterface<> template class, which
 * allow unified access to various binary decision diagram implementations.
 *
 * @note Currently only interface to Cudd's ZDDs are available.
 *
 * @par Copyright:
 *   (c) 2006 by
 *   Dep. of Mathematics, Kaiserslautern University of Technology and @n
 *   Fraunhofer Institute for Industrial Mathematics (ITWM)
 *   D-67663 Kaiserslautern, Germany
 *
 * @internal 
 * @version \$Id$
 *
 * @par History:
 * @verbatim
 * $Log$
 * Revision 1.3  2006/03/24 15:02:44  dreyer
 * ADD: Reference to manager_type can also be used for CDDManager<> -nterface
 * ADD: lead(), (n)usedVariables(), lmDeg() implemented in BoolePolynomial
 *
 * Revision 1.2  2006/03/23 17:15:04  dreyer
 * ADD: lead() and lmdeg() functionality to BoolePolynomial,
 * BoolePolyRing(const manager_type &); leading term exampl.
 *
 * Revision 1.1  2006/03/20 14:51:00  dreyer
 * CHANGE: Use CDDInterface temple specializations instead of raw dd_type
 *
 * @endverbatim
**/
//*****************************************************************************

#ifndef CDDInterface_h_
#define CDDInterface_h_

// load basic definitions
#include "pbori_defs.h"

BEGIN_NAMESPACE_PBORI

/** @class CDDInterfaceBase
 *
 * @brief This is the common base for the specialized template class
 * CDDInterface.
 *
 **/

template<class DDType>
class CDDInterfaceBase {

 public:

  /// The interfaced type
  typedef DDType interfaced_type;

  /// Generic access to type of *this
  typedef CDDInterfaceBase<interfaced_type> self;

  /// Default constructor
  CDDInterfaceBase() :
    m_interfaced() {}

  /// Construct instance from interfaced type
  CDDInterfaceBase(const interfaced_type& interfaced) :
    m_interfaced(interfaced) {}

  /// Copy constructor
  CDDInterfaceBase(const self& rhs) :
    m_interfaced(rhs.m_interfaced) {}

  /// Destructor
  ~CDDInterfaceBase() {}

  /// Casting operator to interfaced type
  operator interfaced_type&() { return m_interfaced; }

  /// Constant casting operator to interfaced type
    operator const interfaced_type&() const { return m_interfaced; }

 protected:
  interfaced_type m_interfaced;
};


/** @class CDDInterface<ZDD>
 *
 * @brief this specialization of the template class CDDInterface gives an
 * interface to Cudd's ZDD type.
 *
 **/

template<>
class CDDInterface<ZDD>:
 public CDDInterfaceBase<ZDD> {
 public:
  
  /// Interfacing Cudd's zero-suppressed decision diagram type
  typedef ZDD interfaced_type;
  
  /// Cudd's decision diagram manager type
  typedef Cudd manager_base;

  /// Interface to Cudd's decision diagram manager type
  typedef CDDManager<Cudd> manager_type;

  /// Generic access to base type
  typedef CDDInterfaceBase<interfaced_type> base_type;

  /// Generic access to type of *this
  typedef CDDInterface<interfaced_type> self;

  /// Define size type
  typedef CTypes::size_type size_type;

  /// Define index type
  typedef CTypes::idx_type idx_type;

  /// Type for output streams
  typedef CTypes::ostream_type ostream_type;

  /// Type for comparisons
  typedef CTypes::bool_type bool_type;

  /// Default constructor
  CDDInterface(): base_type() {}

  /// Copy constructor
  CDDInterface(const self& rhs): base_type(rhs) {}

  /// Construct from interfaced type
  CDDInterface(const interfaced_type& rhs): base_type(rhs) {}

  /// Destructor
  ~CDDInterface() {}

  /// Set union
  self unite(const self& rhs) const {
    return self(base_type(m_interfaced.Union(rhs.m_interfaced)));
  };

  /// Set union with assignment
  self& uniteAssign(const self& rhs) {
    m_interfaced = m_interfaced.Union(rhs.m_interfaced);
    return *this;
  };

  /// Set difference
  self diff(const self& rhs) const {
    return m_interfaced.Diff(rhs.m_interfaced);
  };

  /// Set difference with assignment
  self& diffAssign(const self& rhs) {
    m_interfaced = m_interfaced.Diff(rhs.m_interfaced);
    return *this;
  };

  /// Set intersection
  self intersect(const self& rhs) const {
    return m_interfaced.Intersect(rhs.m_interfaced);
  };

  /// Set intersection with assignment
  self& intersectAssign(const self& rhs) {
    m_interfaced = m_interfaced.Intersect(rhs.m_interfaced);
    return *this;
  };

  /// Product
  self product(const self& rhs) const {
    return m_interfaced.Product(rhs.m_interfaced);
  };

  /// Product with assignment
  self& productAssign(const self& rhs) {
    m_interfaced = m_interfaced.Product(rhs.m_interfaced);
    return *this;
  };

  /// Unate product
  self unateProduct(const self& rhs) const {
    return m_interfaced.UnateProduct(rhs.m_interfaced);
  };

  /// Unate product with assignment
  self& unateProductAssign(const self& rhs) {
    m_interfaced = m_interfaced.UnateProduct(rhs.m_interfaced);
    return *this;
  };

  /// Generate subset, where decision diagram manager variable idx is false
  self subset0(idx_type idx) const {
    return m_interfaced.Subset0(idx);
  };

  /// subset0 with assignment
  self& subset0Assign(idx_type idx) {
    m_interfaced = m_interfaced.Subset0(idx);
    return *this;
  };

  /// Generate subset, where decision diagram manager variable idx is asserted
  self subset1(idx_type idx) const {
    return m_interfaced.Subset1(idx);
  };

  /// subset1 with assignment
  self& subset1Assign(idx_type idx) {
    m_interfaced = m_interfaced.Subset1(idx);
    return *this;
  };

  /// Substitute variable mit index idx with its complement
  self change(idx_type idx) const {
    return m_interfaced.Change(idx);
  };

  /// Change with assignment
  self& changeAssign(idx_type idx) {
    m_interfaced = m_interfaced.Change(idx);
    return *this;
  };

  /// Get number of nodes in decision diagram
  size_type nNodes() const {
    return Cudd_zddDagSize(m_interfaced.getNode());
  }

  /// Get number of nodes in decision diagram
  ostream_type& print(ostream_type& os) const {

    m_interfaced.print( Cudd_ReadZddSize(manager().getManager()) );
    m_interfaced.PrintMinterm();
    return os;
  }

  /// Equality check
  bool_type operator==(const self& rhs) const {
    return (m_interfaced == rhs.m_interfaced);
  }

  /// Nonequality check
  bool_type operator!=(const self& rhs) const {
    return (m_interfaced != rhs.m_interfaced);
  }

  /// Get reference to actual decision diagram manager 
  manager_base& manager() const {
    return *m_interfaced.manager();
  }
};


END_NAMESPACE_PBORI

#endif // of #ifndef CDDInterface_h_
