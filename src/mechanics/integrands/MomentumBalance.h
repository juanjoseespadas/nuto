#pragma once

#include "mechanics/constitutive/MechanicsInterface.h"
#include "mechanics/dofs/DofType.h"
#include "mechanics/dofs/DofVector.h"
#include "mechanics/dofs/DofMatrix.h"
#include "mechanics/interpolation/TypeDefs.h"
#include "mechanics/cell/CellIpData.h"
#include "mechanics/cell/CellData.h"

namespace NuTo
{
namespace Integrands
{

template <int TDim>
class MomentumBalance
{
public:
    MomentumBalance(DofType dofType, const Laws::MechanicsInterface<TDim>& law)
        : mDofType(dofType)
        , mLaw(law)
    {
    }

    DofVector<double> Gradient(const CellIpData& cellIpData, double deltaT)
    {
        DofVector<double> gradient;

        BMatrixStrain B = cellIpData.B(mDofType, Nabla::Strain());
        gradient[mDofType] =
                B.transpose() * mLaw.Stress(cellIpData.Apply(mDofType, Nabla::Strain()), deltaT, cellIpData.Ids());

        return gradient;
    }

    DofMatrix<double> Hessian0(const CellIpData& cellIpData, double deltaT)
    {
        DofMatrix<double> hessian0;

        BMatrixStrain B = cellIpData.B(mDofType, Nabla::Strain());
        hessian0(mDofType, mDofType) =
                B.transpose() * mLaw.Tangent(cellIpData.Apply(mDofType, Nabla::Strain()), deltaT, cellIpData.Ids()) * B;

        return hessian0;
    }

protected:
    DofType mDofType;

private:
    const Laws::MechanicsInterface<TDim>& mLaw;
};
} /* Integrand */
} /* NuTo */