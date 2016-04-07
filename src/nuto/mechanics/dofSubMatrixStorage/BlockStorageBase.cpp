/*
 * BlockStorageBase.cpp
 *
 *  Created on: 4 Apr 2016
 *      Author: ttitsche
 */

#include "nuto/mechanics/dofSubMatrixStorage/BlockStorageBase.h"

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif // ENABLE_SERIALIZATION


#ifdef ENABLE_SERIALIZATION
//! @brief serializes the class
//! @param ar         archive
//! @param version    version
template void NuTo::BlockStorageBase::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::BlockStorageBase::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::BlockStorageBase::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::BlockStorageBase::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::BlockStorageBase::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::BlockStorageBase::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::BlockStorageBase::serialize(Archive & ar, const unsigned int version)
{
#ifdef DEBUG_SERIALIZATION
    std::cout << "start serialize BlockStorageBase" << "\n";
#endif

#ifdef DEBUG_SERIALIZATION
    std::cout << "finish serialize BlockStorageBase \n";
#endif
}
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::BlockStorageBase)
#endif // ENABLE_SERIALIZATION

