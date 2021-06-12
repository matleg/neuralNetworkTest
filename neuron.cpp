#include "neuron.h"
#include "connection.h"

#include <QList>

double Neuron::sEta = 0.4;    // overall net learning rate, [0.0..1.0]
double Neuron::sAlpha = 0.4;   // momentum, multiplier of last deltaWeight, [0.0..1.0]
int Neuron::sNeuronId = 0;

std::random_device rd{};
std::mt19937 gen(rd());

Neuron::Neuron()
{
    mId = sNeuronId++;
}

Neuron::Neuron(const QHash<int, Connection *> &connections)
{
    mConnections = connections;
    setRandomWeight();
    mId = sNeuronId++;
}

void Neuron::setConnections(const QHash<int, Connection *> &connections)
{
    mConnections = connections;
    setRandomWeight();
}

Neuron::~Neuron()
{
    QList<Connection *> connections = mConnections.values();
    for (int i = 0; i < connections.size(); ++i )
    {
        delete connections[i];
    }
    mConnections.clear();
}

void Neuron::updateInputWeights(Layer &prevLayer)
{
    for (int n = 0; n < prevLayer.size(); ++n)
    {
        Neuron *neuron = prevLayer[n];
        if (neuron->mConnections.contains(mId))
        {
            double newDeltaWeight;
            newDeltaWeight = - sEta * neuron->outputVal() * mGradient + sAlpha * neuron->mConnections[mId]->momentum() ;
            neuron->mConnections[mId]->setMomentum(newDeltaWeight);
            neuron->mConnections[mId]->setWeight(newDeltaWeight + neuron->mConnections[mId]->weight());
        }
    }
}

void Neuron::setRandomWeight(void)
{
    if (mConnections.size() == 0)
        return ;
    double interval = 1.0 / sqrt(mConnections.size());
    std::uniform_real_distribution<> dist(-interval, interval);
    for (int i = 0; i < mConnections.size(); i++)
    {
        mConnections[i]->setWeight(dist(gen));
        mConnections[i]->setMomentum(0);
    }
}

double Neuron::sumDOW(const Layer &nextLayer) const
{
    double sum = 0.0;

    for (int n = 0; n < nextLayer.size() - 1; ++n)
    {
        if (mConnections.contains(nextLayer[n]->id()))
        {
            sum += mConnections[n]->weight() * nextLayer[n]->mGradient;
        }
    }
    return sum;
}

void Neuron::calcHiddenGradients(const Layer &nextLayer)
{
    double dow = sumDOW(nextLayer);
    mGradient = dow * Neuron::transferFunctionDerivative();
}

void Neuron::calcOutputGradients(double targetVal)
{
    double delta =  - (targetVal - mOutputVal) ; // dError/dOutput (( 1/2(targetVal - mOutputVal) ^2)')
    mGradient = delta * Neuron::transferFunctionDerivative();
}

double Neuron::transferFunction()
{
    double res = 0;
    if (mFunction == ActivationFunction::TanH)
    {
        res = tanh(mInputVal);
    }
    return res;
}

double Neuron::transferFunctionDerivative()
{
    double res = 0;
    if (mFunction == ActivationFunction::TanH)
    {
        res = 1.0 - tanh(mInputVal) * tanh(mInputVal);
    }
    return res;
}

void Neuron::feedForward(const Layer &prevLayer)
{
    double sum = 0.0;
    for (int n = 0; n < prevLayer.size(); ++n)
    {
        if (prevLayer[n]->mConnections.contains(mId))
        {
            sum += prevLayer[n]->outputVal() * prevLayer[n]->mConnections[mId]->weight();
        }
    }
    mInputVal = sum;
    mOutputVal = Neuron::transferFunction();
}

double Neuron::outputVal() const
{
    return mOutputVal;
}

void Neuron::setOutputVal(double newOutputVal)
{
    mOutputVal = newOutputVal;
}

double Neuron::inputVal() const
{
    return mInputVal;
}

void Neuron::setInputVal(double newInputVal)
{
    mInputVal = newInputVal;
}

double Neuron::gradient() const
{
    return mGradient;
}

void Neuron::setGradient(double newGradient)
{
    mGradient = newGradient;
}

int Neuron::id() const
{
    return mId;
}

void Neuron::setId(int newId)
{
    mId = newId;
}

