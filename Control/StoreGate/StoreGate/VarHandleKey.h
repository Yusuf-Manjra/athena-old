// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

// $Id$
/**
 * @file StoreGate/VarHandleKey.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2016
 * @brief A property holding a SG store/key/clid from which a VarHandle is made.
 */


#ifndef STOREGATE_VARHANDLEKEY_H
#define STOREGATE_VARHANDLEKEY_H


#include "AthenaKernel/IProxyDict.h"
#include "AthenaKernel/StoreID.h"
#include "GaudiKernel/DataHandle.h"
#include "GaudiKernel/ServiceHandle.h"


namespace SG {


enum AllowEmptyEnum {
  AllowEmpty = 1
};


class VarHandleBase;
class VarHandleKeyProperty;

/**
 * @brief A property holding a SG store/key/clid from which a VarHandle is made.
 *
 * This class holds the key part of a VarHandle.  This consists of a handle
 * to the referenced store, the CLID, and the StoreGate key.  The VarHandle
 * classes derived from this.  However, the VarHandle classes cache information
 * about the referenced object, so they can't be used as members of a
 * reentrant algorithm.  Instead, we define separate key classes deriving
 * from this that can be used as properties of algorithms or tools.
 * A VarHandle can then be constructed from the key object and (optionally)
 * an event context object.
 *
 * The classes @c WriteHandleKey, @c ReadHandleKey, and @c UpdateHandleKey
 * derive from this.
 *
 * The actual StoreGate key is stored in the base @c DataHandle object.
 * A reference to the store is saved in this class.
 *
 * The string for the key property can optionally be prefixed with the store
 * name, separated by a "+": "MyStore+Obj".  (However, if the key name
 * starts with a slash, it is interpreted as a hierarchical key name,
 * not an empty store name.)
 */
class VarHandleKey
  : public Gaudi::DataHandle
{
public:
  /**
   * @brief Constructor.
   * @param clid The class ID for the referenced object.
   * @param sgkey The StoreGate key for the object.
   * @param a Mode: read/write.
   * @param storeName Name to use for the store, if it's not encoded in sgkey.
   * @param isCond True if this is a CondHandleKey.
   *
   * The provided key may actually start with the name of the store,
   * separated by a "+":  "MyStore+Obj".  If no "+" is present
   * the store named by @c storeName is used.  However, if the key name
   * starts with a slash, it is interpreted as a hierarchical key name,
   * not an empty store name.
   *
   * A SG::ExcBadHandleKey exception will the thrown if the key string
   * format is bad.
   */
  VarHandleKey (CLID clid,
                const std::string& sgkey,
                Gaudi::DataHandle::Mode a,
                const std::string& storeName = StoreID::storeName(StoreID::EVENT_STORE),
                bool isCond = false);


  /**
   * @brief Declare corresponding property type
   */
  using PropertyType = SG::VarHandleKeyProperty;


  /**
   * @brief Change the key of the object to which we're referring.
   * @param sgkey The StoreGate key for the object.
   * 
   * The provided key may actually start with the name of the store,
   * separated by a "+":  "MyStore+Obj".  If no "+" is present,
   * the store is not changed.  A key name that starts with a slash
   * is interpreted as a hierarchical key name, not an empty store name.
   *
   * A SG::ExcBadHandleKey exception will the thrown if the key string
   * format is bad.
   */
  VarHandleKey& operator= (const std::string& sgkey);


  /**
   * @brief Change the key of the object to which we're referring.
   * @param sgkey The StoreGate key for the object.
   * 
   * The provided key may actually start with the name of the store,
   * separated by a "+":  "MyStore+Obj".  If no "+" is present
   * the store is not changed.  A key name that starts with a slash
   * is interpreted as a hierarchical key name, not an empty store name.
   *
   * Returns failure the key string format is bad.
   */
  virtual StatusCode assign (const std::string& sgkey);

  
  /**
   * @brief If this object is used as a property, then this should be called
   *        during the initialize phase.  It will fail if the requested
   *        StoreGate service cannot be found or if the key is blank.
   *
   * @param used If false, then this handle is not to be used.
   *             Instead of normal initialization, the key will be cleared.
   */
  StatusCode initialize (bool used = true);


  /**
   * @brief If this object is used as a property, then this should be called
   *        during the initialize phase.  This variant will allow the key
   *        to be blank.
   * @param Flag to select this variant.  Call like
   *@code
   *  ATH_CHECK( key.initialize (SG::AllowEmpty) );
   @endcode
   */
  StatusCode initialize (AllowEmptyEnum);


  /**
   * @brief Return the class ID for the referenced object.
   */
  CLID clid() const;


  /**
   * @brief Return the StoreGate ID for the referenced object.
   */
  const std::string& key() const;


  /**
   * @brief Test if the key is blank.
   */
  bool empty() const;


  /**
   * @brief Return handle to the referenced store.
   */
  const ServiceHandle<IProxyDict>& storeHandle() const;


  /**
   * @brief Return the VarHandle that owns this key, if any.
   *
   * This should only be non-null for keys that are created automatically
   * by a VarHandle when it is _not_ created from a VarHandleKey.
   * This should always be null for keys that are created explicitly.
   */
  VarHandleBase* owningHandle();


  /**
   * @brief Does this key reference the primary event store?
   */
  bool isEventStore() const;


  /**
   * @brief Called by the owning algorithm during the START transition.
   *
   * AthAlgorithm and friends will call this during START.  This allows
   * for extra initialization that we can't do during initialize(), such
   * as retrieving a conditions container from the store.
   *
   * The default implementation is a no-op.
   */
  virtual StatusCode start();


  /** 
   * @brief Return the hashed StoreGate key.
   *
   * May be 0 if not yet initialized.
   */
  SG::sgkey_t hashedKey() const;

private:
  /// Set the owning handle.  Only callable from VarHandleBase.
  friend class VarHandleBase;
  void setOwningHandle (VarHandleBase* handle);


  /// Don't allow calling these.
  virtual void setKey(DataObjID key) const override final;
  virtual void updateKey(std::string key) const override final;


  /**
   * @brief Handle assignment/construction from a string key.
   * @param sgkey The StoreGate key for the referenced object.
   *
   * The provided key may actually start with the name of the store,
   * separated by a "+":  "MyStore+Obj".  If no "+" is present,
   * the store named by @c storeName is used.  A key name that starts
   * with a slash is interpreted as a hierarchical key name,
   * not an empty store name.
   *
   * see implementation comments for further details
   */
  void parseKey (const std::string& sgkey, const std::string& storeName);


  /**
   * @brief Update the name of the store to which we're referring.
   * @param name The new store name.
   */
  void updateHandle (const std::string& name);

  /**
   * @brief Python representation of Handle.
   */
  virtual std::string pythonRepr() const override;

  /// Handle to the referenced store.
  ServiceHandle<IProxyDict> m_storeHandle;

  /// StoreGate key, that doesn't include the storename
  std::string m_sgKey;

  /// The hashed StoreGate key.  May be 0 if not yet initialized.
  SG::sgkey_t m_hashedKey = 0;

  /// Cache test for whether we're referencing the event store.
  bool m_isEventStore = false;

  /// Handle that owns this key, or nullptr if it is not owned.
  VarHandleBase* m_owningHandle = nullptr;
};


} // namespace SG

namespace std {
  ostream& operator<<(ostream& s, const SG::VarHandleKey& m);
}


#include "StoreGate/VarHandleKey.icc"


#endif // not STOREGATE_VARHANDLEKEY_H
