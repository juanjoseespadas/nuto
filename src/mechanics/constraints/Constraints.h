#pragma once
#include "mechanics/nodes/NodeBase.h"
#include "math/SparseMatrix.h"

namespace NuTo
{
namespace Constraint
{

class Term
{
public:
    Term(const NodeBase& node, int component, double coefficient)
        : mNode(node)
        , mComponent(component)
        , mCoefficient(coefficient)
    {
    }

    const NodeBase& GetNode() const
    {
        return mNode;
    }

    int GetComponent() const
    {
        return mComponent;
    }

    double GetCoefficient() const
    {
        return mCoefficient;
    }

    void ExchangeNode(const NodeBase& oldNode, const NodeBase& newNode)
    {
        if (&mNode.get() == &oldNode)
            mNode = newNode;
    }

private:
    std::reference_wrapper<const NodeBase> mNode;
    int mComponent;
    double mCoefficient;
};

typedef std::function<double(double)> RhsFunction;

class Equation
{
public:
    Equation(RhsFunction rhs)
        : mRhs(rhs)
    {
    }

    Equation(RhsFunction rhs, std::vector<Term> terms)
        : mRhs(rhs)
        , mTerms(terms)
    {
    }

    Equation(const Equation&) = default;
    Equation(Equation&&) = default;

    Equation& operator=(const Equation&) = default;
    Equation& operator=(Equation&&) = default;
    void AddTerm(Term term)
    {
        mTerms.push_back(term);
    }

    const std::vector<Term>& GetTerms() const
    {
        return mTerms;
    }

    std::vector<Term>& GetTerms()
    {
        return mTerms;
    }

    double GetRhs(double time) const
    {
        return mRhs(time);
    }

    ~Equation() = default;

private:
    RhsFunction mRhs;
    std::vector<Term> mTerms;
};

class Constraints
{
    using Equations = std::vector<Equation>;

public:
    void Add(Node::eDof dof, Equation equation)
    {
        mEquations[dof].push_back(equation);
        mHasNewConstraints = true;
    }

    void Add(Node::eDof dof, std::vector<Equation> equations)
    {
        Equations& dofEquations = mEquations[dof];
        dofEquations.insert(dofEquations.begin(), equations.begin(), equations.end());
        mHasNewConstraints = true;
    }

    void SetHasNewConstraints(bool value)
    {
        mHasNewConstraints = value;
    }

    bool HasNewConstraints() const
    {
        return mHasNewConstraints;
    }

    Eigen::VectorXd GetRhs(Node::eDof dof, double time) const
    {
        const auto it = mEquations.find(dof);
        if (it == mEquations.end())
            return Eigen::VectorXd::Zero(0); // no equations for this dof type

        const Equations& equations = it->second;
        int numEquations = equations.size();
        Eigen::VectorXd rhs(numEquations);
        for (int iEquation = 0; iEquation < numEquations; ++iEquation)
            rhs[iEquation] = equations[iEquation].GetRhs(time);
        return rhs;
    }

    void BuildConstraintMatrix(SparseMatrix<double>& rConstraintMatrix, Node::eDof dof) const
    {
        const auto it = mEquations.find(dof);
        if (it == mEquations.end())
            return; // no equations for this dof type

        const Equations& equations = it->second;
        int numEquations = equations.size();

        for (int iEquation = 0; iEquation < numEquations; ++iEquation)
        {
            const auto& equation = equations[iEquation];
            for (const auto& term : equation.GetTerms())
            {
                if (term.GetNode().GetNum(dof) < term.GetComponent())
                    throw MechanicsException(__PRETTY_FUNCTION__,
                                             "Cannot access component " + std::to_string(term.GetComponent()) +
                                                     " from NuTo::NodeBase " + term.GetNode().GetNodeTypeStr());

                double coefficient = term.GetCoefficient();
                int globalDofNumber = term.GetNode().GetDof(dof, term.GetComponent());
                if (std::abs(coefficient > 1.e-18))
                    rConstraintMatrix.AddValue(iEquation, globalDofNumber, coefficient);
            }
        }
    }

    int GetNumEquations(Node::eDof dof) const
    {
        auto it = mEquations.find(dof);
        if (it == mEquations.end())
            return 0;

        return it->second.size();
    }

    void ExchangeNodePtr(const NodeBase& oldNode, const NodeBase& newNode)
    {
        for (auto& mapPair : mEquations)
        {
            auto& equations = mapPair.second;
            for (auto& equation : equations)
            {
                for (auto& term : equation.GetTerms())
                    term.ExchangeNode(oldNode, newNode);
            }
        }
    }

private:
    std::map<Node::eDof, Equations> mEquations;
    bool mHasNewConstraints = false;
};

} /* Constaint */
} /* NuTo */
