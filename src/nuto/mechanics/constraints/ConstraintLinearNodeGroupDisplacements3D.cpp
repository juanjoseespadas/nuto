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
#include "nuto/mechanics/nodes/NodeDisplacements3D.h"
#include "nuto/mechanics/groups/Group.h"
#include "nuto/mechanics/constraints/ConstraintLinearNodeGroupDisplacements3D.h"
#include "nuto/math/FullMatrix.h"
#include "nuto/math/SparseMatrixCSRGeneral.h"

//! @brief constructor
NuTo::ConstraintLinearNodeGroupDisplacements3D::ConstraintLinearNodeGroupDisplacements3D(const Group<NodeBase>* rGroup, const NuTo::FullMatrix<double>& rDirection, double rValue) :
        ConstraintNodeGroup(rGroup), ConstraintLinear()
{
    if (rDirection.GetNumColumns()!=1 || rDirection.GetNumRows()!=3)
        throw MechanicsException("[NuTo::ConstraintLinearNodeGroupDisplacements3D::ConstraintLinearNodeGroupDisplacements3D] Dimension of the direction matrix must be equal to the dimension of the structure.");

    memcpy(mDirection,rDirection.mEigenMatrix.data(),3*sizeof(double));
    //normalize the direction
    double norm = sqrt(mDirection[0]*mDirection[0]+mDirection[1]*mDirection[1]+mDirection[2]*mDirection[2]);
    if (norm < 1e-14)
    {
        throw MechanicsException("[NuTo::ConstraintLinearNodeGroupDisplacements3D::ConstraintLinearNodeGroupDisplacements3D] direction vector has zero length.");
    }
    double invNorm = 1./norm;
    mDirection[0]*=invNorm;
    mDirection[1]*=invNorm;
    mDirection[2]*=invNorm;
    mRHS = rValue;
}

//! @brief returns the number of constraint equations
//! @return number of constraints
int NuTo::ConstraintLinearNodeGroupDisplacements3D::GetNumLinearConstraints()const
{
    return mGroup->GetNumMembers();
}

//! @brief adds the constraint equations to the matrix
//! @param curConstraintEquation (is incremented during the function call)
//! @param rConstraintMatrix (the first row where a constraint equation is added is given by curConstraintEquation)
//! @param rRHS right hand side of the constraint equation
void NuTo::ConstraintLinearNodeGroupDisplacements3D::AddToConstraintMatrix(int& curConstraintEquation,
        NuTo::SparseMatrixCSRGeneral<double>& rConstraintMatrix,
        NuTo::FullMatrix<double>& rRHS)const
{
    for (Group<NodeBase>::const_iterator itNode=mGroup->begin(); itNode!=mGroup->end(); itNode++)
    {
        rRHS(curConstraintEquation,0) = mRHS;
        if ((*itNode)->GetNumDisplacements()==0)
            throw MechanicsException("[NuTo::ConstraintLinearNodeGroupDisplacements3D::AddToConstraintMatrix] Node does not have displacements or has more than three displacement components.");

        if (fabs(mDirection[0])>1e-18)
            rConstraintMatrix.AddEntry(curConstraintEquation,(*itNode)->GetDofDisplacement(0),mDirection[0]);
        if (fabs(mDirection[1])>1e-18)
            rConstraintMatrix.AddEntry(curConstraintEquation,(*itNode)->GetDofDisplacement(1),mDirection[1]);
        if (fabs(mDirection[2])>1e-18)
            rConstraintMatrix.AddEntry(curConstraintEquation,(*itNode)->GetDofDisplacement(2),mDirection[2]);

        curConstraintEquation++;
    }
}

#ifdef ENABLE_SERIALIZATION
// serialize
template void NuTo::ConstraintLinearNodeGroupDisplacements3D::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintLinearNodeGroupDisplacements3D::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintLinearNodeGroupDisplacements3D::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintLinearNodeGroupDisplacements3D::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::ConstraintLinearNodeGroupDisplacements3D::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::ConstraintLinearNodeGroupDisplacements3D::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::ConstraintLinearNodeGroupDisplacements3D::serialize(Archive & ar, const unsigned int version)
{
#ifdef DEBUG_SERIALIZATION
    std::cout << "start serialize ConstraintLinearNodeGroupDisplacements3D" << std::endl;
#endif
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstraintNodeGroup)
    & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstraintLinear)
    & BOOST_SERIALIZATION_NVP(mDirection);
#ifdef DEBUG_SERIALIZATION
    std::cout << "finish serialize ConstraintLinearNodeGroupDisplacements3D" << std::endl;
#endif
}
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::ConstraintLinearNodeGroupDisplacements3D)
#endif // ENABLE_SERIALIZATION