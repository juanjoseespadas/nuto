#include "base/Exception.h"
#include "InterpolationTriangleLinear.h"

using namespace NuTo;

std::unique_ptr<InterpolationSimple> InterpolationTriangleLinear::Clone() const
{
    return std::make_unique<InterpolationTriangleLinear>(*this);
}

Eigen::VectorXd InterpolationTriangleLinear::GetShapeFunctions(const NaturalCoords& naturalIpCoords) const
{
    Eigen::Matrix<double, 3, 1> shapeFunctions;
    shapeFunctions[0] = 1. - naturalIpCoords(0) - naturalIpCoords(1);
    shapeFunctions[1] = naturalIpCoords(0);
    shapeFunctions[2] = naturalIpCoords(1);
    return shapeFunctions;
}

Eigen::Matrix<double, 3, 2> InterpolationTriangleLinear::DerivativeShapeFunctions()
{
    Eigen::Matrix<double, 3, 2> derivativeShapeFunctions;
    derivativeShapeFunctions(0, 0) = -1.0;
    derivativeShapeFunctions(0, 1) = -1.0;

    derivativeShapeFunctions(1, 0) = 1.0;
    derivativeShapeFunctions(1, 1) = 0.0;

    derivativeShapeFunctions(2, 0) = 0.0;
    derivativeShapeFunctions(2, 1) = 1.0;
    return derivativeShapeFunctions;
}

DerivativeShapeFunctionsNatural InterpolationTriangleLinear::GetDerivativeShapeFunctions(const NaturalCoords&) const
{
    return DerivativeShapeFunctions();
}

Eigen::Vector2d InterpolationTriangleLinear::LocalCoords(int nodeId)
{
    switch (nodeId)
    {
    case 0:
        return Eigen::Vector2d(0.0, 0.0);
    case 1:
        return Eigen::Vector2d(1.0, 0.0);
    case 2:
        return Eigen::Vector2d(0.0, 1.0);
    default:
        throw Exception(__PRETTY_FUNCTION__, "node index out of range (0..2)");
    }
}

NaturalCoords InterpolationTriangleLinear::GetLocalCoords(int nodeId) const
{
    return LocalCoords(nodeId);
}

int InterpolationTriangleLinear::GetNumNodes() const
{
    return 3;
}
