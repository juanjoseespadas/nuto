#pragma once

#include "mechanics/interpolationtypes/InterpolationBaseIGA.h"

namespace NuTo
{
class Interpolation1DIGA : public InterpolationBaseIGA
{

public:
    Interpolation1DIGA(NuTo::Node::eDof rDofType, NuTo::Interpolation::eTypeOrder rTypeOrder, int rDimension,
                       int rDegree, const Eigen::VectorXd& rKnots, const Eigen::VectorXd& rWeights);

    // --- Integration --- //
    //! @brief determines the standard integration type depending on shape, type and order
    //! @return standard integration type
    eIntegrationType GetStandardIntegrationType() const override;

    //! @brief the spline degree
    int GetSplineDegree(int dir) const override
    {
        assert(dir == 0);
        return mDegree;
    }

    //! @brief return the local dimension of the interpolation
    virtual int GetLocalDimension() const override
    {
        return 1;
    }

    //********************************************
    //       SHAPE FUNCTIONS AND DERIVATIVES
    //********************************************

    // --- shape functions --- //

    //! @brief returns specific shape functions at a parameter, which fits to the knot vector
    //! @param rCoordinates ... parameter
    //! @return ... specific shape functions
    Eigen::VectorXd CalculateShapeFunctions(const Eigen::VectorXd& rCoordinates) const override;

    //! @brief returns specific shape functions at a parameter, whicg fits the knot vector
    //! @param rCoordinates ... parameter
    //! @param rKnotID ... knot id specifying the knot interval the rCoordinates are lying in (no need to search)
    //! @return ... specific shape functions
    Eigen::VectorXd CalculateShapeFunctions(const Eigen::VectorXd& rCoordinates, int rKnotID) const;

    virtual Eigen::VectorXd ShapeFunctionsIGA(const Eigen::VectorXd& naturalCoordinates,
                                              const Eigen::VectorXi& rKnotIDs) const override;

    // --- derivatives shape functions --- //

    //! @brief returns specific derivative shape functions at a parameter, which fits to the knot vector
    //! @param rCoordinates ... parameter
    //! @return ... specific derivative shape functions natural
    Eigen::MatrixXd CalculateDerivativeShapeFunctionsNatural(const Eigen::VectorXd& rCoordinates) const override;

    //! @brief returns specific derivative shape functions at a parameter, which fits to the knot vector
    //! @param rCoordinates ... parameter
    //! @param rKnotID ... knot id specifying the knot interval the rCoordinates are lying in (no need to search)
    Eigen::MatrixXd DerivativeShapeFunctionsNaturalIGA(const Eigen::VectorXd& rCoordinates,
                                                       const Eigen::VectorXi& rKnotIDs) const override;

    // --- N-matrix --- //

    //! @brief returns the N matrix at a parameter, which fits to the knot vector (e.g. 3D: N & 0 & 0 \\ 0 & N & 0 \\ 0
    //! & 0 & N ...)
    //! @param rCoordinates ... parameter
    Eigen::MatrixXd CalculateMatrixN(const Eigen::VectorXd& rCoordinates) const override;

    //! @brief returns the N matrix at a parameter, which fits to the knot vector (e.g. 3D: N & 0 & 0 \\ 0 & N & 0 \\ 0
    //! & 0 & N ...)
    //! @param rCoordinates ... parameter
    //! @param rKnotIDs ... knot ids specifying the knot interval the rCoordinates are lying in (no need to search)
    Eigen::MatrixXd MatrixNIGA(const Eigen::VectorXd& rCoordinates, const Eigen::VectorXi& rKnotIDs) const override;

    //! @brief returns the N matrix at a parameter, which fits to the knot vector (e.g. 3D: N & 0 & 0 \\ 0 & N & 0 \\ 0
    //! & 0 & N ...)
    //! @param rParameters ... parameter on the curve
    //! @param rKnotIDs ... knot span
    //! @param rDerivative ... the order of derivative (only 0,1,2 possible)
    //! @param rDirection ... for 1D only 0 (in 2D 0(x) and 1(y))
    Eigen::MatrixXd MatrixNDerivativeIGA(const Eigen::VectorXd& rParameters, const Eigen::VectorXi& rKnotIDs,
                                         int rDerivative, int rDirection) const override;

    //********************************************
    //       SURFACE PARAMETRIZATION
    //********************************************

    Eigen::VectorXi GetSurfaceNodeIndices(int) const override
    {
        throw Exception(__PRETTY_FUNCTION__, "No needed in 1D!");
    }

    //! @brief returns the natural coordinates of the nodes that span the surface
    //! @param rSurface ... index of the surface, see documentation of the specific InterpolationType
    //! @return ... natural surface edge coordinates
    std::vector<Eigen::VectorXd> GetSurfaceEdgesCoordinates(int) const override
    {
        throw Exception(__PRETTY_FUNCTION__, "For 1D no functionality!");
    }

    //! @brief returns the natural coordinates of the elements surface
    //! @param rNaturalSurfaceCoordinates ... natural surface coordinates
    //! @param rSurface ... index of the surface, see documentation of the specific InterpolationType
    //! @return ... natural coordinates of the elements surface
    Eigen::VectorXd CalculateNaturalSurfaceCoordinatesIGA(const Eigen::VectorXd&, int,
                                                          const Eigen::MatrixXd&) const override
    {
        throw Exception(__PRETTY_FUNCTION__, "For 1D no functionality!");
    }

    //! @brief returns the derivative of the surface parametrization
    //! @param rNaturalSurfaceCoordinates ... natural surface coordinates
    //! @param rSurface ... index of the surface, see documentation of the specific InterpolationType
    //! @return ... derivative of the surface parametrization
    Eigen::MatrixXd CalculateDerivativeNaturalSurfaceCoordinates(const Eigen::VectorXd&, int) const override
    {
        throw Exception(__PRETTY_FUNCTION__, "For 1D no functionality!");
    }

    int GetSurfaceDegree(int) const override
    {
        throw Exception(__PRETTY_FUNCTION__, "For 1D no functionality!");
    }

    //! @brief returns the number of surfaces
    inline int GetNumSurfaces() const override
    {
        return 2;
    }

private:
    //! @brief return the number node depending the shape and the order
    int CalculateNumNodes() const override;

    //********************************************
    //               MEMBERS
    //********************************************

    //! @brief Knot vector
    Eigen::VectorXd mKnots;

    //! @brief weights
    Eigen::MatrixXd mWeights;

    //! @brief polynomial degree
    int mDegree;
};

} /* namespace NuTo */
