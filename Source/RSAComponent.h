/*
  ==============================================================================

	RSAComponent.h
	Created: 5 May 2023 11:15:57pm
	Author:  L

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RSA16.h"
#include <iostream>
#include <string>

//==============================================================================
/*
*/
class RSAComponent : public juce::Component
{
public:
	RSAComponent()
	{
		// In your constructor, you should add any child components, and
		// initialise any special settings that your component needs.
		//generate pubkey and privkey automatically
		rsaptr = new RSA16();
		D = rsaptr->getAllInfo().D;
		N = rsaptr->getAllInfo().N;
		E = rsaptr->getAllInfo().E;
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
			//string -> uint8
			auto plainText = leftText.getText().toStdString();
			bytes buf(stringToBytes(plainText));

			N = static_cast<uint16_t>(std::stoi(leftGroupComponent.NLabel.getText().toStdString()));
			E = static_cast<uint16_t>(std::stoi(leftGroupComponent.ELabel.getText().toStdString()));
			D = static_cast<uint16_t>(std::stoi(leftGroupComponent.DLabel.getText().toStdString()));

			delete rsaptr;
			rsaptr = new RSA16(N,E,D);

			//uint8->uint16
			std::vector<uint16_t> res_uint16 = rsaptr->encryptBytes(buf);

			//uint16 -> uint8
			bytes res_uint8;
			for (const auto& elem : res_uint16) {
				uint8_t lowerByte = (elem & 0xff);
				uint8_t upperByte = ((elem >> 8) & 0xff);
				res_uint8.push_back(upperByte);
				res_uint8.push_back(lowerByte);
			}

			rightText.setText(bytesToHexdump(res_uint8));
		};

		decButton.onClick = [&] {
			auto cipherText = rightText.getText().toStdString();
			//hex -> uint8
			bytes buf_uint8(hexdumpToBytes(cipherText));
			std::vector<uint16_t> buf_uint16;
			//uint8 -> uint16
			uint8_t* ptr = buf_uint8.data();
			for (int i = 0; i < buf_uint8.size(); i += 2) {
				uint16_t val = (ptr[i] << 8) | ptr[i + 1];
				buf_uint16.push_back(val);
			}

			//dec
			buf_uint8 = rsaptr->decryptBytes(buf_uint16);

			//uint8 -> string
			leftText.setText(bytesToString(buf_uint8));
		};
	}

	~RSAComponent() override {
		delete(rsaptr);
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
			addAndMakeVisible(NLabel);
			addAndMakeVisible(ELabel);
			addAndMakeVisible(DLabel);
			addAndMakeVisible(textEditor);
			addAndMakeVisible(fileChooser);

			NLabel.setEditable(true);
			ELabel.setEditable(true);
			DLabel.setEditable(true);
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
			NLabel.setText(content[0], juce::NotificationType::dontSendNotification);
			ELabel.setText(content[1], juce::NotificationType::dontSendNotification);
			DLabel.setText(content[2], juce::NotificationType::dontSendNotification);
		}

		void resized() override {
			auto b = getLocalBounds().withTrimmedTop(10).reduced(10);
			fileChooser.setBounds(b.removeFromBottom(40).reduced(5));
			NLabel.setBounds(b.removeFromBottom(100).reduced(5));
			NLabel.setColour(juce::Label::outlineColourId, juce::Colours::white);
			ELabel.setBounds(b.removeFromBottom(100).reduced(5));
			ELabel.setColour(juce::Label::outlineColourId, juce::Colours::white);
			DLabel.setBounds(b.removeFromBottom(100).reduced(5));
			DLabel.setColour(juce::Label::outlineColourId, juce::Colours::white);
			textEditor.setBounds(b.reduced(5));
		}

		juce::Label NLabel{ {}, "3233"};
		juce::Label ELabel{ {}, "17"};
		juce::Label DLabel{ {}, "2753"};
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

	RSA16* rsaptr;
	uint16_t N, E, D;
	bool isHexRight = true;

	LeftGroupComponent leftGroupComponent;
	MiddleGroupComponent middleGroupComponent;
	RightGroupComponent rightGroupComponent;


	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RSAComponent)
};
