#pragma once

#include <vector>
#include "mechanics/nodes/NodeSimple.h"
#include "mechanics/interpolation/Interpolation.h"

namespace NuTo
{

typedef Eigen::VectorXd NodeValues;

class Element
{
public:
    Element(std::vector<NuTo::NodeSimple*> rNodes, const Interpolation& rInterpolation)
        : mNodes(rNodes)
        , mInterpolation(rInterpolation)
    {
    }

    NodeValues ExtractNodeValues() const
    {
        int dim = mNodes[0]->GetNumValues();
        Eigen::VectorXd nodeValues(mNodes.size() * dim);
        for (size_t i = 0; i < mNodes.size(); ++i)
            nodeValues.segment(dim * i, dim) = mNodes[i]->GetValues();
        return nodeValues;
    }

    Eigen::VectorXd Interpolate(const Eigen::VectorXd& rLocalCoords) const
    {
        return mInterpolation.GetN(rLocalCoords) * ExtractNodeValues();
    }

    const Interpolation& GetInterpolation() const
    {
        return mInterpolation;
    }

private:
    std::vector<NuTo::NodeSimple*> mNodes;
    const Interpolation& mInterpolation;
};
} /* NuTo */
