#pragma once
#include "mechanics/interpolation/InterpolationSimple.h"

namespace NuTo
{
class InterpolationTriangleLinear : public InterpolationSimple
{
public:
    std::unique_ptr<InterpolationSimple> Clone() const override;

    static Eigen::Matrix<double, 3, 1> ShapeFunctions(const NaturalCoords& naturalIpCoords);

    Eigen::VectorXd GetShapeFunctions(const NaturalCoords& naturalIpCoords) const override;

    static Eigen::Matrix<double, 3, 2> DerivativeShapeFunctions();

    DerivativeShapeFunctionsNatural GetDerivativeShapeFunctions(const NaturalCoords& naturalIpCoords) const override;

    static Eigen::Vector2d LocalCoords(int nodeId);

    NaturalCoords GetLocalCoords(int nodeId) const override;

    int GetNumNodes() const override;
};
} /* NuTo */
