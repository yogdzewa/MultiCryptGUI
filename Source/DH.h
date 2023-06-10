/*
  ==============================================================================

	DH.h
	Created: 9 Jun 2023 11:12:55pm
	Author:  L

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "DiffieHellman.h"
#include <WinSock2.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

//==============================================================================
void server(bool, void*);
void clientSockSetup(void*);

class DH : public juce::Component
{
public:
	DH()
	{
		addAndMakeVisible(leftComp);
		addAndMakeVisible(rightComp);
	}

	~DH() override {
		//closesocket(rightComp.clientSock);
		//std::terminate();
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
		grid.templateColumns = { Track(Fr(3)),Track(Fr(3)) };
		grid.items = { juce::GridItem(leftComp), juce::GridItem(rightComp) };

		grid.performLayout(getLocalBounds().reduced(6));
	}

	struct LeftGroupComponent : public juce::GroupComponent {
		LeftGroupComponent(DH* p) : p(p) {
			setText("Server");
			uint32_t g = std::stoul(keyLabel.getText().toStdString()),
				n = std::stoul(keyLabel2.getText().toStdString());
			server_dh.initAgreement(g, n);
			addAndMakeVisible(keyLabel);
			addAndMakeVisible(keyLabel2);
			addAndMakeVisible(keyLabel3);
			addAndMakeVisible(listenButton);

			keyLabel.setEditable(true);
			keyLabel2.setEditable(true);
			keyLabel.setColour(juce::Label::outlineColourId, juce::Colours::white);
			keyLabel3.setColour(juce::Label::outlineColourId, juce::Colours::white);
			keyLabel2.setColour(juce::Label::outlineColourId, juce::Colours::white);

			t = std::thread(server, thread_flag, this);

			listenButton.setButtonText("Listen");

			listenButton.onClick = [&] {
				uint32_t g = std::stoul(keyLabel.getText().toStdString()),
					n = std::stoul(keyLabel2.getText().toStdString());
				server_dh.initAgreement(g, n);
			};
		}

		void resized() override {
			auto b = getLocalBounds().withTrimmedTop(10).reduced(10);
			keyLabel2.setBounds(b.removeFromBottom(30).reduced(5));
			keyLabel.setBounds(b.removeFromBottom(30).reduced(5));
			listenButton.setBounds(b.removeFromBottom(60).reduced(5));
			keyLabel3.setBounds(b.reduced(5));
		}

		DH* p;
		std::thread t;
		bool thread_flag = true;
		char buffer[128] = { 0 };
		DiffieHellman server_dh;
		juce::Label keyLabel{ {}, "13" };
		juce::Label keyLabel2{ {}, "53" };
		juce::Label keyLabel3{ {}, "" };
		juce::TextButton listenButton;
	};

	struct RightGroupComponent : public juce::GroupComponent
	{
		RightGroupComponent(DH* _p) {
			p = _p;
			setText("Client");
			addAndMakeVisible(textEditor);
			addAndMakeVisible(sendButton);
			keyLabel1.setEditable(true);
			keyLabel2.setEditable(true);

			sendButton.setButtonText("Send Key");
			keyLabel1.setColour(juce::Label::outlineColourId, juce::Colours::white);
			keyLabel2.setColour(juce::Label::outlineColourId, juce::Colours::white);
			textEditor.setColour(juce::Label::outlineColourId, juce::Colours::white);

			clientSockSetup(this);

			sendButton.onClick = [&] {
				uint32_t g = std::stoul(p->leftComp.keyLabel.getText().toStdString()),
					n = std::stoul(p->leftComp.keyLabel2.getText().toStdString());
				client_dh.initAgreement(g, n);
				auto pubkey = client_dh.generatePubkey();
				juce::String s(pubkey);
				send(clientSock, s.getCharPointer(), 20, 0);
				recv(clientSock, buffer, 20, 0);
				pubkey = std::stoul(buffer);
				auto sharedkey = client_dh.generateSharedKey(pubkey);
				textEditor.setText(juce::String(sharedkey), juce::NotificationType::dontSendNotification);
			};

		}

		void resized() {
			auto b = getLocalBounds().withTrimmedTop(10).reduced(10);
			keyLabel2.setBounds(b.removeFromBottom(30).reduced(5));
			keyLabel1.setBounds(b.removeFromBottom(30).reduced(5));
			sendButton.setBounds(b.removeFromBottom(60).reduced(5));
			textEditor.setBounds(b);
		}

		DH* p;
		char buffer[128];
		SOCKET clientSock;
		DiffieHellman client_dh;
		juce::Label keyLabel1{ {}, "" };
		juce::Label keyLabel2{ {}, "" };
		juce::Label textEditor;
		juce::TextButton sendButton;
	};

	LeftGroupComponent leftComp{ this };
	RightGroupComponent rightComp{ this };

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DH)
};
