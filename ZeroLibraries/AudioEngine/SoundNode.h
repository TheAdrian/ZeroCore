///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Audio
{
  class ListenerNode;
  class ExternalNodeInterface;
  class EventData;

  //------------------------------------------------------------------------------------- Sound Node

  class SoundNode 
  {
  public:
    SoundNode(Zero::StringParam name, const unsigned ID, ExternalNodeInterface* extInt,
      const bool listenerDependent, const bool generator, const bool isThreaded);

    // ***** Virtual functions that CAN be overridden, but usually shouldn't. Be careful. *****

    // Adds a new input to this node and adds this as an output to other node
    virtual void AddInput(SoundNode* newNode);
    // Removes an input from this node and removes this from output of other node
    // Checks for collapsing and for deleting unreferenced nodes
    virtual void RemoveInput(SoundNode* node);
    // Adds a new node before this node in the signal path
    virtual void InsertNodeBefore(SoundNode* newNode);
    // Adds a new node after this node in the signal path
    virtual void InsertNodeAfter(SoundNode* newNode);
    // Replaces this node in the graph with a different node
    virtual void ReplaceWith(SoundNode* replacementNode);
    // Disconnects all input nodes
    virtual void DisconnectInputs();
    // Disconnects all output nodes
    virtual void DisconnectOutputs();
    // Disconnects this node from all other nodes, keeping the signal path
    virtual void DisconnectOnlyThis();
    // Disconnects this node from all other nodes, and disconnects all inputs also
    virtual void DisconnectThisAndAllInputs();
    // Sets external interface to null and deletes the node
    // Disconnects from other nodes if necessary
    virtual void DeleteThisNode();

    // Returns a pointer to the array of all sound node inputs
    const Zero::Array<SoundNode*>* GetInputs();
    // Returns a pointer to the array of all sound node outputs
    const Zero::Array<SoundNode*>* GetOutputs();
    // Returns true if this node has inputs
    bool HasInputs();
    // Returns true if this node has outputs
    bool HasOutputs();
    // Sets the external interface pointer
    void SetExternalInterface(ExternalNodeInterface* externalData);
    // Returns whether or not this node should collapse when inputs are removed
    bool GetCollapse();
    // Sets whether or not this node should collapse when inputs are removed
    void SetCollapse(const bool shouldCollapse);
    // Return true if this sound node is currently outputting audio data
    bool HasAudibleOutput();
    // Returns the current bypass value for this sound node (0 - 1.0)
    float GetBypassValue();
    // Sets the bypass value (fraction of output that is not altered by this node) (0 - 1.0)
    void SetBypassValue(const float bypassValue);

    // The name given to this sound node by the external system when it was constructed
    const Zero::String Name;
    // The ID given to this sound node by the external system when it was constructed
    const unsigned NodeID;
    

    // ***** These must be public but should not be used outside of this system *****

    // Sends an audio event to the external interface
    void SendEventToExternalData(AudioEventTypes::Enum eventType);
    void SendEventDataToExternalData(EventData* data);
    // Should be implemented by nodes if they keep track of data per listeners
    virtual void RemoveListener(ListenerNode* listener) {}
    // Returns the sum of all volumes from outputs. Will return 0.0 by default. The output
    // node will return 1.0. Nodes which modify volume should implement this function
    // and multiply their volume with the return value.
    virtual float GetVolumeChangeFromOutputs();
    // Used for InList
    Zero::Link<SoundNode> link;

  protected:
    virtual ~SoundNode();
    // Handles getting the output from the passed-in sound node
    bool Evaluate(BufferType* outputBuffer, const unsigned numberOfChannels, ListenerNode* listener);
    // Adds the output from all input nodes to the InputSamples buffer
    bool AccumulateInputSamples(const unsigned howManySamples, const unsigned numberOfChannels,
      ListenerNode* listener);
    // Returns true if there is an external interface attached to this node
    bool HasExternalInterface();
    // Get this node's threaded or non-threaded counterpart
    SoundNode* GetSiblingNode();
    // Sets the sibling node variables for both this node and its threaded counterpart
    void SetSiblingNodes(SoundNode* threadedNode);
    // Returns the pointer to the node's external interface
    ExternalNodeInterface* GetExternalInterface() { return ExternalData; }
    // Uses the BypassValue to add a portion of the InputSamples buffer to the passed-in buffer
    void AddBypass(BufferType* outputBuffer);

    // If true, this node is running on the mix thread. If false, it is on the game thread.
    const bool Threaded;
    // A buffer to hold the output of all input nodes
    BufferType InputSamples;

  private:
    // Must be implemented to provide the output of this sound node
    virtual bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) = 0;
    // Called on the non-threaded node when the last input is removed
    virtual void CollapseNode() = 0;

    // Checks if a node is disconnected and should be deleted
    void CheckIfDisconnected();
    // Called on the non-threaded node when the threaded node is deleted
    void OkayToDelete();

    // If false, this node's output should not be saved into the MixedOutput buffer
    bool OkayToSave;
    // If true, this node should be deleted when it has no external interface
    bool DeleteMe;
    // Pointer to external interface
    ExternalNodeInterface* ExternalData;
    // Array of nodes to use as inputs
    Zero::Array<SoundNode*> Inputs;
    // Array of nodes using this node as input
    Zero::Array<SoundNode*> Outputs;
    // Threaded or non-threaded node associated with this node
    SoundNode* SiblingNode;
    // If true, currently in process of getting output
    bool InProcess;
    // Version number of the mixed output
    unsigned Version;
    // Saved output for a mix version
    BufferType MixedOutput;
    // Number of channels in the mixed output
    unsigned NumMixedChannels;
    // The listener used for the mixed output
    ListenerNode* MixedListener;
    // If true, node will collapse when all inputs are removed
    bool Collapse;
    // If true, this node had valid audio output during the last mix
    bool ValidOutputLastMix;
    // If true, this node has listener-specific data (such as an emitter)
    bool ListenerDependent;
    // The fraction of output that shouldn't be affected by this node (0 - 1.0)
    float BypassValue;
    // If true, this is a node which generates audio
    bool Generator;
    
    friend class AudioSystemInternal;
    friend class AudioSystemInterface;

  protected:
    // Set a variable 
    template <typename InstanceType, typename MemberType>
    void AddTaskForSibling(MemberType InstanceType::*memberPointer, MemberType value)
    {
      if (SiblingNode)
        gAudioSystem->AddTask(Zero::CreateFunctor(memberPointer, (InstanceType*)SiblingNode, value));
    }

    // Set a variable from mix thread
    template <typename InstanceType, typename MemberType>
    void AddTaskForSiblingThreaded(MemberType InstanceType::*memberPointer, MemberType value)
    {
      if (SiblingNode)
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(memberPointer, (InstanceType*)SiblingNode, value));
    }

    // Call a function with no parameters
    template <typename ReturnType, typename InstanceType>
    void AddTaskForSibling(ReturnType(InstanceType::*functionPointer)())
    {
      if (SiblingNode)
        gAudioSystem->AddTask(Zero::CreateFunctor(functionPointer, (InstanceType*)SiblingNode));
    }

    // Call a function with no parameters from mix thread
    template <typename ReturnType, typename InstanceType>
    void AddTaskForSiblingThreaded(ReturnType(InstanceType::*functionPointer)())
    {
      if (SiblingNode)
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(functionPointer, (InstanceType*)SiblingNode));
    }

    // Call a function with one parameter
    template <typename ReturnType, typename InstanceType, typename P0>
    void AddTaskForSibling(ReturnType(InstanceType::*functionPointer)(P0), P0 p0)
    {
      if (SiblingNode)
        gAudioSystem->AddTask(Zero::CreateFunctor(functionPointer, (InstanceType*)SiblingNode, p0));
    }

    // Call a function with one parameter from mix thread
    template <typename ReturnType, typename InstanceType, typename P0>
    void AddTaskForSiblingThreaded(ReturnType(InstanceType::*functionPointer)(P0), P0 p0)
    {
      if (SiblingNode)
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(functionPointer, (InstanceType*)SiblingNode, p0));
    }

    // Call a function with two parameters
    template <typename ReturnType, typename InstanceType, typename P0, typename P1>
    void AddTaskForSibling(ReturnType(InstanceType::*functionPointer)(P0, P1), P0 p0, P1 p1)
    {
      if (SiblingNode)
        gAudioSystem->AddTask(Zero::CreateFunctor(functionPointer, (InstanceType*)SiblingNode, p0, p1));
    }

    // Call a function with two parameters from mix thread
    template <typename ReturnType, typename InstanceType, typename P0, typename P1>
    void AddTaskForSiblingThreaded(ReturnType(InstanceType::*functionPointer)(P0, P1), P0 p0, P1 p1)
    {
      if (SiblingNode)
        gAudioSystem->AddTaskThreaded(Zero::CreateFunctor(functionPointer, (InstanceType*)SiblingNode, p0, p1));
    }

    // Call a function with three parameters
    template <typename ReturnType, typename InstanceType, typename P0, typename P1, typename P2>
    void AddTaskForSibling(ReturnType(InstanceType::*functionPointer)(P0, P1, P2), P0 p0, P1 p1, P2 p2)
    {
      if (SiblingNode)
        gAudioSystem->AddTask(Zero::CreateFunctor(functionPointer, (InstanceType*)SiblingNode, p0, p1, p2));
    }
  };

  //------------------------------------------------------------------------------------ Output Node

  // Node used by audio system for final output
  class OutputNode : public SoundNode
  {
  public:
    OutputNode(Zero::StringParam name, ExternalNodeInterface* extInt, bool isThreaded);

    float GetVolumeChangeFromOutputs() override;

  private:
    ~OutputNode() {}
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;

    // None of these functions should be called on this node
    void CollapseNode() override {}
    void InsertNodeAfter(SoundNode* newNode) override {}
    void ReplaceWith(SoundNode* replacementNode) override {}
    void DisconnectOnlyThis() override {}
    void DisconnectThisAndAllInputs() override {}
    void DeleteThisNode() override {}

    friend class AudioSystemInternal;
  };

  //--------------------------------------------------------------------------- Simple Collapse Node

  // Class to derive from for simple collapse functionality
  class SimpleCollapseNode : public SoundNode
  {
  public:
    SimpleCollapseNode(Zero::StringParam name, unsigned ID, ExternalNodeInterface* extInt, 
        bool listenerDependent, bool generator, bool threaded) :
      SoundNode(name, ID, extInt, listenerDependent, generator, threaded) 
    {}

  protected:
    virtual ~SimpleCollapseNode() {}

  private:
    // Will disconnect and remove this node when all inputs are removed
    void CollapseNode() override;

    friend class SoundNode;
  };

  //----------------------------------------------------------------------------------- Combine Node

  class CombineNode : public SimpleCollapseNode
  {
  public:
    CombineNode(Zero::StringParam name, unsigned ID, ExternalNodeInterface* extInt, 
      bool isThreaded = false);

  private:
    ~CombineNode() {}
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;

  };

  //------------------------------------------------------------------------- Combine And Pause Node

  class CombineAndPauseNode : public SimpleCollapseNode
  {
  public:
    CombineAndPauseNode(Zero::StringParam name, unsigned ID, ExternalNodeInterface* extInt, 
      bool isThreaded = false);

    bool GetPaused();
    void SetPaused(const bool paused);
    bool GetMuted();
    void SetMuted(bool muted);

  private:
    ~CombineAndPauseNode() {}
    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;

    bool Paused;
    bool Pausing;
    bool Muted;
    bool Muting;
    InterpolatingObject VolumeInterpolator;
    bool Interpolating;
  };

}
