// $Id: $

#include "nuto/mechanics/elements/Tetrahedron10N.h"
#include "nuto/mechanics/nodes/NodeBase.h"
#include "nuto/mechanics/constitutive/ConstitutiveTangentLocal6x6.h"
#include "nuto/mechanics/constitutive/mechanics/EngineeringStress3D.h"
#include <assert.h>

NuTo::Tetrahedron10N::Tetrahedron10N(NuTo::StructureBase* rStructure, std::vector<NuTo::NodeBase* >& rNodes,
		ElementData::eElementDataType rElementDataType, IpData::eIpDataType rIpDataType) :
        NuTo::Solid::Solid(rStructure, rElementDataType, GetStandardIntegrationType(),rIpDataType)
{
    if (rNodes.size()!=10)
        throw MechanicsException("[NuTo::Tetrahedron10N::Tetrahedron10N] Exactly ten nodes are required for this type of element.");
    mNodes[0] = rNodes[0];
    mNodes[1] = rNodes[1];
    mNodes[2] = rNodes[2];
    mNodes[3] = rNodes[3];
    mNodes[4] = rNodes[4];
    mNodes[5] = rNodes[5];
    mNodes[6] = rNodes[6];
    mNodes[7] = rNodes[7];
    mNodes[8] = rNodes[8];
    mNodes[9] = rNodes[9];
}


//! @brief calculates the shape functions
//! @param rLocalCoordinates local coordinates of the integration point
//! @param shape functions for all the nodes
void NuTo::Tetrahedron10N::CalculateShapeFunctions(const double rLocalCoordinates[3], std::vector<double>& rShapeFunctions)const
{
    assert(rShapeFunctions.size()==10);
    double r,s,t;

    r  =rLocalCoordinates[0];
    s  =rLocalCoordinates[1];
    t  =rLocalCoordinates[2];

    // node 1 (0,0,0)
    rShapeFunctions[0] =1.-3.*(r+s+t)+2.*(r*r+s*s+t*t)+4.*(r*s+r*t+s*t);

    // node 2 (1,0,0)
    rShapeFunctions[1] =-r+2.*r*r;

    // node 3 (0,1,0)
    rShapeFunctions[2] =-s+2.*s*s;

    // node 4 (0,0,1)
    rShapeFunctions[3] =-t+2.*t*t;

    // node 5 (0.5,0,0)
    rShapeFunctions[4] =4.*r*(1-r-s-t);

    // node 6 (0.5,0.5,0)
    rShapeFunctions[5] =4.*r*s;

    // node 7 (0,0.5,0)
    rShapeFunctions[6] =4.*s*(1-r-s-t);

    // node 8 (0,0,0.5)
    rShapeFunctions[7] =4.*t*(1-r-s-t);

    // node 9 (0.5,0,0.5)
    rShapeFunctions[8] =4.*r*t;

    // node 10 (0,0.5,0.5)
    rShapeFunctions[9] =4.*s*t;
}

//! @brief calculates the derivative of the shape functions with respect to local coordinates
//! @param rLocalCoordinates local coordinates of the integration point
//! @param derivative of the shape functions for all the nodes,
//! first all the directions for a single node, and then for the next node
void NuTo::Tetrahedron10N::CalculateDerivativeShapeFunctionsLocal(const double rLocalCoordinates[3], std::vector<double>& rDerivativeShapeFunctions)const
{
    assert(rDerivativeShapeFunctions.size()==30);
    double r,s,t;

    r  =rLocalCoordinates[0];
    s  =rLocalCoordinates[1];
    t  =rLocalCoordinates[2];

    //node1
    rDerivativeShapeFunctions[0]  = -3.+4.*(r+s+t);
    rDerivativeShapeFunctions[1]  = -3.+4.*(s+r+t);
    rDerivativeShapeFunctions[2]  = -3.+4.*(t+r+s);

    //node2
    rDerivativeShapeFunctions[3]  = -1.+4.*r;
    rDerivativeShapeFunctions[4]  =  0;
    rDerivativeShapeFunctions[5]  =  0;
    //node3
    rDerivativeShapeFunctions[6]  =  0;
    rDerivativeShapeFunctions[7]  = -1.+4.*s;
    rDerivativeShapeFunctions[8]  =  0;
    //node4
    rDerivativeShapeFunctions[9]  =  0;
    rDerivativeShapeFunctions[10] =  0;
    rDerivativeShapeFunctions[11] = -1.+4.*t ;
    //node5
    rDerivativeShapeFunctions[12] =  4.-8.*r-4.*s-4.*t;
    rDerivativeShapeFunctions[13] = -4.*r;
    rDerivativeShapeFunctions[14] = -4.*r;
    //node6
    rDerivativeShapeFunctions[15] =  4.*s;
    rDerivativeShapeFunctions[16] =  4.*r;
    rDerivativeShapeFunctions[17] =  0.;
    //node7
    rDerivativeShapeFunctions[18] = -4.*s ;
    rDerivativeShapeFunctions[19] =  4.-8.*s-4.*r-4.*t;
    rDerivativeShapeFunctions[20] = -4.*s;
    //node8
    rDerivativeShapeFunctions[21] = -4.*t;
    rDerivativeShapeFunctions[22] = -4.*t;
    rDerivativeShapeFunctions[23] =  4.-8.*t-4.*r-4.*s;
    //node9
    rDerivativeShapeFunctions[24] =  4.*t;
    rDerivativeShapeFunctions[25] =  0;
    rDerivativeShapeFunctions[26] =  4.*r;
    //node10
    rDerivativeShapeFunctions[27] =  0.;
    rDerivativeShapeFunctions[28] =  4.*t;
    rDerivativeShapeFunctions[29] =  4.*s;
}



//! @brief returns the enum of the standard integration type for this element
NuTo::IntegrationType::eIntegrationType NuTo::Tetrahedron10N::GetStandardIntegrationType()
{
    return NuTo::IntegrationType::IntegrationType3D4NGauss1Ip;
}


//! @brief reorder element nodes
void NuTo::Tetrahedron10N::ReorderNodes()
{
    // swap nodes 2 and 3
    NodeBase* tmp = this->mNodes[1];
    this->mNodes[1] = this->mNodes[2];
    this->mNodes[2] = tmp;

    // swap nodes 6 and 7
    tmp = this->mNodes[5];
    this->mNodes[5] = this->mNodes[6];
    this->mNodes[6] = tmp;

    // swap nodes 9 and 10
    tmp = this->mNodes[8];
    this->mNodes[8] = this->mNodes[9];
    this->mNodes[9] = tmp;
}