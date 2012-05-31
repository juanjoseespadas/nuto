// $Id$
#ifndef IntegrationType2DMod_H
#define IntegrationType2DMod_H

#ifdef ENABLE_SERIALIZATION
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif //ENABLE_SERIALIZATION

#include <boost/ptr_container/ptr_map.hpp>

#include "nuto/mechanics/integrationtypes/IntegrationPointBase.h"
#include "nuto/mechanics/integrationtypes/IntegrationType2D.h"

namespace NuTo
{
//! @author Daniel Arnold, ISM
//! @date February 2011
//! @brief ... integration types in 2D with four nodes and modifiable integration points
class IntegrationType2DMod : public IntegrationType2D
{
#ifdef ENABLE_SERIALIZATION
    friend class boost::serialization::access;
#endif  // ENABLE_SERIALIZATION

public:
    //! @brief constructor
    IntegrationType2DMod();
    IntegrationType2DMod(std::string rName);

    //! @brief destructor
    ~IntegrationType2DMod();

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
    void GetLocalIntegrationPointCoordinates2D(int rIpNum, double rCoordinates[2]) const;


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

    //! @brief adds a new integration point
    //! @param rIp (Input) integration point
    void AddIntegrationPoint(const IntegrationPointBase & rIp);

    //! @brief deletes an integration point
    //! @param rIpNum (Input) integration point (counting from zero)
    void DeleteIntegrationPoint(const int rIpNum);


protected:
    std::string mName;
    boost::ptr_map< int, IntegrationPointBase >	mIpMap;

};
}

#endif //IntegrationType2DMod_H
