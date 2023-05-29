/*
  ==============================================================================

	Stream.h
	Created: 10 May 2023 4:51:50pm
	Author:  L

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RC4.h"
#include "LFSR_JK.h"

//==============================================================================
/*
*/
class StreamComponent : public juce::Component
{
public:
	StreamComponent()
	{
		addAndMakeVisible(leftGroupComponent);
		addAndMakeVisible(middleGroupComponent);
		addAndMakeVisible(rightGroupComponent);

		setButtonCallback();
	}

	bytes getKey(bytes input) {
		auto& genType = middleGroupComponent.genType;
		auto& keyLabel = leftGroupComponent.keyLabel,
			& keyLabel2 = leftGroupComponent.keyLabel2,
			& keyLabel3 = leftGroupComponent.keyLabel3,
			& keyLabel4 = leftGroupComponent.keyLabel4,
			& keyLabel5 = leftGroupComponent.keyLabel5;

		bytes keyBytes;
		if (genType.getToggleState()) {
			rc4_ptr.reset(new RC4());
			rc4_ptr->permuteSbox(stringToBytes(keyLabel.getText().toStdString()));
			keyBytes = rc4_ptr->generateRandBytes(input.size());
		}
		else {
			bits lfsr1_feedback, lfsr1_initial, lfsr2_feedback, lfsr2_initial;
			bit jk_initial;
			lfsr1_feedback = stringToBytes(keyLabel.getText().toStdString());
			lfsr1_initial = stringToBytes(keyLabel2.getText().toStdString());
			lfsr2_feedback = stringToBytes(keyLabel3.getText().toStdString());
			lfsr2_initial = stringToBytes(keyLabel4.getText().toStdString());
			jk_initial = keyLabel5.getText().getIntValue();

			lfsr_jk_ptr.reset(new LFSR_JK(lfsr1_feedback, lfsr1_initial, lfsr2_feedback, lfsr2_initial, jk_initial));
			keyBytes = lfsr_jk_ptr->keyStreamBytes(input.size());
		}
		return keyBytes;
	}

	void setButtonCallback() {
		auto& encButton = middleGroupComponent.encButton,
			& decButton = middleGroupComponent.decButton;
		auto& leftText = leftGroupComponent.textEditor,
			& rightText = rightGroupComponent.textEditor;

		encButton.onClick = [&] {
			auto plainText = leftText.getText().toStdString();
			bytes plainBytes(stringToBytes(plainText));
			auto keyText = leftGroupComponent.keyLabel.getText();

			bytes buf = xorBytes(getKey(plainBytes), plainBytes);
			rightGroupComponent.textEditor.setText(bytesToHexdump(buf));
		};

		decButton.onClick = [&] {
			auto cipherText = rightText.getText().toStdString();
			bytes cipherBytes(hexdumpToBytes(cipherText));

			bytes buf = xorBytes(getKey(cipherBytes), cipherBytes);
			leftGroupComponent.textEditor.setText(bytesToString(buf));
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
				juce::GridItem(middleGroupComponent).withHeight(180).withAlignSelf(juce::GridItem::AlignSelf::center),
				juce::GridItem(rightGroupComponent) };

		grid.performLayout(getLocalBounds().reduced(6));
	}

private:

	struct LeftGroupComponent : public juce::GroupComponent, public juce::FilenameComponentListener
	{
		LeftGroupComponent() {
			setText("Plain Text");
			addAndMakeVisible(textEditor);
			addAndMakeVisible(keyLabel);
			addAndMakeVisible(keyLabel2);
			addAndMakeVisible(keyLabel3);
			addAndMakeVisible(keyLabel4);
			addAndMakeVisible(keyLabel5);
			addAndMakeVisible(fileChooser);

			keyLabel.setEditable(true);
			keyLabel2.setEditable(true);
			keyLabel3.setEditable(true);
			keyLabel4.setEditable(true);
			keyLabel5.setEditable(true);
			keyLabel.setColour(juce::Label::outlineColourId, juce::Colours::white);

			juce::Font font{ "Consolas", 18.0f, juce::Font::plain };
			textEditor.setFont(font);
			textEditor.setText("input plain text here");
			textEditor.setMultiLine(true);
		}

		void filenameComponentChanged(juce::FilenameComponent* comp) {
			if (comp != &fileChooser)
				return;
			auto name = comp->getCurrentFile();
			juce::StringArray content;
			name.readLines(content);
			keyLabel.setText(content[0], juce::NotificationType::dontSendNotification);
		}

		void resized() override {
			auto b = getLocalBounds().withTrimmedTop(10).reduced(10);
			fileChooser.setBounds(b.removeFromBottom(40).reduced(5));
			keyLabel5.setBounds(b.removeFromBottom(30).reduced(5));
			keyLabel4.setBounds(b.removeFromBottom(30).reduced(5));
			keyLabel3.setBounds(b.removeFromBottom(30).reduced(5));
			keyLabel2.setBounds(b.removeFromBottom(30).reduced(5));
			keyLabel.setBounds(b.removeFromBottom(30).reduced(5));
			textEditor.setBounds(b.reduced(5));
		}

		juce::Label keyLabel{ {}, "10101010" };
		juce::Label keyLabel2{ {}, "10101010" };
		juce::Label keyLabel3{ {}, "10101010" };
		juce::Label keyLabel4{ {}, "10101010" };
		juce::Label keyLabel5{ {}, "10" };
		juce::TextEditor textEditor;
		juce::FilenameComponent fileChooser{ "File", {}, true, false, false, "*.txt;*.xml;*.yml;", {},
								"choose a file as key..." };
	};

	struct MiddleGroupComponent : public juce::GroupComponent
	{
		MiddleGroupComponent() {
			setText("Operations");
			addAndMakeVisible(encButton);
			addAndMakeVisible(decButton);
			addAndMakeVisible(genType);
			encButton.setButtonText("ENCRYPT >");
			decButton.setButtonText("DECRYPT <");
			genType.setButtonText("RC4(T) or LFSR-JK(F)");
			genType.setToggleState(true, juce::NotificationType::dontSendNotification);
		}

		void resized() override {
			juce::Grid g;
			using Track = juce::Grid::TrackInfo;
			using Fr = juce::Grid::Fr;
			g.templateRows = { Track(Fr(8)), Track(Fr(8)), Track(Fr(2)) };
			g.templateColumns = { Track(Fr(1)) };
			g.items = { juce::GridItem(encButton),
				juce::GridItem(decButton), juce::GridItem(genType) };
			g.setGap(juce::Grid::Px(5));
			g.performLayout(getLocalBounds().withTrimmedTop(8).reduced(10));
		}

		juce::TextButton encButton, decButton;
		juce::ToggleButton genType;
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

	std::unique_ptr<RC4> rc4_ptr;
	std::unique_ptr<LFSR_JK> lfsr_jk_ptr;

	LeftGroupComponent leftGroupComponent;
	MiddleGroupComponent middleGroupComponent;
	RightGroupComponent rightGroupComponent;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StreamComponent)
};
