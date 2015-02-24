// $Id: IntegrationType2D4NLobatto16Ip.h 331 2010-10-06 09:32:11Z arnold2 $
#ifndef IntegrationType2D4NLobatto16Ip_H
#define IntegrationType2D4NLobatto16Ip_H

#include "nuto/mechanics/integrationtypes/IntegrationType2D.h"

namespace NuTo
{
//! @author Jörg F. Unger, BAM
//! @date November 2013
//! @brief ... integration types in 1D with Lobatto integration (4x4)
class IntegrationType2D4NLobatto16Ip : public IntegrationType2D
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION

public:
    //! @brief constructor
    IntegrationType2D4NLobatto16Ip();

#ifdef ENABLE_SERIALIZATION
    //! @brief serializes the class
    //! @param ar         archive
    //! @param version    version
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version);
#endif // ENABLE_SERIALIZATION

    //! @brief returns the local coordinates of an integration point
    //! @param rIpNum integration point (counting from zero)
    //! @param rCoordinates (result)
    void GetLocalIntegrationPointCoordinates2D(int rIpNum, double rCoordinates[2])const;


    //! @brief returns the total number of integration points for this integration type
    //! @return number of integration points
    int GetNumIntegrationPoints()const;

    //! @brief returns the weight of an integration point
    //! @param rIpNum integration point (counting from zero)
    //! @return weight of integration points
    double GetIntegrationPointWeight(int rIpNum)const;

    //! @brief returns a string with the identifier of the integration type
    //! @return identifier
    std::string GetStrIdentifier()const;

    //! @brief returns a string with the identifier of the integration type
    //! @return identifier
    static std::string GetStrIdentifierStatic();

#ifdef ENABLE_VISUALIZE
    void GetVisualizationCells(
        unsigned int& NumVisualizationPoints,
        std::vector<double>& VisualizationPointLocalCoordinates,
        unsigned int& NumVisualizationCells,
        std::vector<NuTo::CellBase::eCellTypes>& VisualizationCellType,
        std::vector<unsigned int>& VisualizationCellsIncidence,
        std::vector<unsigned int>& VisualizationCellsIP) const;
#endif // ENABLE_VISUALIZE
private:
    //! @brief ... integration points coordinates
    double iPts[16][2];

    //! @brief ... weights for the integration
    double weights[16];
};
}
#ifdef ENABLE_SERIALIZATION
BOOST_CLASS_EXPORT_KEY(NuTo::IntegrationType2D4NLobatto16Ip)
#endif // ENABLE_SERIALIZATION

#endif //IntegrationType2D4NLobatto16Ip_H
