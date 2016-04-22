/*
 * ImplicitExplicitBase.h
 *
 *  Created on: 17 Mar 2016
 *      Author: ttitsche
 */

#pragma once

#include "nuto/mechanics/timeIntegration/TimeIntegrationBase.h"
#include "nuto/math/SparseDirectSolverMUMPS.h"


namespace NuTo
{

class ConstitutiveTimeStep;

//! @brief Base class for implicit/explicit time integration schemes like ImplEx and CycleJump
class ImplicitExplicitBase : public TimeIntegrationBase
{
public:

    ImplicitExplicitBase(StructureBase* rStructure);

    virtual ~ImplicitExplicitBase() = default;

    //! @brief perform the time integration
    //! @param rTimeDelta ... length of the simulation
    virtual NuTo::Error::eError Solve(double rTimeDelta) override;

    //! @brief returns true, if the method is only conditionally stable (for unconditional stable, this is false)
    bool HasCriticalTimeStep() const override {
        return false;
    }

    //! @brief calculate the critical time step for explicit routines
    //! for implicit routines, this will simply return zero (cmp HasCriticalTimeStep())
    double CalculateCriticalTimeStep()const override;

    //! @brief ... Adds a dof type with a constant hessian0 matrix
    //! param rDofWithConstantHessian ... dof type
    void AddDofWithConstantHessian0(Node::eDof rDofWithConstantHessian);

protected:

    //! @brief ... assess the solution and return, may modifiy the mTimeStep
    //! @return ... bool : true - accept solution, false - reject solution
    virtual bool CheckExtrapolationAndAdjustTimeStep()
    {
        throw MechanicsException(__PRETTY_FUNCTION__, "not implemented.");
    }

    //! @brief ... extrapolates the static data
    virtual void ExtrapolateStaticData(const ConstitutiveTimeStep& rTimeStep)
    {
        throw MechanicsException(__PRETTY_FUNCTION__, "not implemented.");
    }


private:

    std::set<Node::eDof> mDofsWithConstantHessian;

    void FactorizeConstantHessians(std::map<Node::eDof, SparseDirectSolverMUMPS>& rPreFactorizedHessians);
};

} /* namespace NuTo */
