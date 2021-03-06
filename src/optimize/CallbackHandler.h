#pragma once

#include <vector>
#include "base/Exception.h"
#include <Eigen/Core>

namespace NuTo
{

//! @author Joerg F. Unger, ISM
//! @date September 2009
//! @brief Abstract class to handle callback routines
class CallbackHandler
{

public:
    CallbackHandler() = default;
    virtual ~CallbackHandler() = default;

    virtual void SetParameters(const Eigen::MatrixXd& rParameters)
    {
        throw Exception(__PRETTY_FUNCTION__, "SetParameters function not implemented in CallbackHandler object.");
    }

    virtual double Objective() const
    {
        throw Exception(__PRETTY_FUNCTION__, "Objective function not implemented in CallbackHandler object.");
    }

    virtual void Gradient(Eigen::MatrixXd& rGradient) const
    {
        throw Exception(__PRETTY_FUNCTION__, "Gradient function not implemented in CallbackHandler object.");
    }

    virtual void Gradient(std::vector<double>& rValues, std::vector<double>& rGradient) const
    {
        throw Exception(__PRETTY_FUNCTION__, "Gradient function not implemented in CallbackHandler object.");
    }

    virtual void Hessian(Eigen::MatrixXd& rHessian) const
    {
        throw Exception(__PRETTY_FUNCTION__, "Hessian function not implemented in CallbackHandler object.");
    }

    virtual void Hessian(std::vector<double>& rDiagHessian) const
    {
        throw Exception(__PRETTY_FUNCTION__, "Hessian function not implemented in CallbackHandler object.");
    }
};
} // namespace NuTo
