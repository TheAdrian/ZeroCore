///////////////////////////////////////////////////////////////////////////////
///
/// Author: Andrea Ellinger
/// Copyright 2017, DigiPen Institute of Technology
///
///////////////////////////////////////////////////////////////////////////////

#include "Precompiled.h"

namespace Audio
{
  //--------------------------------------------------------------------------------- Equalizer Node

  //************************************************************************************************
  EqualizerNode::EqualizerNode(Zero::StringParam name, const unsigned ID, 
      ExternalNodeInterface* extInt, const bool isThreaded) :
    SimpleCollapseNode(name, ID, extInt, false, false, isThreaded),
    LowPassGain(1.0f),
    HighPassGain(1.0f),
    Band1Gain(1.0f),
    Band2Gain(1.0f),
    Band3Gain(1.0f)
  {
    if (!Threaded)
      SetSiblingNodes(new EqualizerNode(name, ID, nullptr, true));
  }

  //************************************************************************************************
  EqualizerNode::~EqualizerNode()
  {
    forRange(Equalizer* filter, FiltersPerListener.Values())
      delete filter;
  }

  //************************************************************************************************
  float EqualizerNode::GetBelow80HzGain()
  {
    return LowPassGain;
  }

  //************************************************************************************************
  void EqualizerNode::SetBelow80HzGain(const float gain)
  {
    LowPassGain = gain;

    if (!Threaded)
    {
      AddTaskForSibling(&EqualizerNode::SetBelow80HzGain, gain);
    }
    else
    {
      forRange(Equalizer* filter, FiltersPerListener.Values())
        filter->SetBelow80HzGain(gain);
    }
  }

  //************************************************************************************************
  float EqualizerNode::GetAbove5000HzGain()
  {
    return HighPassGain;
  }

  //************************************************************************************************
  void EqualizerNode::SetAbove5000HzGain(const float gain)
  {
    HighPassGain = gain;

    if (!Threaded)
    {
      AddTaskForSibling(&EqualizerNode::SetAbove5000HzGain, gain);
    }
    else
    {
      forRange(Equalizer* filter, FiltersPerListener.Values())
        filter->SetAbove5000HzGain(gain);
    }
  }

  //************************************************************************************************
  float EqualizerNode::Get150HzGain()
  {
    return Band1Gain;
  }

  //************************************************************************************************
  void EqualizerNode::Set150HzGain(const float gain)
  {
    Band1Gain = gain;

    if (!Threaded)
    {
      AddTaskForSibling(&EqualizerNode::Set150HzGain, gain);
    }
    else
    {
      forRange(Equalizer* filter, FiltersPerListener.Values())
        filter->Set150HzGain(gain);
    }
  }

  //************************************************************************************************
  float EqualizerNode::Get600HzGain()
  {
    return Band2Gain;
  }

  //************************************************************************************************
  void EqualizerNode::Set600HzGain(const float gain)
  {
    Band2Gain = gain;

    if (!Threaded)
    {
      AddTaskForSibling(&EqualizerNode::Set600HzGain, gain);
    }
    else
    {
      forRange(Equalizer* filter, FiltersPerListener.Values())
        filter->Set600HzGain(gain);
    }
  }

  //************************************************************************************************
  float EqualizerNode::Get2500HzGain()
  {
    return Band3Gain;
  }

  //************************************************************************************************
  void EqualizerNode::Set2500HzGain(const float gain)
  {
    Band3Gain = gain;

    if (!Threaded)
    {
      AddTaskForSibling(&EqualizerNode::Set2500HzGain, gain);
    }
    else
    {
      forRange(Equalizer* filter, FiltersPerListener.Values())
        filter->Set2500HzGain(gain);
    }
  }

  //************************************************************************************************
  void EqualizerNode::InterpolateBands(const EqualizerBandGains gainValues, const float timeToInterpolate)
  {
    LowPassGain = gainValues.Below80Hz;
    HighPassGain = gainValues.Above5000Hz;
    Band1Gain = gainValues.At150Hz;
    Band2Gain = gainValues.At600Hz;
    Band3Gain = gainValues.At2500Hz;

    if (!Threaded)
    {
      AddTaskForSibling(&EqualizerNode::InterpolateBands, gainValues, timeToInterpolate);
    }
    else
    {
      forRange(Equalizer* filter, FiltersPerListener.Values())
        filter->InterpolateBands(LowPassGain, Band1Gain, Band2Gain, Band3Gain, HighPassGain,
          timeToInterpolate);
    }
  }

  //************************************************************************************************
  bool EqualizerNode::GetOutputSamples(BufferType* outputBuffer, const unsigned numberOfChannels,
    ListenerNode* listener, const bool firstRequest)
  {
    if (!Threaded)
      return false;

    unsigned bufferSize = outputBuffer->Size();

    // Get input and return if there is no data
    if (!AccumulateInputSamples(bufferSize, numberOfChannels, listener))
      return false;

    // Check if the listener is in the map
    Equalizer* filter = FiltersPerListener.FindValue(listener, nullptr);
    if (!filter)
    {
      filter = new Equalizer;
      filter->SetBelow80HzGain(LowPassGain);
      filter->SetAbove5000HzGain(HighPassGain);
      filter->Set150HzGain(Band1Gain);
      filter->Set600HzGain(Band2Gain);
      filter->Set2500HzGain(Band3Gain);
      FiltersPerListener[listener] = filter;
    }

    // Apply filter
    filter->ProcessBuffer(InputSamples.Data(), outputBuffer->Data(), numberOfChannels, bufferSize);

    AddBypass(outputBuffer);

    return true;
  }

  //************************************************************************************************
  void EqualizerNode::RemoveListener(ListenerNode* listener)
  {
    if (!Threaded)
      return;

    if (FiltersPerListener.FindValue(listener, nullptr))
    {
      // If there is still another listener, we need to combine the history of the filters to avoid clicks
      if (FiltersPerListener.Size() > 1)
      {
        Equalizer* filter = FiltersPerListener[listener];
        FiltersPerListener.Erase(listener);
        FilterMapType::valuerange range = FiltersPerListener.All();
        filter->MergeWith(*range.Front());
      }

      delete FiltersPerListener[listener];
      FiltersPerListener.Erase(listener);
    }
  }

}