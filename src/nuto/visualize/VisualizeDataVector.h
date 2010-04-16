// $Id$

#ifndef VISUALIZEDATAVECTOR_H_
#define VISUALIZEDATAVECTOR_H_
#include <iostream>

#include "nuto/visualize/VisualizeDataBase.h"

namespace NuTo
{

//! @brief ... vector (size 3) data for visualization
//! @author Stefan Eckardt, ISM
//! @date November 2009
class VisualizeDataVector: public NuTo::VisualizeDataBase
{
public:
    // constructor
    VisualizeDataVector();

    //! @brief ... get data type
    //! @return ... visualize data type
    NuTo::VisualizeDataType::eDataType GetDataType() const;

    //! @brief ... get number of data
    //! @return ... number of data
    unsigned int GetNumData() const;

    //! @brief ... get data
    //! @return ... pointer to data array
    const double* GetData() const;

    //! @brief ... set data
    //! @param rData ... data array
    void SetData(const double* rData);

private:
    double mData[3];

    //! @brief ... create output stream
    //! @param os ... output stream
    //! @return ... output stream
    std::ostream& Output(std::ostream& os) const;
};
}

#endif // VISUALIZEDATAVECTOR_H_ 
