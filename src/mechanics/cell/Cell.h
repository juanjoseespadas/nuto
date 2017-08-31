#pragma once

#include "mechanics/cell/CellInterface.h"
#include <boost/ptr_container/ptr_vector.hpp>
#include "mechanics/interpolation/CellInterpolationBase.h"
#include "mechanics/nodes/DofContainer.h"
#include "mechanics/cell/Integrand.h"
#include "mechanics/integrationtypes/IntegrationTypeBase.h"

namespace NuTo
{
template <int TDim>
class Cell : public CellInterface
{
public:
    Cell(const CellInterpolationBase& coordinateInterpolation, DofContainer<CellInterpolationBase*> cellinterpolation,
         const IntegrationTypeBase& integrationType, const Integrand<TDim>& integrand)
        : mCoordinateInterpolation(coordinateInterpolation)
        , mCellInterpolation(cellinterpolation)
        , mIntegrationType(integrationType)
        , mIntegrand()
    {
        for (int i = 0; i < integrationType.GetNumIntegrationPoints(); i++)
            mIntegrand.push_back(integrand.Clone());
    }

    //! @brief builds the internal gradien
    DofVector<double> Gradient() override
    {
        DofVector<double> gradient;
        CellData cellData(mCellInterpolation);
        for (int iIP = 0; iIP < mIntegrationType.GetNumIntegrationPoints(); ++iIP)
        {
            auto ipCoords = mIntegrationType.GetLocalIntegrationPointCoordinates(iIP);
            auto ipWeight = mIntegrationType.GetIntegrationPointWeight(iIP);
            Jacobian<TDim> jacobian(mCoordinateInterpolation.ExtractNodeValues(),
                                    mCoordinateInterpolation.GetDerivativeShapeFunctions(ipCoords));
            CellIPData<TDim> cellipData(mCellInterpolation, jacobian, ipCoords);
            gradient += mIntegrand[iIP].Gradient(cellData, cellipData) * jacobian.Det() * ipWeight;
        }
        return gradient;
    }

    //! @brief builds the hessian0 matrix
    DofMatrix<double> Hessian0() override
    {
        DofMatrix<double> hessian0;
        CellData cellData(mCellInterpolation);
        for (int iIP = 0; iIP < mIntegrationType.GetNumIntegrationPoints(); ++iIP)
        {
            auto ipCoords = mIntegrationType.GetLocalIntegrationPointCoordinates(iIP);
            auto ipWeight = mIntegrationType.GetIntegrationPointWeight(iIP);
            Jacobian<TDim> jacobian(mCoordinateInterpolation.ExtractNodeValues(),
                                    mCoordinateInterpolation.GetDerivativeShapeFunctions(ipCoords));
            CellIPData<TDim> cellipData(mCellInterpolation, jacobian, ipCoords);
            hessian0 += mIntegrand[iIP].Hessian0(cellData, cellipData) * jacobian.Det() * ipWeight;
        }
        return hessian0;
    }


    DofVector<int> DofNumbering() override
    {
        return DofVector<int>();
    }

    //! @brief Extracts a vector (each IP) of vectors (several IPValues for the same integrion point) of IPValues
    std::vector<std::vector<IPValue>> IPValues() override
    {
        std::vector<std::vector<IPValue>> ipValues;

        CellData cellData(mCellInterpolation);
        for (int iIP = 0; iIP < mIntegrationType.GetNumIntegrationPoints(); ++iIP)
        {
            auto ipCoords = mIntegrationType.GetLocalIntegrationPointCoordinates(iIP);
            Jacobian<TDim> jacobian(mCoordinateInterpolation.ExtractNodeValues(),
                                    mCoordinateInterpolation.GetDerivativeShapeFunctions(ipCoords));
            CellIPData<TDim> cellipData(mCellInterpolation, jacobian, ipCoords);
            ipValues.push_back(mIntegrand[iIP].IPValues(cellData, cellipData));
        }
        return ipValues;
    }

private:
    const CellInterpolationBase& mCoordinateInterpolation;
    DofContainer<CellInterpolationBase*> mCellInterpolation;
    const IntegrationTypeBase& mIntegrationType;
    boost::ptr_vector<Integrand<TDim>> mIntegrand;
};
} /* NuTo */
