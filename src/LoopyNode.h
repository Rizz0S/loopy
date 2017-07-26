
#ifndef LOOPY_NODE
#define LOOPY_NODE

#import <opencv2/opencv.hpp>
#import <functional>
#include <vector>
#include <string>
#include <map>

class LoopyNode;

typedef std::function<cv::Mat (std::map<std::string, LoopyNode *>)> LoopyFunction;


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
    std::string parameterName;

    InputConnection(LoopyNode *node, std::string parameterName) : InputConnection(node, parameterName, true) {}

    InputConnection(LoopyNode *node, std::string parameterName, bool enforceOnFirstRun) {
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
    std::map<std::string, LoopyNode*>inputs;

    /**
     * A mapping from LoopyNode outputKeys to InputConnection parameterNames
     */
    std::map<std::string, std::string>inputNameMapping;

    /**
     * A vector that stores all of this node's InputConnections.
     */
    std::vector<InputConnection> inputConnections;

    /**
     * Child nodes that receive this node's output when it's ready.
     */
    std::vector<LoopyNode *>outputReceivers;

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
     * processFn is called whenever
     * a LoopyNode has received all of its inputs.
     */
    LoopyFunction processFn;

    /**
     * Iterate through all outputReceivers and call inputReady using this node's key.
     */
    void notifyReceivers();

public:
    /**
     * A key that identifies this node in the graph.
     */
    std::string outputKey;

    /**
     * inputConnections: a vector of InputConnections to this node.
     * outputKey: this node's output key.
     */
    LoopyNode(std::vector<InputConnection> inputConnections, std::string outputKey) {
        outputIterations = 0;
        this->outputKey = outputKey;
        for (size_t i = 0; i < inputConnections.size(); ++i) {
            addInput(inputConnections[i]);
        }
    }

    /**
     * Constructor with no InputConnections
     */
    LoopyNode(std::string outputKey) : LoopyNode(std::vector<InputConnection>(), outputKey) {}

    /**
     * Add an InputConnection
     */
    void addInput(InputConnection ic);

    /**
     * Other nodes will call this function when they have finished processing
     * and they are ready to give this node their output as input.
     */
    void inputReady(LoopyNode* node);

    const cv::Mat& getOutput() const { return output; }

    /**
     * Set this node's processing function.
     * A LoopyFunction is any function pointer that takes a map from parameterName strings to LoopyNode pointers.
     * When all InputConnections attached to this node have finished processing,
     * processFn gets called with the inputs map defined above.
     */
    void setProcessFunction(LoopyFunction pfn) {
        processFn = pfn;
    }
};

#endif
