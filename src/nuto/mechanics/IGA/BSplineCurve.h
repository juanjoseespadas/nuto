#pragma once

#include "nuto/math/FullMatrix.h"

#ifdef ENABLE_SERIALIZATION
#include <boost/serialization/export.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#endif // ENABLE_SERIALIZATION

namespace NuTo
{
class BSplineCurve
{
public:
    /** Constructors **/

    //! @brief ... default constructor
    BSplineCurve(){}

    //! @brief ... constructor
    //! @param rDegree ... degree of the polynomial
    //! @param rKnots ... knot vector
    //! @param rControlPoints ... control points
    BSplineCurve(const Eigen::MatrixXd &rKnots,
                 const Eigen::MatrixXd &rControlPoints,
                 int rDegree);

    //! @brief ... constructor (interpolation of a point sequence)
    //! @param rDegree ... degree of the polynomial
    //! @param rPoints ... points to interpolate
    BSplineCurve(int rDegree,
                 const Eigen::MatrixXd& rPoints,
                 Eigen::MatrixXd &AInv);

    /** Getter **/

    //! @brief ... get the degree of the underlying polynomials
    //! @return ... degree of the polynomial
    int GetDegree() const {return mDegree;} const

    //! @brief ... get the dimension of the curve
    //! @return ... dimension of the curve
    int GetDimension() const {return mControlPoints.cols();}

    //! @brief ... get the number of control points
    //! @return ... number of control
    int GetNumControlPoints() const {return mControlPoints.rows();}

    //! @brief ... get the number of knots
    //! @return ... number of knots
    int GetNumKnots() const {return mKnots.rows();}

    //! @brief ... get the control points
    //! @return ... matrix containing the coordinates of the control points
    const Eigen::MatrixXd GetControlPoints() const {return mControlPoints;}

    //! @brief ... get the control points
    //! @return ... matrix containing the coordinates of the control points
    Eigen::VectorXd GetControlPoint(int rControlPointID) const
    {
        assert(rControlPointID < mControlPoints.rows());
        return mControlPoints.row(rControlPointID).transpose();
    }

    //! @brief ... get the knot vector
    //! @return ... knot vector
    const Eigen::VectorXd GetKnotVector() const {return mKnots;}

    //! @brief ... get the maximum knot parameter
    //! @return ... maximum knot parameter
    double GetMaxKnotValue() const
    {
        int numrows = mKnots.rows();
        return mKnots[numrows];
    }

    //! @brief ... get the minimum knot parameter
    //! @return ... minimum knot parameter
    double GetMinKnotValue() const {return mKnots[0];}

    //! @brief ... get the number of nonvanishing sub-intervals in the knot vector
    //! @return ... the number of nonvanishing sub-intervals in the knot vector
    int GetNumIGAElements() const;

    //! @brief ... get the knot vector interval belonging to an element
    //! @return ... id of the knot
    int GetElementFirstKnotID(int rElementID) const;

    //! @brief ... get the knots defining a element
    //! @return ... id of the knot
    Eigen::MatrixXd GetElementKnots(int rElementID) const;

    //! @brief ... get the knot ids defining a element
    //! @return ... id of the knot
    Eigen::VectorXi GetElementKnotIDs(int rElementID) const;

    //! @brief ... get the control points belonging to an element
    //! @return ... control points (#cols = #dim, #rows = number of control points)
    Eigen::MatrixXd GetElementControlPoints(int rElementID) const;

    //! @brief ... get the control point ids belonging to an element
    //! @return ... vector of control point ids
    Eigen::VectorXi GetElementControlPointIDs(int rElementID) const;

    //! @brief ... returns the Bézier extraction operator for the element with ID rElementID
    //! @param rElementID ... element ID
    //! @return ... matrix representing the Bézier extraction operator
    const Eigen::MatrixXd &GetBezierExtraction(int rElementID) const;

    //! @brief ... returns the Bézier extraction operators for all elements
    //! @return ... vector of matrices representing the Bézier extraction operators
    const std::vector<Eigen::MatrixXd> &GetBezierExtraction() const;

    /** Basis Functions **/

    //! @brief ... find span of a parameter (rParameter) in the knot vector
    //! @param rParameter ... parameter to find the span of
    //! @return ... the span index
    Eigen::VectorXd BasisFunctions(double rParameter) const;

    Eigen::MatrixXd BasisFunctionsAndDerivatives(double rParameter, int maxDer) const;

    /** Parametrization **/

    //! @brief ... parametrization for given points to interpolate (chord length method)
    //! @param rPoints ... points to interpolate
    //! @return rParameters ... parameters to the given points
    Eigen::VectorXd ParametrizationChordLengthMethod(const Eigen::MatrixXd &rPoints) const;

    //! @brief ... parametrization for given points to interpolate (centripetal method)
    //! @param rPoints ... points to interpolate
    //! @return rParameters ... parameters to the given points
    Eigen::VectorXd ParametrizationCentripetalMethod(const Eigen::MatrixXd &rPoints) const;

    //! @brief ... set the knot vector due to the parameters given (e.g. computed by the chord length or centripetal methods ...)
    //! @param rPoints ... points to interpolate
    const Eigen::VectorXd& ParametrizationKnotVector(const Eigen::VectorXd &rParameters);

    /** Bézier extraction and related **/

    //! @brief ... elevate the multiplicity of 'inner' knots up to mDegree, to obtain Bernstein polynomials as basis functions, which are equal for every element
    //! the result is saved in the class member mBezierOperators
    void BezierElementExtractionOperators();

    //! @brief ... computes the Bézier control points => advantage: Same structure as FEM matrices. The elements share the minimum amount of control points aka. nodes as in the FEM.
    //! the result is saved in the class member mBezierControlPoints
    void BezierElementControlPoints();


    /** Evaluation **/

    //! @brief ... returns the coordinate of a curve according to a given parameter
    //! @param rParameter ... the parameter
    Eigen::VectorXd CurvePoint(double rParameter) const;

    //! @brief ... returns the coordinates of a curve according to a given parameters
    //! @param rParameter ... the parameters
    Eigen::MatrixXd CurvePoints(const Eigen::VectorXd &rParameter) const;

    /** Knot insertion **/



    /** Degree elevation **/


private:
    //! @brief Knot vector (in isogeometric framework each segment between two knots is an element)
    Eigen::VectorXd mKnots;

    //! @brief Control points of the BSpline curve (# rows = num control points)
    Eigen::MatrixXd mControlPoints;

    //! @brief Degree of the polynomials (order = mDegree+1)
    int mDegree;

    //! @brief Element wise Bézier extraction operators
    std::vector<Eigen::MatrixXd> mBezierOperators;

    //! @brief Element wise Bézier control points
    std::vector<Eigen::MatrixXd> mBezierControlPoints;


};
}
