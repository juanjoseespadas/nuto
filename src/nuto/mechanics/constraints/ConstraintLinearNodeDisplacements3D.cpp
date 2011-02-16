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
#include "nuto/mechanics/constraints/ConstraintLinearNodeDisplacements3D.h"
#include "nuto/math/FullMatrix.h"
#include "nuto/math/SparseMatrixCSRGeneral.h"

NuTo::ConstraintLinearNodeDisplacements3D::ConstraintLinearNodeDisplacements3D(const NodeBase* rNode, const NuTo::FullMatrix<double>& rDirection, double rValue) :
        ConstraintNode(rNode), ConstraintLinear()
{
    if (rDirection.GetNumColumns()!=1 || rDirection.GetNumRows()!=3)
        throw MechanicsException("[NuTo::ConstraintLinearNodeDisplacements3D::ConstraintLinearNodeDisplacements3D] Dimension of the direction matrix must be equal to the dimension of the structure.");

    memcpy(mDirection,rDirection.mEigenMatrix.data(),3*sizeof(double));
    //normalize the direction
    double norm = sqrt(mDirection[0]*mDirection[0]+mDirection[1]*mDirection[1]+mDirection[2]*mDirection[2]);
    if (norm < 1e-14)
    {
        throw MechanicsException("[NuTo::ConstraintLinearNodeDisplacements3D::ConstraintLinearNodeDisplacements3D] direction vector has zero length.");
    }
    double invNorm = 1./norm;
    mDirection[0]*=invNorm;
    mDirection[1]*=invNorm;
    mDirection[2]*=invNorm;
    mRHS = rValue;
}

//! @brief returns the number of constraint equations
//! @return number of constraints
int NuTo::ConstraintLinearNodeDisplacements3D::GetNumLinearConstraints()const
{
    return 1;
}

//! @brief adds the constraint equations to the matrix
//! @param curConstraintEquation (is incremented during the function call)
//! @param rConstraintMatrix (the first row where a constraint equation is added is given by curConstraintEquation)
//! @param rRHS right hand side of the constraint equation
void NuTo::ConstraintLinearNodeDisplacements3D::AddToConstraintMatrix(int& curConstraintEquation,
        NuTo::SparseMatrixCSRGeneral<double>& rConstraintMatrix,
        NuTo::FullMatrix<double>& rRHS)const
{
    rRHS(curConstraintEquation,0) = mRHS;
    if (mNode->GetNumDisplacements()!=3)
        throw MechanicsException("[NuTo::ConstraintLinearNodeDisplacements3D::ConstraintBase] Node does not have displacements or has more than one displacement component.");
    if (fabs(mDirection[0])>1e-18)
        rConstraintMatrix.AddEntry(curConstraintEquation,mNode->GetDofDisplacement(0),mDirection[0]);
    if (fabs(mDirection[1])>1e-18)
        rConstraintMatrix.AddEntry(curConstraintEquation,mNode->GetDofDisplacement(1),mDirection[1]);
    if (fabs(mDirection[2])>1e-18)
        rConstraintMatrix.AddEntry(curConstraintEquation,mNode->GetDofDisplacement(2),mDirection[2]);

    curConstraintEquation++;
}

#ifdef ENABLE_SERIALIZATION
// serialize
template void NuTo::ConstraintLinearNodeDisplacements3D::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintLinearNodeDisplacements3D::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintLinearNodeDisplacements3D::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintLinearNodeDisplacements3D::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::ConstraintLinearNodeDisplacements3D::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::ConstraintLinearNodeDisplacements3D::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::ConstraintLinearNodeDisplacements3D::serialize(Archive & ar, const unsigned int version)
{
#ifdef DEBUG_SERIALIZATION
    std::cout << "start serialize ConstraintLinearNodeDisplacements3D" << std::endl;
#endif
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstraintNode)
       & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstraintLinear)
       & BOOST_SERIALIZATION_NVP(mDirection);
#ifdef DEBUG_SERIALIZATION
    std::cout << "finish serialize ConstraintLinearNodeDisplacements3D" << std::endl;
#endif
}
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::ConstraintLinearNodeDisplacements3D)
BOOST_SERIALIZATION_ASSUME_ABSTRACT(NuTo::ConstraintLinearNodeDisplacements3D)
#endif // ENABLE_SERIALIZATION