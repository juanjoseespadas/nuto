/*
 * InterpolationBaseFEM.h
 *
 *  Created on: 17 Mar 2015
 *      Author: ttitsche
 */

#pragma once

#include "math/NaturalCoordinateMemoizer.h"
#include "mechanics/interpolationtypes/InterpolationBase.h"
#include "base/Exception.h"


namespace NuTo
{

//! @brief this class stores the information of the interpolation of a single dof type
//! @remark the API only allows const access to this class via the InterpolationType.Get(dofType)
//! method. Its data members are set via the friend class property.
class InterpolationBaseFEM : public InterpolationBase
{
    friend class InterpolationType;

public:
    InterpolationBaseFEM(NuTo::Node::eDof rDofType, NuTo::Interpolation::eTypeOrder rTypeOrder, int rDimension);

    virtual ~InterpolationBaseFEM()
    {
    }

    virtual int GetSplineDegree(int dir) const override;

    void ClearCache() const override;

    //********************************************
    //             NODE METHODS
    //********************************************

    const Eigen::VectorXd& GetNaturalNodeCoordinates(int rNodeIndex) const override;

    virtual Eigen::VectorXd CalculateNaturalNodeCoordinates(int rNodeIndex) const override = 0;

    void CalculateSurfaceNodeIds() override;

    //********************************************
    //       SHAPE FUNCTIONS
    //********************************************

    const Eigen::VectorXd& ShapeFunctions(const Eigen::VectorXd& naturalCoordinates) const override;

    const Eigen::MatrixXd& MatrixN(const Eigen::VectorXd& naturalCoordinates) const override;


    // --- IGA interpolation--- //

    virtual Eigen::VectorXd ShapeFunctionsIGA(const Eigen::VectorXd&, const Eigen::VectorXi&) const override
    {
        throw Exception(__PRETTY_FUNCTION__, "IGA specific function!");
    }

    virtual Eigen::MatrixXd MatrixNIGA(const Eigen::VectorXd&, const Eigen::VectorXi&) const override
    {
        throw Exception(__PRETTY_FUNCTION__, "IGA specific function!");
    }

    virtual Eigen::MatrixXd MatrixNDerivativeIGA(const Eigen::VectorXd&, const Eigen::VectorXi&, int,
                                                 int) const override
    {
        throw Exception(__PRETTY_FUNCTION__, "So far implemeneted only for IGA!");
    }
    //********************************************
    //       DERIVATIVE SHAPE FUNCTIONS NATURAL
    //********************************************

    const Eigen::MatrixXd& DerivativeShapeFunctionsNatural(const Eigen::VectorXd& naturalCoordinates) const override;


    // --- IGA interpolation--- //

    virtual Eigen::MatrixXd DerivativeShapeFunctionsNaturalIGA(const Eigen::VectorXd&,
                                                               const Eigen::VectorXi&) const override
    {
        throw Exception(__PRETTY_FUNCTION__, "IGA specific function!");
    }

    //********************************************
    //       SURFACE PARAMETRIZATION
    //********************************************

    virtual Eigen::VectorXd CalculateNaturalSurfaceCoordinates(const Eigen::VectorXd& rNaturalSurfaceCoordinates,
                                                               int rSurface) const override = 0;

    Eigen::VectorXd CalculateNaturalSurfaceCoordinatesIGA(const Eigen::VectorXd&, int,
                                                          const Eigen::MatrixXd&) const override
    {
        throw Exception(__PRETTY_FUNCTION__, "IGA specific function!");
    }

    virtual Eigen::MatrixXd
    CalculateDerivativeNaturalSurfaceCoordinates(const Eigen::VectorXd& rNaturalSurfaceCoordinates,
                                                 int rSurface) const override = 0;

    virtual int GetNumSurfaces() const override = 0;

    virtual int GetLocalDimension() const override = 0;

    Eigen::VectorXi GetSurfaceNodeIndices(int) const override
    {
        throw Exception(__PRETTY_FUNCTION__, "IGA specific function!");
    }

    int GetSurfaceDegree(int) const override
    {
        throw Exception(__PRETTY_FUNCTION__, "IGA specific function!");
    }


protected:
    virtual Eigen::VectorXd CalculateShapeFunctions(const Eigen::VectorXd& rCoordinates) const override = 0;

    Eigen::MatrixXd CalculateMatrixN(const Eigen::VectorXd& rCoordinates) const override;

    virtual Eigen::MatrixXd
    CalculateDerivativeShapeFunctionsNatural(const Eigen::VectorXd& rCoordinates) const override = 0;

    virtual std::vector<Eigen::VectorXd> GetSurfaceEdgesCoordinates(int rSurface) const override = 0;

    bool NodeIsOnSurface(int rSurface, const Eigen::VectorXd& rNaturalNodeCoordinate) const override;

    virtual int CalculateNumNodes() const override = 0;

    //! @brief this method sets the mNumDofs, mNumNodes and mNodeIndices members
    //! @remark it should be called from the ctor InterpolationTypeBase()
    //! but uses pure virutal functions. Thus it must be called in the
    //! ctors of the child classes.
    void Initialize();

    //********************************************
    //               MEMBERS
    //********************************************

    // members for each integration point
    std::vector<Eigen::VectorXd> mNodeCoordinates;

    template <typename TResult>
    using Memoizer = NuTo::NaturalCoordinateMemoizerMap<TResult, Eigen::VectorXd>;

    Memoizer<Eigen::VectorXd> mShapeFunctions;
    Memoizer<Eigen::MatrixXd> mMatrixN;
    Memoizer<Eigen::MatrixXd> mDerivativeShapeFunctionsNatural;
};
} /* namespace NuTo */
