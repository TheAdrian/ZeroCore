///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef LISTENERNODE_H
#define LISTENERNODE_H

namespace Audio
{
  //------------------------------------------------------------------- World Listener Position Info

  // Stores position and velocity information for ListenerNode
  struct ListenerWorldPositionInfo
  {
    ListenerWorldPositionInfo() :
      Position(Math::Vec3(0, 0, 0)),
      Velocity(Math::Vec3(0, 0, 0))
    {}
    ListenerWorldPositionInfo(const Math::Vec3& position, const Math::Vec3& velocity, const Math::Mat3& worldMatrix) :
      Position(position),
      Velocity(velocity),
      WorldMatrix(worldMatrix)
    {}

    Math::Vec3 Position;
    Math::Vec3 Velocity;
    Math::Mat3 WorldMatrix;
  };

  //---------------------------------------------------------------------------------- Listener Node

  class ListenerNodeData;

  class ListenerNode : public SimpleCollapseNode
  {
  public:
    ListenerNode(Zero::StringParam name, unsigned ID, ListenerWorldPositionInfo positionInfo, 
      ExternalNodeInterface* extInt, bool isThreaded = false);

    // Updates the position and velocity of the listener
    void SetPositionData(ListenerWorldPositionInfo positionInfo);
    // Sets whether this listener hears output or not (still processes sounds when inactive)
    void SetActive(const bool active);
    // Returns true if currently active
    bool GetActive();
    // Returns the current scale modifier applied to all attenuation heard by this listener
    float GetAttenuationScale();
    // Sets the scale modifier applied to all attenuation heard by this listener
    void SetAttenuationScale(float scale);

  private:
    ~ListenerNode();

    bool GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
      ListenerNode* listener, const bool firstRequest) override;
    // Gets the relative position of this listener
    Math::Vec3 GetRelativePosition(Math::Vec3Param otherPosition);
    // Gets the relative velocity of this listener
    Math::Vec3 GetRelativeVelocity(Math::Vec3Param otherVelocity);
    // Gets the relative facing direction of this listener
    Math::Vec3 GetRelativeFacing(Math::Vec3Param facingDirection);

    // Data used by the threaded node
    ListenerNodeData* ThreadedData;
    // If false, listener will not pass on output
    bool Active;
    // The scale applied to attenuation heard by this listener
    float mAttenuationScale;

    friend class EmitterNode;
    friend class AttenuatorNode;
  };

}

#endif
