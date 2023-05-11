#pragma once

#include <JuceHeader.h>
#include "AffineComponent.h"
#include "DESComponent.h"
#include "StreamComponent.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    //==============================================================================
    // Your private member variables go here...
    int width = 1200, height = 800;
    juce::TabbedComponent tabbed{juce::TabbedButtonBar::TabsAtTop};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
