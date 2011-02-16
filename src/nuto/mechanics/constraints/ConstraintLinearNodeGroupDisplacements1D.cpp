// $Id$

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif  // ENABLE_SERIALIZATION

#include "nuto/mechanics/MechanicsException.h"
#include "nuto/mechanics/nodes/NodeDisplacements1D.h"
#include "nuto/mechanics/groups/Group.h"
#include "nuto/mechanics/constraints/ConstraintLinearNodeGroupDisplacements1D.h"
#include "nuto/math/FullMatrix.h"
#include "nuto/math/SparseMatrixCSRGeneral.h"

//! @brief constructor
NuTo::ConstraintLinearNodeGroupDisplacements1D::ConstraintLinearNodeGroupDisplacements1D(const Group<NodeBase>* rGroup, double rDirection, double rValue) :
        ConstraintNodeGroup(rGroup), ConstraintLinear()
{
    // set value
    mRHS = rValue;
}

//! @brief returns the number of constraint equations
//! @return number of constraints
int NuTo::ConstraintLinearNodeGroupDisplacements1D::GetNumLinearConstraints()const
{
    return mGroup->GetNumMembers();
}

//! @brief adds the constraint equations to the matrix
//! @param curConstraintEquation (is incremented during the function call)
//! @param rConstraintMatrix (the first row where a constraint equation is added is given by curConstraintEquation)
//! @param rRHS right hand side of the constraint equation
void NuTo::ConstraintLinearNodeGroupDisplacements1D::AddToConstraintMatrix(int& curConstraintEquation,
        NuTo::SparseMatrixCSRGeneral<double>& rConstraintMatrix,
        NuTo::FullMatrix<double>& rRHS)const
{
    // loop over nodes
    for (Group<NodeBase>::const_iterator itNode=mGroup->begin(); itNode!=mGroup->end(); itNode++)
    {
        // set right hand side value
        rRHS(curConstraintEquation,0) = mRHS;

        // add constraint to constrain matrix
        if ((*itNode)->GetNumDisplacements()!=1)
        {
            throw MechanicsException("[NuTo::ConstraintLinearNodeGroupDisplacements1D::AddToConstraintMatrix] Node does not have displacements or has more than one displacement component.");
        }
        rConstraintMatrix.AddEntry(curConstraintEquation,(*itNode)->GetDofDisplacement(0),1);

        // increase constraint equation number
        curConstraintEquation++;
    }
}

#ifdef ENABLE_SERIALIZATION
// serialize
template void NuTo::ConstraintLinearNodeGroupDisplacements1D::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintLinearNodeGroupDisplacements1D::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintLinearNodeGroupDisplacements1D::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintLinearNodeGroupDisplacements1D::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::ConstraintLinearNodeGroupDisplacements1D::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::ConstraintLinearNodeGroupDisplacements1D::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::ConstraintLinearNodeGroupDisplacements1D::serialize(Archive & ar, const unsigned int version)
{
#ifdef DEBUG_SERIALIZATION
    std::cout << "start serialize ConstraintLinearNodeGroupDisplacements1D" << std::endl;
#endif
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstraintNodeGroup)
       & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstraintLinear);
#ifdef DEBUG_SERIALIZATION
    std::cout << "finish serialize ConstraintLinearNodeGroupDisplacements1D" << std::endl;
#endif
}
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::ConstraintLinearNodeGroupDisplacements1D)
#endif // ENABLE_SERIALIZATION