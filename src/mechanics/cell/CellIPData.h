#pragma once

#include "mechanics/interpolation/CellInterpolationBase.h"
#include "mechanics/nodes/DofContainer.h"
#include "mechanics/cell/Jacobian.h"

namespace NuTo
{


//! @brief Similar to NuTo::CellData, but for N and B
template <int TDim>
class CellIPData
{
public:
    CellIPData(const DofContainer<CellInterpolationBase*> cellInterpolation, const NuTo::Jacobian<TDim>& jacobian,
               const NaturalCoords& ipCoords)
        : mCellInterpolation(cellInterpolation)
        , mJacobian(jacobian)
        , mIPCoords(ipCoords)
    {
    }

    NMatrix GetNMatrix(const DofType& dofType) const
    {
        int dim = mCellInterpolation[dofType]->GetDofDimension();
        Eigen::MatrixXd N(dim, dim * mCellInterpolation[dofType]->GetNumNodes());

        auto shapeFunctions = mCellInterpolation[dofType]->GetShapeFunctions(mIPCoords);

        for (int i = 0; i < mCellInterpolation[dofType]->GetNumNodes(); ++i)
            N.block(0, i * dim, dim, dim) = Eigen::MatrixXd::Identity(dim, dim) * shapeFunctions[i];
        return N;
//        return mCellInterpolation[dofType]->GetInterpolation().GetN(mIPCoords);
    }

    BMatrixGradient GetBMatrixGradient(const DofType& dofType) const
    {
        DerivativeShapeFunctionsGlobal dShapeGlobal = CalculateDerivativeShapeFunctionsGlobal(dofType);
        return dShapeGlobal.transpose();
    }

    BMatrixStrain GetBMatrixStrain(const DofType& dofType) const
    {
        DerivativeShapeFunctionsGlobal dShapeGlobal = CalculateDerivativeShapeFunctionsGlobal(dofType);
        const int dim = dShapeGlobal.cols();
        const int numNodes = dShapeGlobal.rows();
        switch (dim)
        {
        case 1:
            return dShapeGlobal.transpose();
        case 2:
        {
            BMatrixStrain B = Eigen::MatrixXd::Zero(3, numNodes * 2);
            for (int iNode = 0, iColumn = 0; iNode < numNodes; ++iNode, iColumn += 2)
            {
                double dNdX = dShapeGlobal(iNode, 0);
                double dNdY = dShapeGlobal(iNode, 1);

                B(0, iColumn) = dNdX;
                B(1, iColumn + 1) = dNdY;
                B(2, iColumn) = dNdY;
                B(2, iColumn + 1) = dNdX;
            }
            return B;
        }
        case 3:
        {

            BMatrixStrain B = Eigen::MatrixXd::Zero(6, numNodes * 3);

            for (int iNode = 0, iColumn = 0; iNode < numNodes; ++iNode, iColumn += 3)
            {
                double dNdX = dShapeGlobal(iNode, 0);
                double dNdY = dShapeGlobal(iNode, 1);
                double dNdZ = dShapeGlobal(iNode, 2);

                /* according to Jirásek
                 *
                 *     +0  +1  +2
                 *    -------------
                 * 0 |  dx  0   0  |     - e_x
                 * 1 |  0   dy  0  |     - e_y
                 * 2 |  0   0   dz |     - e_z
                 * 3 |  0   dz  dy |     - g_yz
                 * 4 |  dz  0   dx |     - g_xz
                 * 5 |  dy  dx  0  |     - g_xy
                 *    -------------
                 */


                B(0, iColumn) = dNdX;
                B(1, iColumn + 1) = dNdY;
                B(2, iColumn + 2) = dNdZ;

                B(3, iColumn + 1) = dNdZ;
                B(3, iColumn + 2) = dNdY;

                B(4, iColumn) = dNdZ;
                B(4, iColumn + 2) = dNdX;

                B(5, iColumn) = dNdY;
                B(5, iColumn + 1) = dNdX;
            }
            return B;
        }
        default:
            throw Exception(__PRETTY_FUNCTION__, "c'mon.");
        }
    }

private:
    DerivativeShapeFunctionsGlobal CalculateDerivativeShapeFunctionsGlobal(const DofType& dofType) const
    {
        DerivativeShapeFunctionsNatural dShapeNatural =  mCellInterpolation[dofType]->GetDerivativeShapeFunctions(mIPCoords);
        return mJacobian.TransformDerivativeShapeFunctions(dShapeNatural);
    }

    const DofContainer<CellInterpolationBase*> mCellInterpolation;
    const NuTo::Jacobian<TDim>& mJacobian;
    const NaturalCoords& mIPCoords;
};
} /* NuTo */
