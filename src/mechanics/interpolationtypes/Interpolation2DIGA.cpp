#include "base/Exception.h"
#include "mechanics/elements/ElementShapeFunctions.h"
#include "mechanics/integrationtypes/IntegrationTypeEnum.h"
#include "mechanics/interpolationtypes/InterpolationTypeEnum.h"
#include "mechanics/interpolationtypes/Interpolation2DIGA.h"
#include "mechanics/nodes/NodeEnum.h"

NuTo::Interpolation2DIGA::Interpolation2DIGA(NuTo::Node::eDof rDofType, NuTo::Interpolation::eTypeOrder rTypeOrder,
                                             int rDimension, const Eigen::Vector2i& rDegree,
                                             const Eigen::VectorXd& rKnotsX, const Eigen::VectorXd& rKnotsY,
                                             const Eigen::MatrixXd& rWeights)
    : InterpolationBaseIGA::InterpolationBaseIGA(rDofType, rTypeOrder, rDimension)
    , mDegree(rDegree)
    , mKnotsX(rKnotsX)
    , mKnotsY(rKnotsY)
    , mWeights(rWeights)

{
    Initialize();
}

NuTo::eIntegrationType NuTo::Interpolation2DIGA::GetStandardIntegrationType() const
{
    // its tensor product
    switch (std::max(mDegree(0), mDegree(1)))
    {
    case 0: // (0+0+1)/2 = 0.5 ips
        return NuTo::eIntegrationType::IntegrationType2D4NGauss1Ip;
    case 1: // (1+1+1)/2 = 1.5 ips or (1+1+3)/2 = 2.5 ips lobatto
        return NuTo::eIntegrationType::IntegrationType2D4NGauss4Ip;
    case 2: // (2+2+1)/2 = 2.5 ips or (2+2+3)/2 = 3.5 ips lobatto
        return NuTo::eIntegrationType::IntegrationType2D4NGauss9Ip;
    case 3: // (3+3+1)/2 = 3.5 ips or (3+3+3)/2 = 4.5 ips lobatto
        return NuTo::eIntegrationType::IntegrationType2D4NLobatto25Ip;
    default:
        throw Exception(__PRETTY_FUNCTION__, "Interpolation for exact integration of " +
                                                     std::to_string(std::max(mDegree(0), mDegree(1))) +
                                                     " IGA not implemented");
    }
}

std::vector<Eigen::VectorXd> NuTo::Interpolation2DIGA::GetSurfaceEdgesCoordinates(int) const
{
    throw Exception(__PRETTY_FUNCTION__, "Not implemented yet");
}

/*******************/
// Shape functions //
/*******************/

Eigen::VectorXd NuTo::Interpolation2DIGA::CalculateShapeFunctions(const Eigen::VectorXd& rCoordinates) const
{
    Eigen::Vector2i spanIdx;
    spanIdx(0) = ShapeFunctionsIGA::FindSpan(rCoordinates(0), mDegree(0), mKnotsX);
    spanIdx(1) = ShapeFunctionsIGA::FindSpan(rCoordinates(1), mDegree(1), mKnotsY);

    return ShapeFunctionsIGA::BasisFunctionsAndDerivatives2DRat(0, rCoordinates, spanIdx, mDegree, mKnotsX, mKnotsY,
                                                                mWeights);
}


Eigen::VectorXd NuTo::Interpolation2DIGA::CalculateShapeFunctions(const Eigen::VectorXd& rCoordinates,
                                                                  const Eigen::Vector2i& rKnotIDs) const
{
    assert(rKnotIDs.rows() == 2);
    assert(rKnotIDs(0) < mKnotsX.rows());
    assert(rKnotIDs(1) < mKnotsY.rows());

    Eigen::Vector2d parameter;

    parameter(0) = transformation(rCoordinates(0), mKnotsX(rKnotIDs(0)), mKnotsX(rKnotIDs(0) + 1));
    parameter(1) = transformation(rCoordinates(1), mKnotsY(rKnotIDs(1)), mKnotsY(rKnotIDs(1) + 1));

    return ShapeFunctionsIGA::BasisFunctionsAndDerivatives2DRat(0, parameter, rKnotIDs, mDegree, mKnotsX, mKnotsY,
                                                                mWeights);
}

Eigen::VectorXd NuTo::Interpolation2DIGA::ShapeFunctionsIGA(const Eigen::VectorXd& naturalCoordinates,
                                                            const Eigen::VectorXi& rKnotIDs) const
{
    return CalculateShapeFunctions(naturalCoordinates, rKnotIDs);
}

/***************/
// Derivatives //
/***************/
Eigen::MatrixXd
NuTo::Interpolation2DIGA::CalculateDerivativeShapeFunctionsNatural(const Eigen::VectorXd& rCoordinates) const
{
    Eigen::Vector2i spanIdx;
    spanIdx(0) = ShapeFunctionsIGA::FindSpan(rCoordinates(0), mDegree(0), mKnotsX);
    spanIdx(1) = ShapeFunctionsIGA::FindSpan(rCoordinates(1), mDegree(1), mKnotsY);

    return ShapeFunctionsIGA::BasisFunctionsAndDerivatives2DRat(1, rCoordinates, spanIdx, mDegree, mKnotsX, mKnotsY,
                                                                mWeights);
}

Eigen::MatrixXd NuTo::Interpolation2DIGA::DerivativeShapeFunctionsNaturalIGA(const Eigen::VectorXd& rCoordinates,
                                                                             const Eigen::VectorXi& rKnotIDs) const
{
    assert(rKnotIDs.rows() == 2);
    assert(rKnotIDs(0) < mKnotsX.rows());
    assert(rKnotIDs(1) < mKnotsY.rows());

    Eigen::Vector2d parameter;

    parameter(0) = transformation(rCoordinates(0), mKnotsX(rKnotIDs(0)), mKnotsX(rKnotIDs(0) + 1));
    parameter(1) = transformation(rCoordinates(1), mKnotsY(rKnotIDs(1)), mKnotsY(rKnotIDs(1) + 1));

    return ShapeFunctionsIGA::BasisFunctionsAndDerivatives2DRat(1, parameter, rKnotIDs, mDegree, mKnotsX, mKnotsY,
                                                                mWeights);
}

/***************/
// N-Matrix    //
/***************/
Eigen::MatrixXd NuTo::Interpolation2DIGA::CalculateMatrixN(const Eigen::VectorXd& rCoordinates) const
{
    auto shapeFunctions = CalculateShapeFunctions(rCoordinates);

    return ConstructMatrixN(shapeFunctions);
}

Eigen::MatrixXd NuTo::Interpolation2DIGA::MatrixNIGA(const Eigen::VectorXd& rCoordinates,
                                                     const Eigen::VectorXi& rKnotIDs) const
{
    assert(rKnotIDs.rows() == 2);
    assert(rKnotIDs(0) < mKnotsX.rows());
    assert(rKnotIDs(1) < mKnotsY.rows());

    Eigen::Vector2d parameter;

    parameter(0) = transformation(rCoordinates(0), mKnotsX(rKnotIDs(0)), mKnotsX(rKnotIDs(0) + 1));
    parameter(1) = transformation(rCoordinates(1), mKnotsY(rKnotIDs(1)), mKnotsY(rKnotIDs(1) + 1));

    return ConstructMatrixN(ShapeFunctionsIGA::BasisFunctionsAndDerivatives2DRat(0, parameter, rKnotIDs, mDegree,
                                                                                 mKnotsX, mKnotsY, mWeights));
}

Eigen::MatrixXd NuTo::Interpolation2DIGA::MatrixNDerivativeIGA(const Eigen::VectorXd& rParameters,
                                                               const Eigen::VectorXi& rKnotIDs, int rDerivative,
                                                               int rDirection) const
{
    assert(rDerivative >= 0 && rDerivative <= 2);
    assert(rKnotIDs(0) < mKnotsX.rows());
    assert(rKnotIDs(1) < mKnotsY.rows());

    Eigen::MatrixXd shapeFunctions;

    switch (rDerivative)
    {
    case 0:
        shapeFunctions = ShapeFunctionsIGA::BasisFunctionsAndDerivatives2DRat(rDerivative, rParameters, rKnotIDs,
                                                                              mDegree, mKnotsX, mKnotsY, mWeights)
                                 .col(0);
        break;
    case 1:
    {
        if (rDirection == 0) // d/dx
        {
            shapeFunctions = ShapeFunctionsIGA::BasisFunctionsAndDerivatives2DRat(rDerivative, rParameters, rKnotIDs,
                                                                                  mDegree, mKnotsX, mKnotsY, mWeights)
                                     .col(0);
        }
        else if (rDirection == 1) // d/dy
        {
            shapeFunctions = ShapeFunctionsIGA::BasisFunctionsAndDerivatives2DRat(rDerivative, rParameters, rKnotIDs,
                                                                                  mDegree, mKnotsX, mKnotsY, mWeights)
                                     .col(1);
        }
        break;
    }
    case 2:
    {
        if (rDirection == 0) // d²/d²xx
        {
            shapeFunctions = ShapeFunctionsIGA::BasisFunctionsAndDerivatives2DRat(rDerivative, rParameters, rKnotIDs,
                                                                                  mDegree, mKnotsX, mKnotsY, mWeights)
                                     .col(0);
        }
        else if (rDirection == 1) // d²/d²yy
        {
            shapeFunctions = ShapeFunctionsIGA::BasisFunctionsAndDerivatives2DRat(rDerivative, rParameters, rKnotIDs,
                                                                                  mDegree, mKnotsX, mKnotsY, mWeights)
                                     .col(1);
        }
        else if (rDirection == 2) // d²/d²xy = d²/d²yx
        {
            shapeFunctions = ShapeFunctionsIGA::BasisFunctionsAndDerivatives2DRat(rDerivative, rParameters, rKnotIDs,
                                                                                  mDegree, mKnotsX, mKnotsY, mWeights)
                                     .col(2);
        }
        break;
    }
    default:
        throw NuTo::Exception(__PRETTY_FUNCTION__, "Maximum derivative is of order 2!");
        break;
    }

    return ConstructMatrixN(shapeFunctions);
}

Eigen::MatrixXd NuTo::Interpolation2DIGA::ConstructMatrixN(Eigen::VectorXd rShapeFunctions) const
{
    int numNodes = GetNumNodes();
    int dimBlock = NuTo::Node::GetNumComponents(mDofType, 2);

    assert(rShapeFunctions.rows() == (mDegree(0) + 1) * (mDegree(1) + 1));

    Eigen::MatrixXd matrixN(dimBlock, numNodes * dimBlock);
    for (int iNode = 0, iBlock = 0; iNode < numNodes; ++iNode, iBlock += dimBlock)
    {
        matrixN.block(0, iBlock, dimBlock, dimBlock) =
                Eigen::MatrixXd::Identity(dimBlock, dimBlock) * rShapeFunctions(iNode);
    }

    return matrixN;
}

Eigen::VectorXd
NuTo::Interpolation2DIGA::CalculateNaturalSurfaceCoordinatesIGA(const Eigen::VectorXd& rNaturalSurfaceCoordinates,
                                                                int rSurface, const Eigen::MatrixXd& rKnots) const
{
    assert(rNaturalSurfaceCoordinates.rows() == 1);

    switch (rSurface)
    {
    case 0:
        return Eigen::Vector2d(rNaturalSurfaceCoordinates(0), rKnots(1, 0));
    case 1:
        return Eigen::Vector2d(rKnots(0, 1), rNaturalSurfaceCoordinates(0));
    case 2:
        return Eigen::Vector2d(rNaturalSurfaceCoordinates(0), rKnots(1, 1));
    case 3:
        return Eigen::Vector2d(rKnots(0, 0), rNaturalSurfaceCoordinates(0));
    default:
        throw Exception(__PRETTY_FUNCTION__, "IGA2D has exactly four surfaces, 0 to 3. You tried to access " +
                                                     std::to_string(rSurface) + ".");
    }
}

Eigen::MatrixXd NuTo::Interpolation2DIGA::CalculateDerivativeNaturalSurfaceCoordinates(
        const Eigen::VectorXd& rNaturalSurfaceCoordinates, int rSurface) const
{
    assert(rNaturalSurfaceCoordinates.rows() == 1);
    switch (rSurface)
    {
    case 0:
        return Eigen::Vector2d(1, 0);
    case 1:
        return Eigen::Vector2d(0, 1);
    case 2:
        return Eigen::Vector2d(-1, 0);
    case 3:
        return Eigen::Vector2d(0, -1);
    default:
        throw Exception(__PRETTY_FUNCTION__, "IGA2D has exactly four surfaces, 0 to 3. You tried to access " +
                                                     std::to_string(rSurface) + ".");
    }
}

//       2
//    -------
//  3 |     | 1
//    |     |
//    -------
//       0
Eigen::VectorXi NuTo::Interpolation2DIGA::GetSurfaceNodeIndices(int rSurface) const
{
    Eigen::VectorXi indices;

    switch (rSurface)
    {
    case 0:
    {
        indices.resize(mDegree(0) + 1);
        for (int i = 0; i <= mDegree(0); i++)
            indices(i) = i;
        return indices;
    }
    case 1:
    {
        indices.resize(mDegree(1) + 1);
        for (int i = 1; i <= mDegree(1) + 1; i++)
            indices(i - 1) = (mDegree(0) + 1) * i - 1;
        return indices;
    }
    case 2:
    {
        indices.resize(mDegree(0) + 1);
        int count = 0;
        for (int i = CalculateNumNodes() - mDegree(0) - 1; i < CalculateNumNodes(); i++, count++)
            indices(count) = i;
        return indices;
    }
    case 3:
    {
        indices.resize(mDegree(1) + 1);
        for (int i = 0; i <= mDegree(1); i++)
            indices(i) = (mDegree(0) + 1) * i;
        return indices;
    }
    default:
        throw Exception(__PRETTY_FUNCTION__, "IGA2D has exactly four surfaces, 0 to 3. You tried to access " +
                                                     std::to_string(rSurface) + ".");
    }
}

int NuTo::Interpolation2DIGA::GetSurfaceDegree(int rSurface) const
{
    switch (rSurface)
    {
    case 0:
        return mDegree(0);
    case 1:
        return mDegree(1);
    case 2:
        return mDegree(0);
    case 3:
        return mDegree(1);
    default:
        throw Exception(__PRETTY_FUNCTION__, "IGA2D has exactly four surfaces, 0 to 3. You tried to access " +
                                                     std::to_string(rSurface) + ".");
    }
}

int NuTo::Interpolation2DIGA::CalculateNumNodes() const
{
    return (mDegree(0) + 1) * (mDegree(1) + 1);
}
