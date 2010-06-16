// $Id: $

#include "nuto/mechanics/integrationtypes/IntegrationType2D4NGauss4Ip.h"
#include <assert.h>


//! @brief constructor
NuTo::IntegrationType2D4NGauss4Ip::IntegrationType2D4NGauss4Ip()
{
}

//! @brief returns the local coordinates of an integration point
//! @param rIpNum integration point (counting from zero)
//! @param rCoordinates (result)
void NuTo::IntegrationType2D4NGauss4Ip::GetLocalIntegrationPointCoordinates2D(int rIpNum, double rCoordinates[2])const
{
    assert(rIpNum>=0 && rIpNum<4);
    switch (rIpNum)
    {
    case 0 :
        rCoordinates[0] = -0.577350269189626;
        rCoordinates[1] = -0.577350269189626;
        break;
    case 1 :
        rCoordinates[0] = +0.577350269189626;
        rCoordinates[1] = -0.577350269189626;
        break;
    case 2 :
        rCoordinates[0] = +0.577350269189626;
        rCoordinates[1] = +0.577350269189626;
        break;
    case 3 :
        rCoordinates[0] = -0.577350269189626;
        rCoordinates[1] = +0.577350269189626;
        break;
    default:
        throw MechanicsException("[NuTo::IntegrationType2D4NGauss4Ip::GetLocalIntegrationPointCoordinates] Ip number out of range.");
    }
}


//! @brief returns the total number of integration points for this integration type
//! @return number of integration points
int NuTo::IntegrationType2D4NGauss4Ip::GetNumIntegrationPoints()const
{
    return 4;
}

//! @brief returns the weight of an integration point
//! @param rIpNum integration point (counting from zero)
//! @return weight of integration points
double NuTo::IntegrationType2D4NGauss4Ip::GetIntegrationPointWeight(int rIpNum)const
{
    return 1;
}

//! @brief returns a string with the identifier of the integration type
//! @return identifier
std::string NuTo::IntegrationType2D4NGauss4Ip::GetStrIdentifier()const
{
    return GetStrIdentifierStatic();
}

//! @brief returns a string with the identifier of the integration type
//! @return identifier
std::string NuTo::IntegrationType2D4NGauss4Ip::GetStrIdentifierStatic()
{
    return std::string("2D4NGAUSS4IP");
}

#ifdef ENABLE_VISUALIZE
void NuTo::IntegrationType2D4NGauss4Ip::GetVisualizationCells(
    unsigned int& NumVisualizationPoints,
    std::vector<double>& VisualizationPointLocalCoordinates,
    unsigned int& NumVisualizationCells,
    std::vector<NuTo::CellBase::eCellTypes>& VisualizationCellType,
    std::vector<unsigned int>& VisualizationCellsIncidence,
    std::vector<unsigned int>& VisualizationCellsIP) const
{
    NumVisualizationPoints = 9;

    // Point 0
    VisualizationPointLocalCoordinates.push_back(-1);
    VisualizationPointLocalCoordinates.push_back(-1);

    // Point 1
    VisualizationPointLocalCoordinates.push_back(0);
    VisualizationPointLocalCoordinates.push_back(-1);

    // Point 2
    VisualizationPointLocalCoordinates.push_back(1);
    VisualizationPointLocalCoordinates.push_back(-1);

    // Point 3
    VisualizationPointLocalCoordinates.push_back(-1);
    VisualizationPointLocalCoordinates.push_back(0);

    // Point 4
    VisualizationPointLocalCoordinates.push_back(0);
    VisualizationPointLocalCoordinates.push_back(0);

    // Point 5
    VisualizationPointLocalCoordinates.push_back(1);
    VisualizationPointLocalCoordinates.push_back(0);

    // Point 6
    VisualizationPointLocalCoordinates.push_back(-1);
    VisualizationPointLocalCoordinates.push_back(+1);

    // Point 7
    VisualizationPointLocalCoordinates.push_back(0);
    VisualizationPointLocalCoordinates.push_back(+1);

    // Point 8
    VisualizationPointLocalCoordinates.push_back(1);
    VisualizationPointLocalCoordinates.push_back(+1);

    NumVisualizationCells = 4;

    // cell 0
    VisualizationCellType.push_back(NuTo::CellBase::QUAD);
    VisualizationCellsIncidence.push_back(0);
    VisualizationCellsIncidence.push_back(1);
    VisualizationCellsIncidence.push_back(4);
    VisualizationCellsIncidence.push_back(3);
    VisualizationCellsIP.push_back(0);

    // cell 1
    VisualizationCellType.push_back(NuTo::CellBase::QUAD);
    VisualizationCellsIncidence.push_back(1);
    VisualizationCellsIncidence.push_back(2);
    VisualizationCellsIncidence.push_back(5);
    VisualizationCellsIncidence.push_back(4);
    VisualizationCellsIP.push_back(1);

    // cell 2
    VisualizationCellType.push_back(NuTo::CellBase::QUAD);
    VisualizationCellsIncidence.push_back(4);
    VisualizationCellsIncidence.push_back(5);
    VisualizationCellsIncidence.push_back(8);
    VisualizationCellsIncidence.push_back(7);
    VisualizationCellsIP.push_back(2);

    // cell 3
    VisualizationCellType.push_back(NuTo::CellBase::QUAD);
    VisualizationCellsIncidence.push_back(3);
    VisualizationCellsIncidence.push_back(4);
    VisualizationCellsIncidence.push_back(7);
    VisualizationCellsIncidence.push_back(6);
    VisualizationCellsIP.push_back(3);
}
#endif // ENABLE_VISUALIZE