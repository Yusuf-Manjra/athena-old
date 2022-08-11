// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthContainers/tools/ElementProxy.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Sep, 2010
 * @brief Proxy for lvalue access to @c DataVector/@c DataList elements.
 */


#ifndef ATHCONTAINERS_TOOLS_ELEMENTPROXY_H
#define ATHCONTAINERS_TOOLS_ELEMENTPROXY_H


#include "AthContainers/OwnershipPolicy.h"
#include "AthLinks/tools/selection_ns.h"
#include "CxxUtils/concepts.h"
#include <memory>


// Forward declarations.
ENTER_ROOT_SELECTION_NS
namespace DataModel_detail {
template <class DVL> class ElementProxy;
}
EXIT_ROOT_SELECTION_NS


namespace DataModel_detail {


/**
 * @brief Proxy for lvalue access to @c DataVector/@c DataList elements.
 *
 * We need to be careful with lvalue references to @c DataVector/List elements.
 * If the @c DataVector/List owns its elements, then if we write into the
 * @c DataVector/List via an lvalue, the old element should be deleted
 * (and the @c DataVector/List then takes ownership of the new value).
 * We implement this by having the lvalue references return a proxy
 * object rather than a raw reference.  The proxy handles
 * ownership issues.  To do this, the proxy stores a pointer
 * to the original container.
 *
 * When inheritance is used with @c DataVector/List, we have the additional
 * restriction that we can only write into the @c DataVector/List using
 * the most-derived @c DataVector/List class.  We also need to enforce that
 * in @c ElementProxy when there's an attempt to write via an lvalue.
 * We can also get this information from the original container.
 */
template <class DVL>
class ElementProxy
{
public:
  /**
   * @brief Constructor.
   * @param i The underlying container iterator pointing at
   *          the element which we're proxying.
   * @param container The container that the iterator references.
   */
  ElementProxy (typename DVL::BaseContainer::iterator i,
                DVL* container);


  /// Default copy.
  ElementProxy (const ElementProxy&) = default;


  /**
   * @brief Assignment operator, from an @c Element proxy.
   * @param rhs The proxy from which we're assigning.
   *
   * If @a rhs is the same as the element we're proxying, then
   * we don't need to do anything.  Otherwise, @c can_insert must
   * be true.  If the parent @c DataVector/List owns its elements,
   * we then need to delete the proxied object before making
   * the assignment.  We also disallow copying between two
   * @c DataVector/List's, both of which own their elements.
   */
  ElementProxy& operator= (const ElementProxy& rhs);


  /**
   * @brief Assignment operator, from a pointer.
   * @param rhs The pointer from which we're assigning.
   *
   * If @a rhs is the same as the element we're proxying, then
   * we don't need to do anything.  Otherwise, @c can_insert must
   * be true.  If the parent @c DataVector/List owns its elements,
   * we then need to delete the proxied object before making
   * the assignment.
   */
  // Disable this method if the container must own its elements.
  // In that case, only the unique_ptr overload is relevant.
  ATH_MEMBER_REQUIRES(!DVL::must_own, ElementProxy&)
  operator= (typename DVL::value_type rhs);


  /**
   * @brief Assignment operator, from a unique_ptr.
   * @param rhs The pointer from which we're assigning.
   *
   * If @a rhs is the same as the element we're proxying, then
   * we don't need to do anything.  Otherwise, @c can_insert must
   * be true.  The container must own its elements in order
   * to use this interface.
   */
  ElementProxy& operator= (typename DVL::unique_type rhs);


  /**
   * @brief Conversion to a (const) pointer.
   *
   * We just need to do a cast here.
   */
  operator typename DVL::value_type const() const;


  /**
   * @brief Conversion to a (const) pointer.
   *
   * We just need to do a cast here.
   */
  typename DVL::value_type const operator-> () const;



  /**
   * @brief Return the container holding the element that this object proxies.
   */
  DVL* container();


  /**
   * @brief Return the container holding the element that this object proxies.
   */
  const DVL* container() const;


  typedef typename
    ROOT_SELECTION_NS::DataModel_detail::ElementProxy<DVL>::self
  self;


private:
  /// Iterator pointing at the proxied element.
  typename DVL::BaseContainer::iterator m_proxied;

  /// The container that it references.
  DVL* m_container;
};


} // namespace DataModel_detail


ENTER_ROOT_SELECTION_NS

namespace DataModel_detail {

template <class DVL> class ElementProxy : public SelectNoInstance
{
public:
  typedef ElementProxy<DVL> self;
  ROOT_SELECTION_NS::MemberAttributes< kTransient > m_proxied;
  ROOT_SELECTION_NS::MemberAttributes< kTransient > m_container;
};

}

EXIT_ROOT_SELECTION_NS


#include "AthContainers/tools/ElementProxy.icc"


#endif // not ATHCONTAINERS_TOOLS_ELEMENTPROXY_H
