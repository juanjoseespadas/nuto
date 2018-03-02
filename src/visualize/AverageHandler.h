#pragma once

#include "visualize/HandlerInterface.h"
#include "visualize/VisualizeEnum.h"

namespace NuTo
{
namespace Visualize
{

//! Geometry description of one cell for the AverageHandler
struct AverageGeometry
{
    std::vector<Eigen::VectorXd> cornerCoordinates;
    eCellTypes cellType;
};

//! Cell handler that averages the cell values over each cell.
class AverageHandler : public HandlerInterface
{
public:
    //! Construct for a given AverageGeometry
    //! @param geometry geometry description of one averaged cell
    AverageHandler(AverageGeometry geometry);

    virtual std::unique_ptr<HandlerInterface> Clone() const override;

    virtual std::vector<int> WriteGeometry(const CellInterface& cell, UnstructuredGrid* grid) override;

    virtual void WriteDofValues(const CellInterface& cell, const DofType dof, std::vector<int> pointIds,
                                UnstructuredGrid* grid) override;

    virtual void CellData(int cellId, std::vector<Eigen::VectorXd> values, std::string name,
                          UnstructuredGrid* grid) override;

    virtual void PointData(const CellInterface& cell, std::function<Eigen::VectorXd(Eigen::VectorXd)> f,
                           std::vector<int> pointIds, std::string name, UnstructuredGrid* grid) override;

private:
    AverageGeometry mGeometry;
};

} // namespace Visualize
} // namespace NuTo