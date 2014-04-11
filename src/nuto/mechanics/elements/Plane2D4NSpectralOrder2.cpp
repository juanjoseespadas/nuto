// $Id: Plane2D4NSpectralOrder2.cpp 647 2013-10-31 13:23:00Z unger3 $

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif // ENABLE_SERIALIZATION

#include <tuple>

#include <assert.h>

#include <boost/foreach.hpp>

#include "nuto/mechanics/MechanicsException.h"
#include "nuto/mechanics/elements/Plane2D4NSpectral.h"
#include "nuto/mechanics/nodes/NodeBase.h"

//! @brief calculates the shape functions
//! @param rLocalCoordinates local coordinates of the integration point
//! @param shape functions for all the nodes
namespace NuTo
{
template<>
void NuTo::Plane2D4NSpectral<2>::CalculateShapeFunctionsField1D(double rNaturalCoordinate, std::vector<double>& rShapeFunctions)const
{
	assert(((int)rShapeFunctions.size())==GetNumNodesField1D());
	rShapeFunctions[0] = 0.5*rNaturalCoordinate*(-1.+rNaturalCoordinate);
	rShapeFunctions[1] = 1.-(rNaturalCoordinate*rNaturalCoordinate);
	rShapeFunctions[2] = 0.5*rNaturalCoordinate*(1.+rNaturalCoordinate);
}

//! @brief calculates the derivative of the shape functions
//! @param rLocalCoordinates local coordinates of the integration point
//! @param derivative of the shape functions for all the nodes,
//! first all the directions for a single node, and then for the next node
template<>
void NuTo::Plane2D4NSpectral<2>::CalculateDerivativeShapeFunctionsFieldNatural1D(double rNaturalCoordinate, std::vector<double>& rDerivativeShapeFunctions)const
{
	assert(((int)rDerivativeShapeFunctions.size())==GetNumNodesField1D());
	rDerivativeShapeFunctions[0] = -0.5+rNaturalCoordinate;
	rDerivativeShapeFunctions[1] = -2.*rNaturalCoordinate;
	rDerivativeShapeFunctions[2] = 0.5+rNaturalCoordinate;
}


//! @brief returns the enum of the standard integration type for this element
template<>
NuTo::IntegrationType::eIntegrationType NuTo::Plane2D4NSpectral<2>::GetStandardIntegrationType()
{
    return NuTo::IntegrationType::IntegrationType2D4NLobatto9Ip;
}

//! @brief returns the enum (type of the element)
//! @return enum
template<>
NuTo::Element::eElementType NuTo::Plane2D4NSpectral<2>::GetEnumType()const
{
    return NuTo::Element::PLANE2D4NSPECTRALORDER2;
}

} //end namespace nuto

#ifdef ENABLE_SERIALIZATION
BOOST_CLASS_EXPORT_IMPLEMENT(NuTo::Plane2D4NSpectral<2>)
#endif // ENABLE_SERIALIZATION