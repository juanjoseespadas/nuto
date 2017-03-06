#pragma once
#include "BoostUnitTest.h"
#include <vector>
#include "mechanics/interpolation/Interpolation.h"

namespace NuTo
{
namespace Test
{

void CheckPartitionOfUnity(const NuTo::Interpolation& r, const std::vector<Eigen::VectorXd>& rPoints)
{
    for (const auto& point : rPoints)
        BOOST_CHECK_CLOSE(r.GetShapeFunctions(point).sum(), 1, 1.e-10);
}

void CheckDerivativeShapeFunctionsCDF(const NuTo::Interpolation& r, const std::vector<Eigen::VectorXd>& rPoints,
                                      double rDelta = 1.e-6)
{
    for (const auto& point : rPoints)
    {
        auto shapeFunctions           = r.GetShapeFunctions(point);
        auto derivativeShapeFunctions = r.GetDerivativeShapeFunctions(point);

        auto cdf = derivativeShapeFunctions;
        cdf.setZero();

        for (int iDim = 0; iDim < point.rows(); ++iDim)
        {
            auto pointDelta = point;
            pointDelta[iDim] += rDelta;
            cdf.col(iDim) = (r.GetShapeFunctions(pointDelta) - shapeFunctions) / rDelta;
        }
        BoostUnitTest::CheckEigenMatrix(cdf, derivativeShapeFunctions, 1.e-6);
    }
}

void CheckShapeFunctionsAndNodePositions(const NuTo::Interpolation& r)
{

    for (int iNode = 0; iNode < r.GetNumNodes(); ++iNode)
    {
        auto localNodeCoordinates = r.GetLocalCoords(iNode);
        auto N                    = r.GetShapeFunctions(localNodeCoordinates);
        for (int i = 0; i < r.GetNumNodes(); ++i)
        {
            if (i == iNode)
                BOOST_CHECK_CLOSE(N[i], 1, 1.e-10);
            else
                BOOST_CHECK_SMALL(N[i], 1.e-10);
        }
    }
}
} /* Test */
} /* NuTo */
