#pragma once

#include "common.h"
#include "RSA16.h"
#include "MD5.h"

class TrustedAuthority
{
	//This key pair is pre-generated with RSA16.
	//In real world, the public key is known to EVERYONE.
	static RSA16 authorityKeys;
public:
	typedef std::vector<uint16_t> certificate;

	struct Certificate
	{
		certificate certData;
	public:
		Certificate(const certificate& cert)
		{certData = cert;}

		operator certificate() const { return certData; }

		bytes getNameHash() const {
			RSA16 rsa(TrustedAuthority::authorityPubkey());
			bytes b = rsa.decryptSignedBytes(certData);
			return bytes(b.begin(), b.begin() + 16);
		}

		RSA16::PubKey getPubkey() const {
			RSA16 rsa(TrustedAuthority::authorityPubkey());
			bytes b = rsa.decryptSignedBytes(certData);
			return castBytesToVariable<RSA16::PubKey>(bytes(b.begin() + 16, b.end()));
		}
	};

	static RSA16::PubKey authorityPubkey()
	{return authorityKeys.getPubKey();}

	static Certificate generateCertificate(const std::string& name, const RSA16::PubKey& pubkey)
	{
		bytes certContent = concatBytes(MD5::digest(stringToBytes(name)), castVariableToBytes(pubkey));
		return Certificate(authorityKeys.signBytes(certContent));
	}
};

class ServerThread
{
public:
	virtual ~ServerThread(){};

	static RSA16 serverKeys;

	//In real world, the certificate is generated only once and kept by server.
	//Logic here is equivilant, does not affect C/S identification.
	TrustedAuthority::certificate cert;
	ServerThread() { cert = TrustedAuthority::generateCertificate("Server", serverKeys.getPubKey()); }

	//Thread entry is required to be static in most cases. This helper function redirects the call to actual instance.
	static void __Run(void* p_this)
	{((ServerThread*)p_this)->Run();}

	//This is the real "entrance"
	virtual void Run() {} //= 0;
	/*
	{
		STEP1:
			send a session token to client.

		STEP2:
			receive client's name and pubkey.
			receive client's certificate.
			verify client's identity with certificate.

		STEP3:
			send server's name and pubkey to client.
			send server's certificate.

		STEP4:
			Do a Diffie-Hellman key exchange with client, all messages encrypted with client's public key.
		
		STEP5:
			connection established, send something, sign with private key.
		-----------------------------------------------------
	}
	*/
};

class ClientThread
{
public:
	std::string serverIdentity = "Server";
	std::string clientName;
	RSA16 clientRSA;
	TrustedAuthority::certificate cert;

	virtual ~ClientThread() {};

	ClientThread(const std::string& name) { 
		cert = TrustedAuthority::generateCertificate(name, clientRSA.getPubKey()); 
	}

	//Thread entry is required to be static in most cases. This helper function redirects the call to actual instance.
	static void __Run(void* p_this)
	{((ClientThread*)p_this)->Run();}

	//This is the real "entrance"
	virtual void Run(){} //= 0;
	/*
	{
		STEP1:
			send a connection request to server, server should run a new ServerThread to serve this request.
			receive a session token from server.

		STEP2:
			send client's name and pubkey.
			send client's certificate.

		STEP3:
			receive server's name and pubkey.
			receive server's certificate.
			verify server's certificate with server's identity.

		STEP4:
			do a Diffie-Hellman key exchange with server, all messages encrypted with server's public key.

		STEP5:
			connection established, send something, sign with private key.
	}
	*/
};