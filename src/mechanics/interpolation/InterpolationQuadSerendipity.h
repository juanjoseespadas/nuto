#pragma once
#include "mechanics/interpolation/InterpolationSimple.h"
#include "mechanics/elements/ElementShapeFunctions.h"

namespace NuTo
{
class InterpolationQuadSerendipity : public InterpolationSimple
{
public:
    std::unique_ptr<InterpolationSimple> Clone() const override
    {
        return std::make_unique<InterpolationQuadSerendipity>(*this);
    }

    Eigen::VectorXd GetShapeFunctions(const NaturalCoords& naturalIpCoords) const override
    {
        return ShapeFunctions2D::ShapeFunctionsQuadOrder2(naturalIpCoords);
    }

    DerivativeShapeFunctionsNatural GetDerivativeShapeFunctions(const NaturalCoords& naturalIpCoords) const override
    {
        return ShapeFunctions2D::DerivativeShapeFunctionsQuadOrder2(naturalIpCoords);
    }

    NaturalCoords GetLocalCoords(int nodeId) const override
    {
        return ShapeFunctions2D::NodeCoordinatesQuadOrder2(nodeId);
    }

    int GetNumNodes() const override
    {
        return 8;
    }
};
} /* NuTo */
