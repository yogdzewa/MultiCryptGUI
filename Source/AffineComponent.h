/*
  ==============================================================================

	AffineComponent.h
	Created: 5 May 2023 11:03:44pm
	Author:  L

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Affine.h"
#include "common.h"

//==============================================================================
/*
*/
class AffineComponent : public juce::Component
{
public:
	AffineComponent()
	{
		// In your constructor, you should add any child components, and
		// initialise any special settings that your component needs.
		affineptr.reset(new Affine(1, 1));
		a = b = 1;

		addAndMakeVisible(leftGroupComponent);
		addAndMakeVisible(middleGroupComponent);
		addAndMakeVisible(rightGroupComponent);

		setButtonCallback();
	}

	void setButtonCallback() {
		auto& encButton = middleGroupComponent.encButton;
		auto& decButton = middleGroupComponent.decButton;
		auto& leftText = leftGroupComponent.textEditor,
			& rightText = rightGroupComponent.textEditor;

		encButton.onClick = [&] {
			auto plainText = leftText.getText().toStdString();
			bytes buf(stringToBytes(plainText));

			auto multiplier = leftGroupComponent.multiplier.getText().getIntValue();
			auto offset = leftGroupComponent.offset.getText().getIntValue();

			if (multiplier != a || offset != b) {
				affineptr.reset(new Affine(multiplier, offset));
			}

			buf = affineptr->encryptBytes(buf);
			rightGroupComponent.textEditor.setText(juce::String(reinterpret_cast<const char*>(buf.data()), buf.size()));
		};

		decButton.onClick = [&] {
			auto cipherText = rightText.getText().toStdString();
			bytes buf(stringToBytes(cipherText));
			buf = affineptr->decryptBytes(buf);
			leftText.setText(juce::String((const char*)(buf.data()), buf.size()));
		};
	}

	void paint(juce::Graphics& g) override
	{
		g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));   // clear the background

		g.setColour(juce::Colours::grey);
		g.drawRect(getLocalBounds(), 1);   // draw an outline around the component
	}

	void resized() override
	{
		juce::Grid grid;

		using Track = juce::Grid::TrackInfo;
		using Fr = juce::Grid::Fr;

		grid.columnGap = juce::Grid::Px(6);
		grid.templateRows = { Track(Fr(1)) };
		grid.templateColumns = { Track(Fr(3)), Track(Fr(1)), Track(Fr(3)) };
		grid.items = { juce::GridItem(leftGroupComponent),
				juce::GridItem(middleGroupComponent).withHeight(150).withAlignSelf(juce::GridItem::AlignSelf::center),
				juce::GridItem(rightGroupComponent) };

		grid.performLayout(getLocalBounds().reduced(6));
	}

private:

	struct LeftGroupComponent : public juce::GroupComponent
	{
		LeftGroupComponent() {
			setText("Plain Text");
			addAndMakeVisible(textEditor);
			addAndMakeVisible(multiplier);
			addAndMakeVisible(offset);

			multiplier.setEditable(true);
			offset.setEditable(true);
			multiplier.setColour(juce::Label::outlineColourId, juce::Colours::white);
			offset.setColour(juce::Label::outlineColourId, juce::Colours::white);

			juce::Font font{ "Consolas", 18.0f, juce::Font::plain };
			textEditor.setFont(font);
			textEditor.setText("input plain text here");
			textEditor.setMultiLine(true);
		}

		void resized() override {
			auto b = getLocalBounds().withTrimmedTop(10).reduced(10);
			offset.setBounds(b.removeFromBottom(50).reduced(5));
			multiplier.setBounds(b.removeFromBottom(50).reduced(5));
			textEditor.setBounds(b.reduced(5));
		}

		juce::Label multiplier{ {}, "1" };
		juce::Label offset{ {}, "1" };
		juce::TextEditor textEditor;
	};

	struct MiddleGroupComponent : public juce::GroupComponent
	{
		MiddleGroupComponent() {
			setText("Operations");
			addAndMakeVisible(encButton);
			addAndMakeVisible(decButton);
			encButton.setButtonText("ENCRYPT >");
			decButton.setButtonText("DECRYPT <");
		}

		void resized() override {
			juce::Grid g;
			using Track = juce::Grid::TrackInfo;
			using Fr = juce::Grid::Fr;
			g.templateRows = { Track(Fr(1)), Track(Fr(1)) };
			g.templateColumns = { Track(Fr(1)) };
			g.items = { juce::GridItem(encButton),
				juce::GridItem(decButton) };
			g.setGap(juce::Grid::Px(5));
			g.performLayout(getLocalBounds().withTrimmedTop(8).reduced(10));
		}

		juce::TextButton encButton, decButton;
	};

	struct RightGroupComponent : public juce::GroupComponent
	{
		RightGroupComponent() {
			setText("Cipher Text");
			addAndMakeVisible(textEditor);

			textEditor.setMultiLine(true);
			juce::Font font{ "Consolas", 18.0f, juce::Font::plain };
			textEditor.setFont(font);
			textEditor.setText("input cipher text here");
		}

		void resized() {
			auto b = getLocalBounds().reduced(15).withTrimmedTop(8);
			textEditor.setBounds(b);
		}

		juce::TextEditor textEditor;

	};

	std::unique_ptr<Affine> affineptr;
	int a, b;

	LeftGroupComponent leftGroupComponent;
	MiddleGroupComponent middleGroupComponent;
	RightGroupComponent rightGroupComponent;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AffineComponent)
};
