#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
	setSize(width, height);
	addAndMakeVisible(tabbed);
	tabbed.setSize(getWidth(), getHeight());
	tabbed.setColour(juce::TabbedButtonBar::tabTextColourId, juce::Colours::grey);

	tabbed.addTab("DES", juce::Colours::darkgrey, new DESComponent(), false);
	tabbed.addTab("Affine", juce::Colours::darkgrey, new AffineComponent(), false);

}

MainComponent::~MainComponent() {}

//==============================================================================
void MainComponent::paint(juce::Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	//g.setColour(juce::Colours::green);
	//g.drawLine(10, 300, 590, 300, 5);
}


void MainComponent::resized()
{
	auto b = getLocalBounds();
	tabbed.setBounds(b);
}
