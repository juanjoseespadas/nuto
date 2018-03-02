#pragma once
#include <eigen3/Eigen/Core>
#include "mechanics/interpolation/InterpolationSimple.h"
#include "mechanics/elements/ElementShapeFunctions.h"

namespace NuTo
{
class InterpolationTriangleQuadratic : public InterpolationSimple
{
public:
    std::unique_ptr<InterpolationSimple> Clone() const override
    {
        return std::make_unique<InterpolationTriangleQuadratic>(*this);
    }

    Eigen::VectorXd GetShapeFunctions(const NaturalCoords& naturalIpCoords) const override
    {
        return ShapeFunctions2D::ShapeFunctionsTriangleOrder2(naturalIpCoords);
    }

    DerivativeShapeFunctionsNatural GetDerivativeShapeFunctions(const NaturalCoords& naturalIpCoords) const override
    {
        return ShapeFunctions2D::DerivativeShapeFunctionsTriangleOrder2(naturalIpCoords);
    }

    NaturalCoords GetLocalCoords(int nodeId) const override
    {
        return ShapeFunctions2D::NodeCoordinatesTriangleOrder2(nodeId);
    }

    int GetNumNodes() const override
    {
        return 6;
    }
};
} /* NuTo */
