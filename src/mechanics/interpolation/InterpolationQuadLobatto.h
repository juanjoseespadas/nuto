#pragma once
#include "mechanics/interpolation/InterpolationSimple.h"
#include "mechanics/elements/SpectralShapeFunctions.h"

namespace NuTo
{
class InterpolationQuadLobatto : public InterpolationSimple
{
public:
    InterpolationQuadLobatto(int order)
    {
        mNodes = ShapeFunctions1D::NodeCoordinatesTrussLobatto(order);
    }

    std::unique_ptr<InterpolationSimple> Clone() const override
    {
        return std::make_unique<InterpolationQuadLobatto>(*this);
    }

    Eigen::VectorXd GetShapeFunctions(const NaturalCoords& naturalIpCoords) const override
    {
        return ShapeFunctions2D::ShapeFunctionsQuadLagrange(naturalIpCoords, mNodes);
    }

    DerivativeShapeFunctionsNatural GetDerivativeShapeFunctions(const NaturalCoords& naturalIpCoords) const override
    {
        return ShapeFunctions2D::DerivativeShapeFunctionsQuadLagrange(naturalIpCoords, mNodes);
    }

    NaturalCoords GetLocalCoords(int nodeId) const override
    {
        return ShapeFunctions2D::NodeCoordinatesQuadLobatto(nodeId, mNodes);
    }

    int GetNumNodes() const override
    {
        return mNodes.size() * mNodes.size();
    }

private:
    Eigen::VectorXd mNodes;
};
} /* NuTo */
