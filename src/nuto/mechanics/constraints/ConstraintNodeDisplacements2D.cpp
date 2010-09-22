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
#include "nuto/mechanics/nodes/NodeDisplacements2D.h"
#include "nuto/mechanics/constraints/ConstraintNodeDisplacements2D.h"
#include "nuto/math/FullMatrix.h"
#include "nuto/math/SparseMatrixCSRGeneral.h"

NuTo::ConstraintNodeDisplacements2D::ConstraintNodeDisplacements2D(const NodeBase* rNode, const NuTo::FullMatrix<double>& rDirection, double rValue) :
        ConstraintNode(rNode)
{
    if (rDirection.GetNumColumns()!=1 || rDirection.GetNumRows()!=2)
        throw MechanicsException("[NuTo::ConstraintNodeDisplacements2D::ConstraintNodeDisplacements2D] Dimension of the direction matrix must be equal to the dimension of the structure.");

    memcpy(mDirection,rDirection.mEigenMatrix.data(),2*sizeof(double));
    //normalize the direction
    double norm = sqrt(mDirection[0]*mDirection[0]+mDirection[1]*mDirection[1]);
    if (norm < 1e-14)
    {
        throw MechanicsException("[NuTo::ConstraintNodeDisplacements2D::ConstraintNodeDisplacements2D] direction vector has zero length.");
    }
    double invNorm = 1./norm;
    mDirection[0]*=invNorm;
    mDirection[1]*=invNorm;
    mValue = rValue;
}

//!@brief sets/modifies the right hand side of the constraint equations
//!@param rRHS new right hand side
void NuTo::ConstraintNodeDisplacements2D::SetRHS(double rRHS)
{
    mValue = rRHS;
}

//! @brief adds the constraint equations to the matrix
//! @param curConstraintEquation (is incremented during the function call)
//! @param rConstraintMatrix (the first row where a constraint equation is added is given by curConstraintEquation)
//! @param rRHS right hand side of the constraint equation
void NuTo::ConstraintNodeDisplacements2D::AddToConstraintMatrix(int& curConstraintEquation,
        NuTo::SparseMatrixCSRGeneral<double>& rConstraintMatrix,
        NuTo::FullMatrix<double>& rRHS)const
{
    rRHS(curConstraintEquation,0) = mValue;
    if (mNode->GetNumDisplacements()!=2)
        throw MechanicsException("[NuTo::ConstraintNodeDisplacements2D::ConstraintBase] Node does not have displacements or has more than one displacement component.");
    if (fabs(mDirection[0])>1e-18)
        rConstraintMatrix.AddEntry(curConstraintEquation,mNode->GetDofDisplacement(0),mDirection[0]);
    if (fabs(mDirection[1])>1e-18)
        rConstraintMatrix.AddEntry(curConstraintEquation,mNode->GetDofDisplacement(1),mDirection[1]);

    curConstraintEquation++;
}


#ifdef ENABLE_SERIALIZATION
// serializes the class
template void NuTo::ConstraintNodeDisplacements2D::serialize(boost::archive::binary_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintNodeDisplacements2D::serialize(boost::archive::xml_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintNodeDisplacements2D::serialize(boost::archive::text_oarchive & ar, const unsigned int version);
template void NuTo::ConstraintNodeDisplacements2D::serialize(boost::archive::binary_iarchive & ar, const unsigned int version);
template void NuTo::ConstraintNodeDisplacements2D::serialize(boost::archive::xml_iarchive & ar, const unsigned int version);
template void NuTo::ConstraintNodeDisplacements2D::serialize(boost::archive::text_iarchive & ar, const unsigned int version);
template<class Archive>
void NuTo::ConstraintNodeDisplacements2D::serialize(Archive & ar, const unsigned int version)
{
#ifdef DEBUG_SERIALIZATION
    std::cout << "start serialize ConstraintNodeDisplacements2D" << std::endl;
#endif
    ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ConstraintNode)
       & BOOST_SERIALIZATION_NVP(mValue)
       & BOOST_SERIALIZATION_NVP(mDirection);
#ifdef DEBUG_SERIALIZATION
    std::cout << "finish serialize ConstraintNodeDisplacements2D" << std::endl;
#endif
}
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::ConstraintNodeDisplacements2D)
#endif // ENABLE_SERIALIZATION
