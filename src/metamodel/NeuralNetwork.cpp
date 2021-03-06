#include <vector>
#include "metamodel/NeuralNetwork.h"
#include "optimize/ConjugateGradientNonLinear.h"
#include "metamodel/TransferFunction.h"
#include <Eigen/LU>


// constructor
NuTo::NeuralNetwork::NeuralNetwork(std::vector<int> rvNumNeurons)
    : Metamodel()
    , CallbackHandler()
    , mBayesian(true)
    , mUseDiagHessian(true)
    , mInitAlpha(1e-5)
    , mAccuracyGradient(0.0)
    , mMinObjective(0.0)
    , mMinDeltaObjectiveBetweenRestarts(1e-6)
    , mMinDeltaObjectiveBayesianIteration(1e-3)
    , mMaxFunctionCalls(INT_MAX)
    , mShowSteps(100)
    , mMaxBayesianIterations(INT_MAX)
{
    mvNumNeurons = rvNumNeurons;
    mNumLayers = mvNumNeurons.size() + 1;
    mvNumNeurons.resize(mNumLayers); // output layer is included
    mvNumNeurons.insert(mvNumNeurons.begin(), 0); // input layer is included
    mvTransferFunction.resize(mNumLayers, nullptr);
}

//! constructor required for serialization (private)
NuTo::NeuralNetwork::NeuralNetwork()
    : Metamodel()
    , CallbackHandler()
    , mBayesian(true)
    , mUseDiagHessian(true)
    , mInitAlpha(1e-5)
    , mAccuracyGradient(0.0)
    , mMinObjective(0.0)
    , mMinDeltaObjectiveBetweenRestarts(1e-6)
    , mMinDeltaObjectiveBayesianIteration(1e-3)
    , mMaxFunctionCalls(INT_MAX)
    , mShowSteps(100)
    , mMaxBayesianIterations(INT_MAX)
{
    mNumLayers = 1;
    mvNumNeurons.resize(mNumLayers); // output layer is included
    mvNumNeurons.insert(mvNumNeurons.begin(), 0); // input layer is included
    mvTransferFunction.resize(mNumLayers, nullptr);
}


double NuTo::NeuralNetwork::Objective() const
{
    // printf("call mObjective from NuTo::NeuralNetwork\n");
    double objective = 0;

    int dimInput = mSupportPoints.GetDimInput(), dimOutput = mSupportPoints.GetDimOutput();

    int numNeurons(0);
    for (int currentLayer = 0; currentLayer < mNumLayers + 1; currentLayer++)
        numNeurons += mvNumNeurons[currentLayer];

    std::vector<double> pA(numNeurons); // store the current value of each neuron before the transfer function
    std::vector<double> pO(numNeurons); // store the current value of each neuron after the transfer function
    std::vector<double> pDelta(dimOutput); // store the current error for each sample

    if (mBayesian)
    {
        // add the part related to 0.5 alpha w^2
        for (int cntCurrentLayer = 0; cntCurrentLayer < mNumLayers; cntCurrentLayer++)
        {
            const double* pCurWeight = &mvWeights[cntCurrentLayer][0];
            for (int cntCurrentNeuron = 0; cntCurrentNeuron < mvNumNeurons[cntCurrentLayer + 1]; cntCurrentNeuron++)
            {
                if (cntCurrentLayer < mNumLayers - 1)
                {
                    objective += mvAlpha[dimInput + cntCurrentLayer] * mvBias[cntCurrentLayer][cntCurrentNeuron] *
                                 mvBias[cntCurrentLayer][cntCurrentNeuron];
                }
                else
                {
                    objective += mvAlpha[dimInput + cntCurrentLayer + cntCurrentNeuron] *
                                 mvBias[cntCurrentLayer][cntCurrentNeuron] * mvBias[cntCurrentLayer][cntCurrentNeuron];
                }
                for (int cntPreviousNeuron = 0; cntPreviousNeuron < mvNumNeurons[cntCurrentLayer];
                     cntPreviousNeuron++, pCurWeight++)
                {
                    if (cntCurrentLayer == 0) // connection from the input to the first hidden layer
                    {
                        objective += mvAlpha[cntPreviousNeuron] * (*pCurWeight) * (*pCurWeight);
                    }
                    else
                    {
                        if (cntCurrentLayer == mNumLayers - 1)
                            objective += mvAlpha[dimInput + mNumLayers - 1 + cntCurrentNeuron] * (*pCurWeight) *
                                         (*pCurWeight);
                        else
                            objective += mvAlpha[dimInput + cntCurrentLayer - 1] * (*pCurWeight) * (*pCurWeight);
                    }
                }
            }
        }
    }

    for (int cntSample = 0; cntSample < mSupportPoints.GetNumSupportPoints(); cntSample++)
    {
        // set input of input layer
        double curObjective(0);
        memcpy(&pO[0], &(mSupportPoints.GetTransformedSupportPointsInput().data()[cntSample * dimInput]),
               dimInput * sizeof(double));
        ForwardPropagateInput(pA, pO);

        // mapping of approximated and current output to eigen2 matrices
        Eigen::VectorXd VecCurExactOutput = Eigen::Map<Eigen::VectorXd>(
                (double*)&(mSupportPoints.GetTransformedSupportPointsOutput().data()[cntSample * dimOutput]),
                dimOutput);
        Eigen::VectorXd VecCurApproxOutput = Eigen::Map<Eigen::VectorXd>(&pO[numNeurons - dimOutput], dimOutput);

        // error for the current training sample
        Eigen::VectorXd Error = VecCurExactOutput - VecCurApproxOutput;

        if (mBayesian)
            curObjective +=
                    mSupportPoints.GetWeight(cntSample) * (Error.transpose() * mvCovarianceInv * Error).coeff(0, 0);
        else
            curObjective += mSupportPoints.GetWeight(cntSample) * (Error.transpose() * Error).coeff(0, 0);

        objective += curObjective;
    }
    objective *= 0.5; // because o=0.5*alpha*w^2+0.5*EtSE
    // printf("NuTo::NeuralNetwork::mObjective mObjective %g\n\n",mObjective);

    return objective;
}


void NuTo::NeuralNetwork::Gradient(Eigen::MatrixXd& rGradient) const
{
    // printf("call gradient from NuTo::NeuralNetwork\n");
    rGradient.resize(
            mNumWeights + mNumBiases,
            1); // store first the gradient wrt all the weights (starting from first hiddenlayer, and then the biases
    rGradient.setZero();

    int dimInput = mSupportPoints.GetDimInput(), dimOutput = mSupportPoints.GetDimOutput();

    int curNeuron, prevNeuron, nextNeuron, firstNeuronCurrentLayer, firstNeuronPrevLayer, firstNeuronNextLayer;
    int numNeurons = GetNumNeurons();

    std::vector<double> pA(numNeurons); // store the current value of each neuron before applying the transfer function
    std::vector<double> pO(
            numNeurons); // store the current value of each neuron after having applied the transfer function
    std::vector<double> pSigma(
            numNeurons); // store the derivative of the mObjective with respect to A of the current Neuron
    Eigen::VectorXd pDelta(dimOutput); // store the current difference between training and output data
    // const double *pCurWeight(0), *pCurBias(0);            //pointer to current weight
    double *pGradientCurWeight, *pGradientCurBias; // pointer to gradient of current weight/biases

    // forward propagation of the inputs
    for (int cntSample = 0; cntSample < mSupportPoints.GetNumSupportPoints(); cntSample++)
    {
        // set input of input layer
        memcpy(&(pO[0]), &(mSupportPoints.GetTransformedSupportPointsInput().data()[cntSample * dimInput]),
               dimInput * sizeof(double));
        ForwardPropagateInput(pA, pO);

        // backpropagate sensitivities
        // for last layer, this is simple
        pDelta = Eigen::Map<Eigen::VectorXd>(&(pO[numNeurons - dimOutput]), dimOutput) -
                 Eigen::Map<Eigen::VectorXd>(
                         (double*)&(mSupportPoints.GetTransformedSupportPointsOutput().data()[cntSample * dimOutput]),
                         dimOutput);

        if (mBayesian)
            Eigen::Map<Eigen::VectorXd>(&(pSigma[numNeurons - dimOutput]), dimOutput) =
                    pDelta.transpose() * mvCovarianceInv;
        else
            memcpy(&(pSigma[numNeurons - dimOutput]), pDelta.data(), dimOutput * sizeof(double));

        curNeuron = numNeurons - dimOutput;
        for (int cntCurrentNeuron = 0; cntCurrentNeuron < dimOutput; cntCurrentNeuron++, curNeuron++)
        {
            pSigma[curNeuron] *= mvTransferFunction[mNumLayers - 1]->derivative(pA[curNeuron]);
        }

        // for all the other hidden layers, this is a bit more tricky
        firstNeuronCurrentLayer = numNeurons - dimOutput;
        for (int cntCurrentLayer = mNumLayers - 1; cntCurrentLayer > 0; cntCurrentLayer--)
        {
            firstNeuronNextLayer = firstNeuronCurrentLayer;
            firstNeuronCurrentLayer -= mvNumNeurons[cntCurrentLayer];

            curNeuron = firstNeuronCurrentLayer;
            for (int cntCurrentNeuron = 0; cntCurrentNeuron < mvNumNeurons[cntCurrentLayer];
                 cntCurrentNeuron++, curNeuron++)
            {
                pSigma[curNeuron] = 0;
                nextNeuron = firstNeuronNextLayer;
                for (int cntNextNeuron = 0; cntNextNeuron < mvNumNeurons[cntCurrentLayer + 1];
                     cntNextNeuron++, nextNeuron++)
                {
                    // attention, mvWeights[0] are the weights from layer 0 to layer 1
                    // but mvNumNeurons[0] are the number of neurons in first input layer
                    pSigma[curNeuron] += pSigma[nextNeuron] *
                                         mvWeights[cntCurrentLayer]
                                                  [cntNextNeuron * mvNumNeurons[cntCurrentLayer] + cntCurrentNeuron];
                }
                pSigma[curNeuron] *= mvTransferFunction[cntCurrentLayer - 1]->derivative(pA[curNeuron]);
            }
        }

        // calculate gradient for biases and weights according to dE/dw=dE/da*da/dw
        pGradientCurWeight = rGradient.data(); // pointer to gradient of weights
        pGradientCurBias = &(rGradient.data()[mNumWeights]); // pointer to gradient of biases
        firstNeuronCurrentLayer = 0;
        for (int cntCurrentLayer = 0; cntCurrentLayer < mNumLayers; cntCurrentLayer++)
        {
            firstNeuronPrevLayer = firstNeuronCurrentLayer;
            firstNeuronCurrentLayer += mvNumNeurons[cntCurrentLayer];
            curNeuron = firstNeuronCurrentLayer;
            // pCurWeight = &mvWeights[cntCurrentLayer][0];
            // pCurBias = &mvBias[cntCurrentLayer][0];
            for (int cntCurrentNeuron = 0; cntCurrentNeuron < mvNumNeurons[cntCurrentLayer + 1];
                 cntCurrentNeuron++, curNeuron++, pGradientCurBias++)
            {
                *pGradientCurBias += mSupportPoints.GetWeight(cntSample) * pSigma[curNeuron];
                prevNeuron = firstNeuronPrevLayer;
                for (int cntPreviousNeuron = 0; cntPreviousNeuron < mvNumNeurons[cntCurrentLayer];
                     cntPreviousNeuron++, prevNeuron++, pGradientCurWeight++)
                {
                    *pGradientCurWeight += mSupportPoints.GetWeight(cntSample) * pSigma[curNeuron] * pO[prevNeuron];
                }
            }
        }
    }
    if (mBayesian)
    {
        Eigen::VectorXd alpha;
        GetAlphas(alpha);
        // Get initialized parameters
        auto curParameters = GetParameters();
        rGradient.array() += curParameters.array() * alpha.array();
    }
}

void NuTo::NeuralNetwork::Hessian(Eigen::MatrixXd& rHessian) const
{
    if (mUseDiagHessian)
        HessianDiag(rHessian);
    else
        HessianFull(rHessian);
}

void NuTo::NeuralNetwork::HessianFull(Eigen::MatrixXd& rHessian) const
{
    int NumParameters = mNumWeights + mNumBiases;
    Eigen::MatrixXd jacobian(NumParameters, mSupportPoints.GetDimOutput());

    // initialize Hessian
    rHessian.setZero(NumParameters, NumParameters);

    int numNeurons = GetNumNeurons();
    std::vector<double> pA(numNeurons); // store the current value of each neuron before applying the transfer function
    std::vector<double> pO(
            numNeurons); // store the current value of each neuron after having applied the transfer function
    Eigen::MatrixXd pM(numNeurons, mSupportPoints.GetDimOutput()); // store the derivative of the output with respect to
    // A of the current Neuron

    for (int cntSample = 0; cntSample < mSupportPoints.GetNumSupportPoints(); cntSample++)
    {
        // set input of input layer
        memcpy(&(pO[0]),
               &(mSupportPoints.GetTransformedSupportPointsInput().data()[cntSample * mSupportPoints.GetDimInput()]),
               mSupportPoints.GetDimInput() * sizeof(double));
        // calculate Jacobian
        this->Jacobian(jacobian, pA, pO, pM);
        if (mBayesian)
            rHessian += mSupportPoints.GetWeight(cntSample) * (jacobian.transpose() * mvCovarianceInv * jacobian);
        else
            rHessian += mSupportPoints.GetWeight(cntSample) * (jacobian.transpose() * jacobian);
    }
    if (mBayesian)
    {
        // calculate alphas for each weight and bias
        Eigen::VectorXd pAlpha(NumParameters);
        GetAlphas(pAlpha);
        std::cout << rHessian << std::endl;
        std::cout << pAlpha << std::endl;
        rHessian += pAlpha.asDiagonal();
        std::cout << rHessian << std::endl;
    }
}

void NuTo::NeuralNetwork::HessianDiag(Eigen::MatrixXd& rHessian) const
{
    int NumParameters = mNumWeights + mNumBiases;
    Eigen::MatrixXd jacobian(NumParameters, mSupportPoints.GetDimOutput());

    // initialize Hessian
    rHessian.setZero(NumParameters, NumParameters);
    Eigen::MatrixXd rHessian2(rHessian);
    rHessian2.setZero();

    int numNeurons = GetNumNeurons();
    std::vector<double> pA(numNeurons); // store the current value of each neuron before applying the transfer function
    std::vector<double> pO(
            numNeurons); // store the current value of each neuron after having applied the transfer function
    Eigen::MatrixXd pM(numNeurons, mSupportPoints.GetDimOutput()); // store the derivative of the output with respect to
    // A of the current Neuron

    Eigen::MatrixXd tmpMat;
    for (int cntSample = 0; cntSample < mSupportPoints.GetNumSupportPoints(); cntSample++)
    {
        double* ptrHessian(rHessian.data());
        // set input of input layer
        memcpy(&(pO[0]),
               &(mSupportPoints.GetTransformedSupportPointsInput().data()[cntSample * mSupportPoints.GetDimInput()]),
               mSupportPoints.GetDimInput() * sizeof(double));
        // calculate Jacobian
        this->Jacobian(jacobian, pA, pO, pM);

        if (mBayesian)
        {
            tmpMat = mSupportPoints.GetWeight(cntSample) * (jacobian.transpose() * mvCovarianceInv);

            // add only the diagonal values
            double* ptrtmpMat(tmpMat.data());
            double* ptrJacobian(jacobian.data());
            for (int count = 0; count < jacobian.cols(); count++, ptrHessian += jacobian.cols() + 1)
            {
                ptrtmpMat = &(tmpMat.data()[count]);
                for (int count2 = 0; count2 < jacobian.rows(); count2++, ptrtmpMat += jacobian.cols(), ptrJacobian++)
                    *ptrHessian += *ptrtmpMat * (*ptrJacobian);
            }
        }
        else
        {
            // add only the diagonal values
            for (int count = 0; count < jacobian.cols(); count++, ptrHessian += jacobian.cols() + 1)
            {
                *ptrHessian += jacobian.col(count).squaredNorm();
            }
        }
    }
    if (mBayesian)
    {
        // calculate alphas for each weight and bias
        Eigen::VectorXd pAlpha(NumParameters);
        GetAlphas(pAlpha);
        rHessian += pAlpha.asDiagonal();
    }
}

void NuTo::NeuralNetwork::Jacobian(Eigen::MatrixXd& rJacobian, std::vector<double>& pA, std::vector<double>& pO,
                                   Eigen::MatrixXd& pM) const
{
    // before entering the routine, pO, pA have to be resized and the input of the transformed support points has to be
    // copied into the first values of pO
    // pA vector(numNeurons)
    // pO vector(numNeurons)

    // printf("call Jacobian from NuTo::NeuralNetwork\n");
    int dimOutput = mSupportPoints.GetDimOutput();

    int numParameters = mNumWeights + mNumBiases;
    rJacobian.resize(dimOutput, numParameters); // store first the gradient wrt all the weights (starting from first
    // hiddenlayer, and then the biases

    int prevNeuron, curNeuron, nextNeuron, firstNeuronPrevLayer, firstNeuronCurrentLayer, firstNeuronNextLayer;
    int numNeurons(0);
    for (int currentLayer = 0; currentLayer < mNumLayers + 1; currentLayer++)
        numNeurons += mvNumNeurons[currentLayer];

    if ((int)pO.size() != numNeurons)
        throw Exception("[NeuralNetwork::Jacobian] p0 has not the proper size - check source code");
    if ((int)pA.size() != numNeurons)
        throw Exception("[NeuralNetwork::Jacobian] p0 has not the proper size - check source code");

    // set input of input layer
    // memcpy(&(pO[0]),&(mSupportPoints.GetTransformedSupportPointsInput().data()[cntSample*dimInput]),dimInput*sizeof(double));
    ForwardPropagateInput(pA, pO);

    //***********************************************************************
    //** backpropagate sensitivities using Marquardt sensitivities (dy/da) **
    //***********************************************************************
    pM.setZero(dimOutput, numNeurons);
    // for the output layer
    curNeuron = numNeurons - dimOutput;
    for (int cntOutput = 0; cntOutput < dimOutput; cntOutput++, curNeuron++)
    {
        pM(cntOutput, curNeuron) = mvTransferFunction[mNumLayers - 1]->derivative(pA[curNeuron]);
    }

    // for the hidden layers
    firstNeuronCurrentLayer = numNeurons - dimOutput;
    for (int cntCurrentLayer = mNumLayers - 1; cntCurrentLayer > 0; cntCurrentLayer--)
    {
        firstNeuronNextLayer = firstNeuronCurrentLayer;
        firstNeuronCurrentLayer -= mvNumNeurons[cntCurrentLayer];
        curNeuron = firstNeuronCurrentLayer;
        for (int cntCurrentNeuron = 0; cntCurrentNeuron < mvNumNeurons[cntCurrentLayer];
             cntCurrentNeuron++, curNeuron++)
        {
            nextNeuron = firstNeuronNextLayer;
            for (int cntNextNeuron = 0; cntNextNeuron < mvNumNeurons[cntCurrentLayer + 1];
                 cntNextNeuron++, nextNeuron++)
            {
                for (int cntOutput = 0; cntOutput < dimOutput; cntOutput++)
                {
                    pM(cntOutput, curNeuron) +=
                            pM(cntOutput, nextNeuron) *
                            mvTransferFunction[cntCurrentLayer - 1]->derivative(pA[curNeuron]) *
                            mvWeights[cntCurrentLayer]
                                     [cntNextNeuron * mvNumNeurons[cntCurrentLayer] + cntCurrentNeuron];
                }
            }
        }
    }

    double* curJacBias = &rJacobian.data()[mNumWeights * dimOutput];
    double* curJacWeight = rJacobian.data();
    firstNeuronCurrentLayer = 0;
    for (int cntCurrentLayer = 0; cntCurrentLayer < mNumLayers; cntCurrentLayer++)
    {
        firstNeuronPrevLayer = firstNeuronCurrentLayer;
        firstNeuronCurrentLayer += mvNumNeurons[cntCurrentLayer];
        curNeuron = firstNeuronCurrentLayer;
        for (int cntCurrentNeuron = 0; cntCurrentNeuron < mvNumNeurons[cntCurrentLayer + 1];
             cntCurrentNeuron++, curNeuron++)
        {
            for (int cntOutput = 0; cntOutput < dimOutput; cntOutput++, curJacBias++)
            {
                // update sensitivities for biases
                *curJacBias = pM(cntOutput, curNeuron);
            }

            // update sensitivities for weights
            prevNeuron = firstNeuronPrevLayer;
            for (int cntPrevNeuron = 0; cntPrevNeuron < mvNumNeurons[cntCurrentLayer]; cntPrevNeuron++, prevNeuron++)
            {
                for (int cntOutput = 0; cntOutput < dimOutput; cntOutput++, curJacWeight++)
                {
                    *curJacWeight = pM(cntOutput, curNeuron) * pO[prevNeuron];
                }
            }
        }
    }
}

void NuTo::NeuralNetwork::BuildDerived()
{

    // set number of neurons of the final layer to the dimension of the output and in 0 to the dimension of the input
    mvNumNeurons[0] = mSupportPoints.GetDimInput();
    mvNumNeurons[mNumLayers] = mSupportPoints.GetDimOutput();

    // allocate Weights and Biases
    mvWeights.resize(mNumLayers);
    mvBias.resize(mNumLayers);

    mNumWeights = 0;
    mNumBiases = 0;
    // connection of input layer to next layer
    for (int currentLayer = 0; currentLayer < mNumLayers; currentLayer++)
    {
        // attention - mvNumNeurons[0] is the input layer and mvNumNeurons[1] the first hidden layer
        // but mvWeights[0] corresponds to the weights from the first hidden layer to the previous layer
        mvWeights[currentLayer].resize(mvNumNeurons[currentLayer] * mvNumNeurons[currentLayer + 1]);

        double b = sqrt(3. / mvNumNeurons[currentLayer]);
        for (double& weight : mvWeights[currentLayer])
        {
            weight = 2 * b * RandomDouble() - b;
        }
        mNumWeights += mvNumNeurons[currentLayer] * mvNumNeurons[currentLayer + 1];
        mvBias[currentLayer].resize(mvNumNeurons[currentLayer + 1], 0);
        mNumBiases += mvNumNeurons[currentLayer + 1];
        // check if all transfer functions are set
        if (mvTransferFunction[currentLayer] == nullptr)
            throw Exception("NuTo::NeuralNetwork::BuildDerived - Transferfunction not correctly set for all layers.");
    }

    int numParameters(mNumWeights + mNumBiases);
    // calculate for each parameter the specific position in mvAlpha for the corresponding alpha
    std::vector<int> posInAlphaVector;

    // calculate number of parameters that belong to a specific alpha
    std::vector<int> refsPerAlpha;

    if (mBayesian)
    {
        // initialize covariance matrix
        mvCovarianceInv.setIdentity(mSupportPoints.GetDimOutput(), mSupportPoints.GetDimOutput());

        // initialize alphas (precision of weights)
        mvAlpha.resize(mSupportPoints.GetDimInput() + mSupportPoints.GetDimOutput() + mNumLayers - 1, mInitAlpha);

        // calculate for each parameter the specific position in mvAlpha for the corresponding alpha
        GetPosInAlphaVector(posInAlphaVector);

        // calculate number of parameters that belong to a specific alpha
        GetRefsPerAlpha(refsPerAlpha);
    }

    Eigen::MatrixXd hessian(numParameters, numParameters);
    hessian.setZero();
    Eigen::MatrixXd invHessian(numParameters, numParameters);
    invHessian.setZero();

    // allocate Optimizer
    ConjugateGradientNonLinear myOptimizer(numParameters);

    // set callback routines for the calculation of the mObjective function, gradient etc
    // this works, because Neural network has been derived from CallbackHandler of the optimization module
    myOptimizer.SetCallback(this);
    myOptimizer.SetVerboseLevel(mVerboseLevel);
    myOptimizer.SetAccuracyGradient(mAccuracyGradient);
    myOptimizer.SetMinObjective(mMinObjective);
    myOptimizer.SetMinDeltaObjBetweenRestarts(mMinDeltaObjectiveBetweenRestarts);
    myOptimizer.SetMaxFunctionCalls(mMaxFunctionCalls);
    myOptimizer.SetShowSteps(mShowSteps);

    // Get initialized parameters
    auto curParameters = GetParameters();
    myOptimizer.SetParameters(curParameters);

    // help arrays for forward/backward propagation
    int numNeurons = GetNumNeurons();
    std::vector<double> pA(numNeurons); // store the current value of each neuron before applying the transfer function
    std::vector<double> pO(
            numNeurons); // store the current value of each neuron after having applied the transfer function
    Eigen::MatrixXd pM(numNeurons, mSupportPoints.GetDimOutput()); // store the derivative of the output with respect to
    // A of the current Neuron

    // help arrays for the update of the hyperparameters
    std::vector<double> gammas;
    gammas.resize(mvAlpha.size());
    std::vector<double> sumW2;
    sumW2.resize(mvAlpha.size());
    std::vector<double> sumDiagInv;
    sumDiagInv.resize(mvAlpha.size());

    // previous hyperparameters
    Eigen::MatrixXd vCovarianceInvPrev;
    std::vector<double> vAlphaPrev;

    // updated hyperparameters
    std::vector<double> vAlphaNew;
    vAlphaNew.resize(mvAlpha.size());
    Eigen::MatrixXd vCovarianceNew;
    Eigen::MatrixXd vCovarianceInvNew;

    // covergence flags for the hyperparameters
    // bool convergedAlpha;
    // bool convergedCovarianceInv;

    // cutback factor for peform kind of a line search in the update of the hyperparameters
    double cutbackFactor(1.0);
    int theGlobalIteration(0);
    double prevObjective(-1e99);
    bool converged(false);
    while (!converged)
    {
        // convergedAlpha=false;
        // convergedCovarianceInv=false;

        // store old hyperparameters
        vCovarianceInvPrev = mvCovarianceInv;
        vAlphaPrev = mvAlpha;

        // Perform optimization procedure with constant hyperparameters
        if (this->mVerboseLevel > 0)
        {
            std::cout << "[NeuralNetwork::BuildDerived] Determine weights and biases of the neural network (call "
                         "optimization procedure)."
                      << std::endl;
        }
        myOptimizer.Optimize();

        if (!mBayesian)
        {
            converged = true;
        }
        else if (theGlobalIteration >= mMaxBayesianIterations)
        {
            if (this->mVerboseLevel > 0)
            {
                std::cout << "[NeuralNetwork::BuildDerived] Bayesian loop is stopped since the maximum number of "
                             "iteration steps is reached."
                          << std::endl;
            }
            converged = true;
        }
        else
        {
            if (this->mVerboseLevel > 0)
            {
                std::cout << "[NeuralNetwork::BuildDerived] Update hyperparameters alpha and noise covariance matrix "
                             "of the bayesian neural network."
                          << std::endl;
            }
            // calculate full hessian
            HessianFull(hessian);

            // compute inverse hessian
            invHessian = hessian.inverse();

            // update hyperparameters alpha and noise level
            double sumGamma(0);
            for (int theAlpha = 0; theAlpha < (int)mvAlpha.size(); theAlpha++)
            {
                // calculate gammas
                sumW2[theAlpha] = 0.;
                sumDiagInv[theAlpha] = 0.;
                for (int count2 = 0; count2 < numParameters; count2++)
                {
                    if (posInAlphaVector[count2] == theAlpha)
                    {
                        sumDiagInv[theAlpha] += invHessian(count2, count2);
                        sumW2[theAlpha] +=
                                myOptimizer.GetParameters()(count2, 0) * myOptimizer.GetParameters()(count2, 0);
                    }
                }
                gammas[theAlpha] = refsPerAlpha[theAlpha] - mvAlpha[theAlpha] * sumDiagInv[theAlpha];

                sumGamma += gammas[theAlpha];

                // calculate new alphas
                if (gammas[theAlpha] >= 0)
                {
                    if (sumW2[theAlpha] > 1e-10)
                    {
                        vAlphaNew[theAlpha] = gammas[theAlpha] / sumW2[theAlpha];
                    }
                    else
                    {
                        vAlphaNew[theAlpha] = gammas[theAlpha] / (1e-10);
                    }
                }
                else
                {
                    vAlphaNew[theAlpha] = 1e-12;
                    printf("gammas[%d] = %g (%g/%g)\n", theAlpha, gammas[theAlpha] / sumW2[theAlpha], gammas[theAlpha],
                           sumW2[theAlpha]);
                    throw NuTo::Exception("[NuTo::NeuralNetwork::BuildDerived] Gamma is negative.");
                }
            }

            dLnDetA_dBeta(invHessian, vCovarianceNew);

            double totalWeight(0);
            for (int cntSample = 0; cntSample < mSupportPoints.GetNumSupportPoints(); cntSample++)
            {
                // set input of input layer
                memcpy(&pO[0], &(mSupportPoints.GetTransformedSupportPointsInput()
                                         .data()[cntSample * mSupportPoints.GetDimInput()]),
                       mSupportPoints.GetDimInput() * sizeof(double));
                ForwardPropagateInput(pA, pO);

                // mapping of approximated and current output to eigen2 matrices
                Eigen::VectorXd VecCurExactOutput = Eigen::Map<Eigen::VectorXd>(
                        (double*)&(mSupportPoints.GetTransformedSupportPointsOutput()
                                           .data()[cntSample * mSupportPoints.GetDimOutput()]),
                        mSupportPoints.GetDimOutput());
                Eigen::VectorXd VecCurApproxOutput = Eigen::Map<Eigen::VectorXd>(
                        &pO[numNeurons - mSupportPoints.GetDimOutput()], mSupportPoints.GetDimOutput());
                Eigen::VectorXd Error = VecCurExactOutput - VecCurApproxOutput;
                vCovarianceNew += mSupportPoints.GetWeight(cntSample) * Error * Error.transpose();
                totalWeight += mSupportPoints.GetWeight(cntSample);
            }

            vCovarianceNew *= 1. / totalWeight;

            vCovarianceInvNew = vCovarianceNew.inverse();

            // calculate new values according to a linesearch with a cutback factor, which depends on the current
            // iteration number (cycles for the update of hyperparameters
            for (int count = 0; count < (int)mvAlpha.size(); count++)
            {
                mvAlpha[count] += (vAlphaNew[count] - mvAlpha[count]) * cutbackFactor;
                if (mvAlpha[count] > 1e99)
                    mvAlpha[count] = 1e99;
            }

            mvCovarianceInv += cutbackFactor * (vCovarianceInvNew - mvCovarianceInv);

            if (mVerboseLevel > 2)
            {
                std::cout << "previous alpha values (hyperparameters):" << std::endl;
                std::cout << Eigen::VectorXd::Map(vAlphaPrev.data(), vAlphaPrev.size()).transpose() << std::endl;

                std::cout << "alpha values (hyperparameters):" << std::endl;
                std::cout << Eigen::VectorXd::Map(mvAlpha.data(), mvAlpha.size()).transpose() << std::endl;

                std::cout << "previous inverse covariance matrix:" << std::endl;
                std::cout << vCovarianceInvPrev << std::endl;

                std::cout << "inverse covariance matrix:" << std::endl;
                std::cout << mvCovarianceInv << std::endl;

                // calculate the standard deviation of the noise for each output
                vCovarianceNew = vCovarianceInvNew.inverse();
                Eigen::VectorXd stdDev = vCovarianceNew.diagonal().array().sqrt().matrix();
                std::cout << "standard deviation of the noise: - (transformed space)" << std::endl;
                std::cout << stdDev.transpose() << std::endl;

                // calculate the correlation of the noise for the outputs
                vCovarianceNew.array() /= (stdDev * stdDev.transpose()).array();
                std::cout << "correlation of the noise:" << std::endl;
                std::cout << vCovarianceNew << std::endl;
            }
            /*
                        double deltaNormAlpha(0);
                        for (unsigned int counter=0; counter<mvAlpha.size(); counter++)
                        {
                            if (vAlphaPrev[counter]>1)
                                deltaNormAlpha+=(mvAlpha[counter]-vAlphaPrev[counter])*(mvAlpha[counter]-vAlphaPrev[counter])/(vAlphaPrev[counter]*vAlphaPrev[counter]);
                            else
                                deltaNormAlpha+=(mvAlpha[counter]-vAlphaPrev[counter])*(mvAlpha[counter]-vAlphaPrev[counter]);
                        }
                        if (deltaNormAlpha<1e-6)
                        {
                            convergedAlpha = true;
                        }

                        double deltaNormCovarianceInv(0);
                        for (int thecol=0; thecol<mvCovarianceInv.cols(); thecol++)
                        {
                            for (int therow=0; therow<mvCovarianceInv.rows(); therow++)
                            {
                                if (vCovarianceInvPrev(therow,thecol)>1)
                                    deltaNormCovarianceInv =
               (mvCovarianceInv(therow,thecol)-vCovarianceInvPrev(therow,thecol))*(mvCovarianceInv(therow,thecol)-vCovarianceInvPrev(therow,thecol))/(vCovarianceInvPrev(therow,thecol)*vCovarianceInvPrev(therow,thecol));
                                else
                                    deltaNormCovarianceInv =
               (mvCovarianceInv(therow,thecol)-vCovarianceInvPrev(therow,thecol))*(mvCovarianceInv(therow,thecol)-vCovarianceInvPrev(therow,thecol));

                        }
                        if (deltaNormCovarianceInv<1e-6)
                        {
                            convergedCovarianceInv = true;
                        }
            */
            // these values are empirical
            if (theGlobalIteration > 300)
                cutbackFactor = 0.5;
            if (theGlobalIteration > 1000)
                cutbackFactor = 0.25;
            if (theGlobalIteration > 3000)
                cutbackFactor = 0.1;
            if (theGlobalIteration > 5000)
                cutbackFactor = 0.01;
            if (theGlobalIteration > 8000)
                cutbackFactor = 0.001;


            if (std::abs(myOptimizer.GetObjective() - prevObjective) / myOptimizer.GetObjective() <
                this->mMinDeltaObjectiveBayesianIteration)
            {
                if (mVerboseLevel > 0)
                    std::cout << "[NeuralNetwork::BuildDerived] Bayesian loop converged due to delta of "
                              << std::abs(myOptimizer.GetObjective() - prevObjective) << " between iterations."
                              << std::endl;
                converged = true;

                // Perform final optimization procedure with constant hyperparameters
                if (this->mVerboseLevel > 0)
                {
                    std::cout << "[NeuralNetwork::BuildDerived] Determine the final weights and biases of the bayesian "
                                 "neural network (call optimization procedure)."
                              << std::endl;
                }
                myOptimizer.Optimize();
            }
            else
            {
                if (this->mVerboseLevel > 0)
                {
                    std::cout << "[NeuralNetwork::BuildDerived] Bayes iteration step " << theGlobalIteration
                              << ": delta in mObjective between two bayes iterations: "
                              << std::abs(myOptimizer.GetObjective() - prevObjective) << " ("
                              << this->mMinDeltaObjectiveBayesianIteration * myOptimizer.GetObjective() << ")"
                              << std::endl;
                }
            }

            prevObjective = myOptimizer.GetObjective();
            theGlobalIteration++;
        } // mBayesian
    } // totalConverged
}

void NuTo::NeuralNetwork::ForwardPropagateInput(std::vector<double>& pA, std::vector<double>& pO) const
{
    // pA and pO have have the dimension equal to the number of neurons including the input
    // pO for the first values (corresponding to the input layer) is set to the input
    const double* pCurWeight;
    int firstNeuronPrevLayer = 0;
    int firstNeuronCurrentLayer = mSupportPoints.GetDimInput();
    int prevNeuron, curNeuron;

    // progressively update for next layers, layer 0 is the input layer
    for (int cntCurrentLayer = 1; cntCurrentLayer < mNumLayers + 1; cntCurrentLayer++)
    {
        curNeuron = firstNeuronCurrentLayer;
        // printf("cntCurrentLayer %d mvWeights.size() %d\n",cntCurrentLayer,mvWeights[cntCurrentLayer-1].size());
        pCurWeight = &(mvWeights[cntCurrentLayer - 1][0]);
        for (int cntCurrentNeuron = 0; cntCurrentNeuron < mvNumNeurons[cntCurrentLayer];
             cntCurrentNeuron++, curNeuron++)
        {
            pA[curNeuron] = mvBias[cntCurrentLayer - 1][cntCurrentNeuron];
            prevNeuron = firstNeuronPrevLayer;
            for (int cntPreviousNeuron = 0; cntPreviousNeuron < mvNumNeurons[cntCurrentLayer - 1];
                 cntPreviousNeuron++, prevNeuron++, pCurWeight++)
            {
                pA[curNeuron] += pO[prevNeuron] * (*pCurWeight);
            }
            // apply transfer function
            pO[curNeuron] = mvTransferFunction[cntCurrentLayer - 1]->evaluate(pA[curNeuron]);
            // printf("%g\n",(*pCurValue));
        }
        firstNeuronPrevLayer = firstNeuronCurrentLayer;
        firstNeuronCurrentLayer += mvNumNeurons[cntCurrentLayer];
    }
}


void NuTo::NeuralNetwork::SetTransferFunction(int rLayer, eTransferFunctions rTransferFunction)
{
    if (rLayer >= mNumLayers)
        throw Exception("Metamodel::SetTransferFunction - Layer out of size.");

    switch (rTransferFunction)
    {
    case Empty:
        if (mvTransferFunction[rLayer] != nullptr)
            delete mvTransferFunction[rLayer];
        mvTransferFunction[rLayer] = new EmptyTransferFunction();
        break;
    case HardLim:
        if (mvTransferFunction[rLayer] != nullptr)
            delete mvTransferFunction[rLayer];
        mvTransferFunction[rLayer] = new HardLimTransferFunction();
        break;
    case HardLims:
        if (mvTransferFunction[rLayer] != nullptr)
            delete mvTransferFunction[rLayer];
        mvTransferFunction[rLayer] = new HardLimsTransferFunction();
        break;
    case PureLin:
        if (mvTransferFunction[rLayer] != nullptr)
            delete mvTransferFunction[rLayer];
        mvTransferFunction[rLayer] = new PureLinTransferFunction();
        break;
    case SatLin:
        if (mvTransferFunction[rLayer] != nullptr)
            delete mvTransferFunction[rLayer];
        mvTransferFunction[rLayer] = new SatLinTransferFunction();
        break;
    case SatLins:
        if (mvTransferFunction[rLayer] != nullptr)
            delete mvTransferFunction[rLayer];
        mvTransferFunction[rLayer] = new SatLinsTransferFunction();
        break;
    case LogSig:
        if (mvTransferFunction[rLayer] != nullptr)
            delete mvTransferFunction[rLayer];
        mvTransferFunction[rLayer] = new LogSigTransferFunction();
        break;
    case TanSig:
        if (mvTransferFunction[rLayer] != nullptr)
            delete mvTransferFunction[rLayer];
        mvTransferFunction[rLayer] = new TanSigTransferFunction();
        break;
    case PosLin:
        if (mvTransferFunction[rLayer] != nullptr)
            delete mvTransferFunction[rLayer];
        mvTransferFunction[rLayer] = new PosLinTransferFunction();
        break;
    default:
        throw Exception("NuTo::NeuralNetwork::SetTransferFunction - TransferFunction not known).");
        break;
    }
}

void NuTo::NeuralNetwork::SetParameters(const Eigen::MatrixXd& Parameters)
{
    if (Parameters.rows() != mNumWeights + mNumBiases)
    {
        throw Exception("Metamodel::SetParameters - Weights and Biases not allocated - build first.");
    }

    if (Parameters.cols() != 1)
    {
        throw Exception("Metamodel::SetParameters - Number of Columns is not equal to one.");
    }

    const double* pCurParameter = Parameters.data();
    // progressively update for next layers, layer 0 is the input layer
    for (int cntCurrentLayer = 0; cntCurrentLayer < mNumLayers; cntCurrentLayer++)
    {
        memcpy(&mvWeights[cntCurrentLayer][0], pCurParameter,
               mvNumNeurons[cntCurrentLayer] * mvNumNeurons[cntCurrentLayer + 1] * sizeof(double));
        pCurParameter += mvNumNeurons[cntCurrentLayer] * mvNumNeurons[cntCurrentLayer + 1];
    }

    for (int cntCurrentLayer = 0; cntCurrentLayer < mNumLayers; cntCurrentLayer++)
    {
        memcpy(&mvBias[cntCurrentLayer][0], pCurParameter, mvNumNeurons[cntCurrentLayer + 1] * sizeof(double));
        pCurParameter += mvNumNeurons[cntCurrentLayer + 1];
    }
}

Eigen::MatrixXd NuTo::NeuralNetwork::GetParameters() const
{
    Eigen::MatrixXd Parameters;
    Parameters.resize(mNumWeights + mNumBiases, 1);
    Parameters.setZero();

    double* pCurParameter = Parameters.data();
    // progressively update for next layers, layer 0 is the input layer
    for (int cntCurrentLayer = 0; cntCurrentLayer < mNumLayers; cntCurrentLayer++)
    {
        memcpy(pCurParameter, &mvWeights[cntCurrentLayer][0],
               mvNumNeurons[cntCurrentLayer] * mvNumNeurons[cntCurrentLayer + 1] * sizeof(double));
        pCurParameter += mvNumNeurons[cntCurrentLayer] * mvNumNeurons[cntCurrentLayer + 1];
    }

    for (int cntCurrentLayer = 0; cntCurrentLayer < mNumLayers; cntCurrentLayer++)
    {
        memcpy(pCurParameter, &mvBias[cntCurrentLayer][0], mvNumNeurons[cntCurrentLayer + 1] * sizeof(double));
        pCurParameter += mvNumNeurons[cntCurrentLayer + 1];
    }

    return Parameters;
}

void NuTo::NeuralNetwork::SolveTransformed(const Eigen::MatrixXd& rInputCoordinates,
                                           Eigen::MatrixXd& rOutputCoordinates) const
{
    int dimInput = mSupportPoints.GetDimInput(), dimOutput = mSupportPoints.GetDimOutput();

    int numNeurons = GetNumNeurons();

    std::vector<double> pA(numNeurons); // store the current value of each neuron before the transfer function
    std::vector<double> pO(numNeurons); // store the current value of each neuron after the transfer function

    if (rInputCoordinates.rows() != dimInput)
    {
        throw Exception(
                "Metamodel::SolveTransformed - Dimension of input (number of rows) is not identical with metamodel.");
    }

    rOutputCoordinates.resize(dimOutput, rInputCoordinates.cols());
    rOutputCoordinates.setZero();

    for (int cntSample = 0; cntSample < rInputCoordinates.cols(); cntSample++)
    {
        // set input of input layer
        memcpy(&pO[0], &(rInputCoordinates.data()[cntSample * dimInput]), dimInput * sizeof(double));
        ForwardPropagateInput(pA, pO);

        // mapping of approximated and current output to eigen2 matrices and blockset
        rOutputCoordinates.col(cntSample) = Eigen::Map<Eigen::VectorXd>(&pO[numNeurons - dimOutput], dimOutput);
    }
}

void NuTo::NeuralNetwork::SolveConfidenceIntervalTransformed(const Eigen::MatrixXd& rInputCoordinates,
                                                             Eigen::MatrixXd& rOutputCoordinates,
                                                             Eigen::MatrixXd& rOutputCoordinatesMin,
                                                             Eigen::MatrixXd& rOutputCoordinatesMax) const
{
    if (!mBayesian)
    {
        throw Exception("Metamodel::SolveConfidenceIntervalTransformed - A prediction of the confidence "
                        "interval is only possible with Bayesian neural networks.");
    }
    int dimInput = mSupportPoints.GetDimInput(), dimOutput = mSupportPoints.GetDimOutput();

    int numNeurons = GetNumNeurons();

    int numParameters(mNumWeights + mNumBiases);
    Eigen::MatrixXd hessian(numParameters, numParameters);
    hessian.setZero();
    Eigen::MatrixXd invHessian(numParameters, numParameters);
    HessianFull(hessian);
    invHessian = hessian.inverse();

    std::vector<double> pA(numNeurons); // store the current value of each neuron before the transfer function
    std::vector<double> pO(numNeurons); // store the current value of each neuron after the transfer function
    Eigen::MatrixXd pM(numNeurons, mSupportPoints.GetDimOutput()); // store the derivative of the output with respect to
    // A of the current Neuron
    pM.setZero();
    Eigen::MatrixXd tmpCovariance(
            dimOutput, dimOutput); // store the derivative of the output with respect to A of the current Neuron
    tmpCovariance.setZero();
    Eigen::MatrixXd mvCovariance;
    mvCovariance.setZero();

    if (rInputCoordinates.rows() != dimInput)
    {
        throw Exception(
                "Metamodel::SolveTransformed - Dimension of input (number of rows) is not identical with metamodel.");
    }

    mvCovariance = mvCovarianceInv.inverse();

    rOutputCoordinates.resize(dimOutput, rInputCoordinates.cols());
    rOutputCoordinates.setZero();
    rOutputCoordinatesMin.resize(dimOutput, rInputCoordinates.cols());
    rOutputCoordinatesMin.setZero();
    rOutputCoordinatesMax.resize(dimOutput, rInputCoordinates.cols());
    rOutputCoordinatesMax.setZero();

    Eigen::MatrixXd jacobian(numParameters, mSupportPoints.GetDimOutput());

    for (int cntSample = 0; cntSample < rInputCoordinates.cols(); cntSample++)
    {
        // set input of input layer
        memcpy(&pO[0], &(rInputCoordinates.data()[cntSample * dimInput]), dimInput * sizeof(double));

        // calculate the Jacobian for the current sample including the output
        this->Jacobian(jacobian, pA, pO, pM);

        // mapping of approximated and current output to eigen2 matrices and blockset
        rOutputCoordinates.col(cntSample) = Eigen::Map<Eigen::VectorXd>(&pO[numNeurons - dimOutput], dimOutput);

        // calculate covariance matrix for output
        tmpCovariance = mvCovariance + jacobian * invHessian * jacobian.transpose();

        // calculate standard deviation for outputs
        Eigen::VectorXd interval = tmpCovariance.diagonal().array().sqrt().matrix();

        // add/subtract standard deviation to/from mean output
        rOutputCoordinatesMax.col(cntSample) = rOutputCoordinates.col(cntSample) + interval;
        rOutputCoordinatesMin.col(cntSample) = rOutputCoordinates.col(cntSample) - interval;
    }
}

void NuTo::NeuralNetwork::Info() const
{
    NuTo::Metamodel::Info();
    // progressively update for next layers, layer 0 is the input layer
    for (int cntCurrentLayer = 0; cntCurrentLayer < mNumLayers; cntCurrentLayer++)
    {
        std::cout << "Layer " << cntCurrentLayer << "\n Weights\n";
        std::cout << Eigen::VectorXd::Map(mvWeights[cntCurrentLayer].data(), mvWeights[cntCurrentLayer].size())
                             .transpose()
                  << std::endl;
        std::cout << "Biases\n";
        std::cout << Eigen::VectorXd::Map(mvBias[cntCurrentLayer].data(), mvBias[cntCurrentLayer].size()).transpose()
                  << std::endl;
    }
}

void NuTo::NeuralNetwork::GetAlphas(Eigen::VectorXd& rAlpha) const
{
    rAlpha.setZero(mNumWeights + mNumBiases);
    double* curBias = &(rAlpha.data()[mNumWeights]);
    double* curWeights = rAlpha.data();

    if (mNumLayers == 1)
    {
        throw Exception("Metamodel::GetAlphas - without hidden layer the definition of alpha is not implemented.");
    }
    // for the first layer, each input has its own alpha
    for (int cntCurrentNeuron = 0; cntCurrentNeuron < mvNumNeurons[1]; cntCurrentNeuron++, curBias++)
    {
        for (int cntPrevNeuron = 0; cntPrevNeuron < mvNumNeurons[0]; cntPrevNeuron++, curWeights++)
        {
            *curWeights = mvAlpha[cntPrevNeuron];
        }
        *curBias = mvAlpha[mSupportPoints.GetDimInput()];
    }

    // for the hidden layer, bias and weights have the same alpha
    for (int cntCurrentLayer = 1; cntCurrentLayer < mNumLayers - 1; cntCurrentLayer++)
    {
        for (int cnt = 0; cnt < mvNumNeurons[cntCurrentLayer] * mvNumNeurons[cntCurrentLayer + 1]; cnt++, curWeights++)
            *curWeights = mvAlpha[mSupportPoints.GetDimInput() + cntCurrentLayer];
        for (int cnt = 0; cnt < mvNumNeurons[cntCurrentLayer + 1]; cnt++, curBias++)
            *curBias = mvAlpha[mSupportPoints.GetDimInput() + cntCurrentLayer];
    }

    // for the output layer, an alpha for each output is defined
    for (int cntCurrentNeuron = 0; cntCurrentNeuron < mvNumNeurons[mNumLayers]; cntCurrentNeuron++, curBias++)
    {
        for (int cntPrevNeuron = 0; cntPrevNeuron < mvNumNeurons[mNumLayers - 1]; cntPrevNeuron++, curWeights++)
        {
            *curWeights = mvAlpha[mvAlpha.size() - mSupportPoints.GetDimOutput() + cntCurrentNeuron];
        }
        *curBias = mvAlpha[mvAlpha.size() - mSupportPoints.GetDimOutput() + cntCurrentNeuron];
    }
}

void NuTo::NeuralNetwork::GetPosInAlphaVector(std::vector<int>& rPosInAlphaVector) const
{
    rPosInAlphaVector.resize(mNumWeights + mNumBiases);
    int* curBias = &(rPosInAlphaVector[mNumWeights]);
    int* curWeights = &(rPosInAlphaVector[0]);

    if (mNumLayers == 1)
    {
        throw Exception(
                "Metamodel::GetPosInAlphaVector - without hidden layer the definition of alpha is not implemented.");
    }
    // for the first layer, each input has its own alpha
    for (int cntCurrentNeuron = 0; cntCurrentNeuron < mvNumNeurons[1]; cntCurrentNeuron++, curBias++)
    {
        for (int cntPrevNeuron = 0; cntPrevNeuron < mvNumNeurons[0]; cntPrevNeuron++, curWeights++)
        {
            *curWeights = cntPrevNeuron;
        }
        *curBias = mSupportPoints.GetDimInput();
    }

    // for the hidden layer, bias and weights have the same alpha
    for (int cntCurrentLayer = 1; cntCurrentLayer < mNumLayers - 1; cntCurrentLayer++)
    {
        for (int cnt = 0; cnt < mvNumNeurons[cntCurrentLayer] * mvNumNeurons[cntCurrentLayer + 1]; cnt++, curWeights++)
            *curWeights = mSupportPoints.GetDimInput() + cntCurrentLayer;
        for (int cnt = 0; cnt < mvNumNeurons[cntCurrentLayer + 1]; cnt++, curBias++)
            *curBias = mSupportPoints.GetDimInput() + cntCurrentLayer;
    }

    // for the output layer, an alpha for each output is defined
    for (int cntCurrentNeuron = 0; cntCurrentNeuron < mvNumNeurons[mNumLayers]; cntCurrentNeuron++, curBias++)
    {
        for (int cntPrevNeuron = 0; cntPrevNeuron < mvNumNeurons[mNumLayers - 1]; cntPrevNeuron++, curWeights++)
        {
            *curWeights = mvAlpha.size() - mSupportPoints.GetDimOutput() + cntCurrentNeuron;
        }
        *curBias = mvAlpha.size() - mSupportPoints.GetDimOutput() + cntCurrentNeuron;
    }
}


void NuTo::NeuralNetwork::GetRefsPerAlpha(std::vector<int>& rRefsPerAlpha) const
{
    std::vector<int> posInAlphaVector;
    GetPosInAlphaVector(posInAlphaVector);
    rRefsPerAlpha.resize(mvAlpha.size());
    for (int pos : posInAlphaVector)
    {
        rRefsPerAlpha[pos]++;
    }
}


void NuTo::NeuralNetwork::dLnDetA_dBeta(Eigen::MatrixXd& rHessianInv, Eigen::MatrixXd& rResult) const
{
    int numOutputs(mSupportPoints.GetDimOutput());
    int numParameters = GetNumParameters();
    int numNeurons = GetNumNeurons();
    Eigen::MatrixXd totalJacobian(numOutputs * mSupportPoints.GetNumSupportPoints(), numParameters);
    Eigen::MatrixXd tmpJacobian(numOutputs, numParameters);
    Eigen::MatrixXd dHessian_dbetaij(numParameters, numParameters);

    std::vector<double> pA(numNeurons); // store the current value of each neuron before the transfer function
    std::vector<double> pO(numNeurons); // store the current value of each neuron after the transfer function

    Eigen::MatrixXd pM;
    rResult.resize(numOutputs, numOutputs);

    // precalculate the Jacobian for all samples
    for (int theSample = 0; theSample < mSupportPoints.GetNumSupportPoints(); theSample++)
    {
        // set input of input layer
        memcpy(&(pO[0]),
               &(mSupportPoints.GetTransformedSupportPointsInput().data()[theSample * mSupportPoints.GetDimInput()]),
               mSupportPoints.GetDimInput() * sizeof(double));
        // calculate the Jacobian
        Jacobian(tmpJacobian, pA, pO, pM);
        totalJacobian.block(numOutputs * theSample, 0, numOutputs, numParameters) = tmpJacobian;
    }

    for (int i = 0; i < numOutputs; i++)
    {
        for (int j = 0; j <= i; j++)
        {
            dHessian_dbetaij.setZero(numParameters, numParameters);
            for (int theSample = 0; theSample < mSupportPoints.GetNumSupportPoints(); theSample++)
            {
                dHessian_dbetaij += 0.5 * mSupportPoints.GetWeight(theSample) *
                                    (totalJacobian.row(theSample * numOutputs + i).transpose() *
                                             (totalJacobian.row(theSample * numOutputs + j)) +
                                     totalJacobian.row(theSample * numOutputs + j).transpose() *
                                             (totalJacobian.row(theSample * numOutputs + i)));
            }
            rResult(i, j) = (rHessianInv * dHessian_dbetaij).trace();
        }
    }

    // copy upper triangular matrix
    for (int i = 0; i < numOutputs; i++)
    {
        for (int j = i + 1; j < numOutputs; j++)
        {
            rResult(i, j) = rResult(j, i);
        }
    }
}

// get inverse covariance matrix
void NuTo::NeuralNetwork::GetInverseNoiseCovarianceMatrixTransformed(Eigen::MatrixXd& rInverseCovariance) const
{
    rInverseCovariance = this->mvCovarianceInv;
}

// get covariance matrix
void NuTo::NeuralNetwork::GetNoiseCovarianceMatrixTransformed(Eigen::MatrixXd& rCovariance) const
{
    Eigen::MatrixXd mvCovariance;
    mvCovariance = mvCovarianceInv.inverse();
    rCovariance = mvCovariance;
}

// get noise correlation matrix
void NuTo::NeuralNetwork::GetNoiseCorrelationMatrix(Eigen::MatrixXd& rNoiseCorrelation) const
{
    // calculate noise covariance matrix (transformed)
    Eigen::MatrixXd noiseCovarianceMatrix;
    noiseCovarianceMatrix = mvCovarianceInv.inverse();

    // calculate standard deviation of noise
    Eigen::VectorXd noiseStdDev = noiseCovarianceMatrix.diagonal().array().sqrt().matrix();

    // calculate correlation matrix
    rNoiseCorrelation.resize(this->mSupportPoints.GetDimOutput(), this->mSupportPoints.GetDimOutput());
    for (int col = 0; col < this->mSupportPoints.GetDimOutput(); col++)
    {
        for (int row = 0; row < this->mSupportPoints.GetDimOutput(); row++)
        {
            rNoiseCorrelation(row, col) = noiseCovarianceMatrix(row, col) / (noiseStdDev[row] * noiseStdDev[col]);
        }
    }
}

// get precision parameters
void NuTo::NeuralNetwork::GetPrecisionParametersTransformed(Eigen::MatrixXd& rPrecisionParameters) const
{
    rPrecisionParameters.resize(this->mvAlpha.size(), 1);
    for (unsigned int count = 0; count < this->mvAlpha.size(); count++)
    {
        rPrecisionParameters(count, 0) = this->mvAlpha[count];
    }
}
