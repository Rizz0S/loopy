#ifndef LOOPY_NODE
#define LOOPY_NODE

#import <opencv2/opencv.hpp>
#import <functional>
#include <vector>
#include <string>
#include <map>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

class LoopyNode;

typedef function<cv::Mat (map<string, LoopyNode *>)> LoopyFunction;
typedef map<string, LoopyNode *> LoopyFunctionInput;


/**
 * An input connection represents an incoming edge to a node.
 * If enforceOnFirstRun is false then a node with this connection doesn't need to wait
 * for inputNode to have valid output on the first iteration.
 *
 * parameterName is this name of this connection inside a node's LoopyFunction.
 * This is kind of dumb but it makes LoopyFunction reuse a bit easier until I find a better way
 * to handle InputConnections.
 */
struct InputConnection
{
    LoopyNode *inputNode;
    bool enforceOnFirstRun;
    string parameterName;

    InputConnection(LoopyNode *node, string parameterName) : InputConnection(node, parameterName, true)
    {
    }

    InputConnection(LoopyNode *node, string parameterName, bool enforceOnFirstRun)
    {
        this->inputNode = node;
        this->enforceOnFirstRun = enforceOnFirstRun;
        this->parameterName = parameterName;
    }
};

/**
 */
class LoopyNode
{

private:
    //Check if all this node's inputs are ready to be used
    bool allInputsReady();

protected:
    /**
     * A map from InputConnection parameterNames to LoopyNodes.
     * This stores all a node's inputs
     */
    map<string, LoopyNode*>inputs;

    /**
     * A mapping from LoopyNode outputKeys to InputConnection parameterNames
     */
    map<string, vector<string>>inputNameMapping;

    /**
     * A vector that stores all of this node's InputConnections.
     */
    vector<InputConnection> inputConnections;

    /**
     * Child nodes that receive this node's output when it's ready.
     */
    vector<LoopyNode *>outputReceivers;

    /**
     * A matrix that represents this node's output.
     */
    cv::Mat output;

    /**
     * The number of processing iterations this node has performed.
     * At some point it would be cool to expose this as a parameter inside LoopyFunctions
     * so that output can depend on how long the graph has been running.
     */
    int outputIterations;

    /**
     * Iterate through all outputReceivers and call inputReady using this node's key.
     */
    void notifyReceivers();

    float getFloatParam(string paramName);

    float getFloatParam(string paramName, float defaultValue);

    bool getBoolParam(string paramName);

    bool getBoolParam(string paramName, bool defaultValue);

    int getIntParam(string paramName);

    int getIntParam(string paramName, int defaultValue);

public:
    /**
     * A key that identifies this node in the graph.
     */
    string outputKey;

    /**
     * This generates keys automatically for LoopyNodes
     */
    static int nextId;

    /**
     * inputConnections: a vector of InputConnections to this node.
     * outputKey: this node's output key.
     */
    LoopyNode(vector<InputConnection> inputConnections, string outputKey)
    {
        outputIterations = 0;
        this->outputKey = outputKey;
        for (size_t i = 0; i < inputConnections.size(); ++i) {
            addInput(inputConnections[i]);
        }
    }

    virtual ~LoopyNode();

    /**
     * Constructor with no InputConnections
     */
    LoopyNode(string outputKey) : LoopyNode(vector<InputConnection>(), outputKey) 
    {
    }

    LoopyNode() : LoopyNode(vector<InputConnection>(), to_string(LoopyNode::nextId++))
    {
    }

    /**
     * Add an InputConnection
     */
    void addInput(InputConnection ic);

    void clearInputs()
    {
        outputReceivers.clear();
        inputConnections.clear();
        inputNameMapping.clear();
    }

    /**
     * add a new node as input with a parameter name and a boolean representing whether
     * this node should wait for the input node's output before running the first iteration
     */
    void addInput(LoopyNode *node, string parameterName, bool enforceOnFirstRun) {
        addInput(InputConnection(node, parameterName, enforceOnFirstRun));
    }

    /**
     * Other nodes will call this function when they have finished processing
     * and they are ready to give this node their output as input.
     */
    void inputReady(LoopyNode* node);

    const cv::Mat& getOutput() const
    { 
        return output;
    }

    /**
     * this node's processing function.
     * takes a map from parameterName strings to LoopyNode pointers.
     * When all InputConnections attached to this node have finished processing,
     * this gets called with the inputs map defined above.
     */

    virtual cv::Mat process(LoopyFunctionInput inputs) = 0;
};

#endif
