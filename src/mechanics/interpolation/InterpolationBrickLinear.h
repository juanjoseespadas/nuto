#pragma once
#include "mechanics/interpolation/InterpolationSimple.h"
#include "mechanics/elements/ElementShapeFunctions.h"

namespace NuTo
{
class InterpolationBrickLinear : public InterpolationSimple
{
public:
    std::unique_ptr<InterpolationSimple> Clone() const override
    {
        return std::make_unique<InterpolationBrickLinear>(*this);
    }

    Eigen::VectorXd GetShapeFunctions(const NaturalCoords& naturalIpCoords) const override
    {
        return ShapeFunctions3D::ShapeFunctionsBrickOrder1(naturalIpCoords);
    }

    DerivativeShapeFunctionsNatural GetDerivativeShapeFunctions(const NaturalCoords& naturalIpCoords) const override
    {
        return ShapeFunctions3D::DerivativeShapeFunctionsBrickOrder1(naturalIpCoords);
    }

    NaturalCoords GetLocalCoords(int nodeId) const override
    {
        return ShapeFunctions3D::NodeCoordinatesBrickOrder1(nodeId);
    }

    int GetNumNodes() const override
    {
        return 8;
    }
};
} /* NuTo */
