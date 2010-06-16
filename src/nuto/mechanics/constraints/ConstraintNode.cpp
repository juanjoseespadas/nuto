// $Id$

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif  // ENABLE_SERIALIZATION

#include "nuto/mechanics/nodes/NodeBase.h"
#include "nuto/mechanics/constraints/ConstraintNode.h"


//! @brief constructor
NuTo::ConstraintNode::ConstraintNode(const NodeBase* rNode) : ConstraintBase(), mNode(rNode)
{}

//! @brief returns the number of constraint equations
//! @return number of constraints
int NuTo::ConstraintNode::GetNumConstraintEquations()const
{
    return 1;
}

#ifdef ENABLE_SERIALIZATION
// serialize
template void NuTo::ConstraintNode::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintNode::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintNode::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintNode::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::ConstraintNode::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::ConstraintNode::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::ConstraintNode::serialize(Archive & ar, const unsigned int version)
{
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstraintBase)
    & BOOST_SERIALIZATION_NVP(const_cast<NodeBase*&>(mNode));
}
#endif // ENABLE_SERIALIZATION