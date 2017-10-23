#include "mechanics/constraints/ConstraintCompanion.h"
#include "mechanics/groups/Group.h"

namespace NuTo
{
namespace Constraint
{

std::vector<Equation> Component(const NodeBase& node, std::vector<eDirection> directions, double value)
{
    return Component(node, directions, RhsConstant(value));
}


std::vector<Equation> Component(const NodeBase& node, std::vector<eDirection> directions, RhsFunction rhs)
{
    std::vector<Equation> eqs;
    for (auto direction : directions)
    {
        int component = ToComponentIndex(direction);
        if (node.GetNumDofs() < component)
            // TODO This check is not meaningful at the moment, since this method returns the
            // total number of dofs, say 4 (3 disp, 1 temp). This will not find the error
            // if you try to constrain the Z component of the temperature. Which would be wrong.
            throw Exception(__PRETTY_FUNCTION__, "Dimension mismatch");

        eqs.push_back(Equation(node, component, rhs));
    }
    return eqs;
}


std::vector<Equation> Component(const Group<NodeBase>& nodes, std::vector<eDirection> directions, double value)
{
    return Component(nodes, directions, RhsConstant(value));
}


std::vector<Equation> Component(const Group<NodeBase>& nodes, std::vector<eDirection> directions, RhsFunction rhs)
{
    std::vector<Equation> eqs;
    for (auto& nodePair : nodes)
    {
        auto tmpEqs = Component(*nodePair.second, directions, rhs);
        eqs.insert(eqs.end(), tmpEqs.begin(), tmpEqs.end());
    }
    return eqs;
}


Equation Direction(const NodeBase& node, Eigen::VectorXd direction, RhsFunction rhs)
{
    if (node.GetNumDofs() < direction.rows())
        // TODO This check is not meaningful at the moment, since this method returns the
        // total number of dofs, say 4 (3 disp, 1 temp). This will not find the error
        // if you try to constrain the Z component of the temperature. Which would be wrong.
        throw Exception(__PRETTY_FUNCTION__, "Dimension mismatch");

    direction.normalize();
    int dependentComponent = -1;
    direction.cwiseAbs().maxCoeff(&dependentComponent);
    assert(dependentComponent > -1 && dependentComponent < direction.rows());

    double invertedFactor = 1. / direction[dependentComponent];

    Equation e(node, dependentComponent, [&](double time) { return rhs(time) * invertedFactor; });
    for (int iComponent = 0; iComponent < direction.rows(); ++iComponent)
        if (iComponent != dependentComponent)
            e.AddTerm(Term(node, iComponent, direction[iComponent] * invertedFactor));

    return e;
}

Equation Direction(const NodeBase& node, Eigen::VectorXd direction, double value)
{
    return Direction(node, direction, RhsConstant(value));
}

std::vector<Equation> Direction(const Group<NodeBase>& nodes, Eigen::VectorXd direction, RhsFunction rhs)
{
    std::vector<Equation> eqs;
    for (auto& nodePair : nodes)
        eqs.push_back(Direction(*nodePair.second, direction, rhs));
    return eqs;
}

std::vector<Equation> Direction(const Group<NodeBase>& nodes, Eigen::VectorXd direction, double value)
{
    return Direction(nodes, direction, RhsConstant(value));
}

Equation Value(const NodeBase& node, double value)
{
    return Value(node, [=](double) { return value; });
}

Equation Value(const NodeBase& node, RhsFunction rhs)
{
    return Direction(node, Eigen::VectorXd::Ones(1), rhs);
}

std::vector<Equation> Value(const Group<NodeBase>& nodes, double value)
{
    return Direction(nodes, Eigen::VectorXd::Ones(1), RhsConstant(value));
}

std::vector<Equation> Value(const Group<NodeBase>& nodes, RhsFunction rhs)
{
    return Direction(nodes, Eigen::VectorXd::Ones(1), rhs);
}

} /* Constraint */
} /* NuTo */
