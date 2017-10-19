#include "mechanics/integrationtypes/IntegrationType0DBoundary.h"

using namespace NuTo;

Eigen::VectorXd NuTo::IntegrationType0DBoundary::GetLocalIntegrationPointCoordinates(int) const
{
    throw Exception("[NuTo::IntegrationType0DBoundary::GetLocalIntegrationPointCoordinates] Ip number out of range.");
}


//! @brief returns the total number of integration points for this integration type
//! @return number of integration points
int NuTo::IntegrationType0DBoundary::GetNumIntegrationPoints() const
{
    return 1;
}

//! @brief returns the weight of an integration point
//! @param rIpNum integration point (counting from zero)
//! @return weight of integration points
double NuTo::IntegrationType0DBoundary::GetIntegrationPointWeight(int rIpNum) const
{
    switch (rIpNum)
    {
    case 0:
        return 1;
    default:
        throw Exception("[NuTo::IntegrationType0DBoundary::GetIntegrationPointWeight] Ip number out of range.");
    }
}


int IntegrationType0DBoundary::GetDimension() const
{
    return 0;
}

void NuTo::IntegrationType0DBoundary::GetVisualizationCells(unsigned int& NumVisualizationPoints, std::vector<double>&,
                                                            unsigned int& NumVisualizationCells,
                                                            std::vector<NuTo::eCellTypes>&, std::vector<unsigned int>&,
                                                            std::vector<unsigned int>&) const
{
    // no visualisation since its a 0D element
    NumVisualizationPoints = 0;
    NumVisualizationCells = 0;
}
