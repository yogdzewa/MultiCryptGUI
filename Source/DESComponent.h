/*
  ==============================================================================

	DESComponent.h
	Created: 5 May 2023 11:15:57pm
	Author:  L

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "des.h"

//==============================================================================
/*
*/
class DESComponent : public juce::Component
{
public:
	DESComponent()
	{
		// In your constructor, you should add any child components, and
		// initialise any special settings that your component needs.
		desptr = new DES(key);

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
			bytes buf(plainText.begin(), plainText.end());

			auto keyText = leftGroupComponent.keyLabel.getText().toStdString();
			bytes keyBytes;
			juce::StringArray tokens;
			tokens.addTokens(keyText, " ", "");
			for (auto& token : tokens)
			{
				uint8_t value = std::stoi(token.toStdString(), nullptr, 16);
				keyBytes.push_back(value);
			}
			delete desptr;
			desptr = new DES(keyBytes);

			buf = desptr->encryptMessageWithLength(buf);
			//buf = desptr->decryptMessageWithLength(buf);
			if (isHexRight)
				rightText.setText(juce::String::toHexString(reinterpret_cast<const char*>(buf.data()), buf.size()));
			else
				rightText.setText(juce::String(reinterpret_cast<const char*>(buf.data()), buf.size()));
			isHexRight = !isHexRight;
		};

		decButton.onClick = [&] {
			auto cipherText = rightText.getText().toStdString();
			bytes buf;
			juce::StringArray tokens;
			tokens.addTokens(cipherText, " ", "");
			std::string str = tokens[0].toStdString();
			if (str.length() != 2 || !std::all_of(str.begin(), str.end(), ::isdigit))
				return;
			for (auto& token : tokens)
			{
				uint8_t value = std::stoi(token.toStdString(), nullptr, 16);
				buf.push_back(value);
			}
			buf = desptr->decryptMessageWithLength(buf);
			leftText.setText(juce::String((const char*)(buf.data()), buf.size()));
		};
	}

	~DESComponent() override {
		delete(desptr);
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

	struct LeftGroupComponent : public juce::GroupComponent, public juce::FilenameComponentListener
	{
		LeftGroupComponent() {
			setText("Plain Text");
			addAndMakeVisible(keyLabel);
			addAndMakeVisible(textEditor);
			addAndMakeVisible(fileChooser);

			keyLabel.setEditable(true);
			juce::Font font{ "Consolas", 18.0f, juce::Font::plain };
			textEditor.setFont(font);
			textEditor.setText("input plain text here");
			textEditor.setMultiLine(true);
			fileChooser.addListener(this);
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
			keyLabel.setBounds(b.removeFromBottom(200).reduced(5));
			keyLabel.setColour(juce::Label::outlineColourId, juce::Colours::white);
			textEditor.setBounds(b.reduced(5));
		}

		juce::Label keyLabel{ {}, "00 00 00 00 00 00 00 00" };
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

	DES* desptr;
	bytes key{ 0,0,0,0,0,0,0,0 };
	bool isHexRight = true;

	LeftGroupComponent leftGroupComponent;
	MiddleGroupComponent middleGroupComponent;
	RightGroupComponent rightGroupComponent;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DESComponent)
};
