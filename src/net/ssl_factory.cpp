#include <zg/net/ssl_factory.hpp>
#include <openssl/x509v3.h>
using namespace zg::net;
SSL_CTX* ssl_factory::createClient(const std::string& host)
{
	SSL_CTX* ctx = SSL_CTX_new(TLS_client_method());
	if (!ctx)
	{
		throw std::runtime_error("failed to create SSL context");
	}
    X509_VERIFY_PARAM *param = SSL_CTX_get0_param(ctx);
    X509_VERIFY_PARAM_set_hostflags(param, X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);
    X509_VERIFY_PARAM_set1_host(param, host.c_str(), 0);
	return ctx;
}
SSL_CTX* ssl_factory::createServer()
{
	SSL_CTX* ssl_ctx = SSL_CTX_new(TLS_server_method());
	if (!ssl_ctx)
	{
		throw std::runtime_error("failed to create SSL context");
	}
	const int kBits = 2048;
	const int kCertDuration = 365;
	RSA* rsa = nullptr;
	BIGNUM* bne = nullptr;
	X509* x509 = nullptr;
	EVP_PKEY* pkey = nullptr;
	BIO* certBio = nullptr;
	BIO* keyBio = nullptr;
	int ret;
	bne = BN_new();
	ret = BN_set_word(bne, RSA_F4);
	if (ret != 1)
	{
		throw std::runtime_error("Error setting big number for RSA.");
	}
	rsa = RSA_new();
	ret = RSA_generate_key_ex(rsa, kBits, bne, nullptr);
	if (ret != 1)
	{
		throw std::runtime_error("Error generating RSA key.");
	}
	pkey = EVP_PKEY_new();
	if (!EVP_PKEY_assign_RSA(pkey, rsa))
	{
		throw std::runtime_error("Error assigning RSA key.");
	}
	{
		rsa = nullptr;
		x509 = X509_new();
		X509_set_version(x509, 2);
		ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
		X509_gmtime_adj(X509_get_notBefore(x509), 0);
		X509_gmtime_adj(X509_get_notAfter(x509), 60 * 60 * 24 * kCertDuration);
		X509_set_pubkey(x509, pkey);
		X509_NAME* name = X509_get_subject_name(x509);
		// X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (uInteger8*)"NZ", -1, -1, 0);
		// X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (uInteger8*)"Coje Link", -1, -1, 0);
		// X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (uInteger8*)(myHostname + ".local").c_str(), -1, -1, 0);
		X509_set_issuer_name(x509, name);
		if (!X509_sign(x509, pkey, EVP_sha256()))
		{
			throw std::runtime_error("Error signing X509 certificate.");
		}
		certBio = BIO_new(BIO_s_mem());
		keyBio = BIO_new(BIO_s_mem());
		if (!PEM_write_bio_PrivateKey(keyBio, pkey, nullptr, nullptr, 0, nullptr, nullptr))
		{
			throw std::runtime_error("Error writing private key to BIO.");
		}
		if (!PEM_write_bio_X509(certBio, x509))
		{
			throw std::runtime_error("Error writing certificate to BIO.");
		}
		{
			if (SSL_CTX_use_certificate(ssl_ctx, x509) != 1 ||
				SSL_CTX_use_PrivateKey(ssl_ctx, pkey) != 1)
			{
				throw std::runtime_error("Failed to assign certificate or private key to SSL context");
			}
		}
	}
cleanup:
	if (bne)
		BN_free(bne);
	if (rsa)
		RSA_free(rsa);
	if (pkey)
		EVP_PKEY_free(pkey);
	if (x509)
		X509_free(x509);
	if (certBio)
		BIO_free(certBio);
	if (keyBio)
		BIO_free(keyBio);
	return ssl_ctx;
}
SSL_CTX* ssl_factory::createServer(const interfaces::IFile& cert, const interfaces::IFile& key)
{
	std::string certString;
	certString.resize(cert.size());
	((interfaces::IFile&)cert).readBytes(0, certString.size(), certString.data());
	std::string keyString;
	keyString.resize(key.size());
	((interfaces::IFile&)key).readBytes(0, keyString.size(), keyString.data());
	return createServer(certString, keyString);
}
SSL_CTX* ssl_factory::createServer(const std::string& cert, const std::string& key)
{
	SSL_CTX* ctx = SSL_CTX_new(TLS_server_method());
	if (!ctx)
	{
		throw std::runtime_error("failed to create SSL context");
	}
	BIO* certBio = BIO_new_mem_buf(cert.data(), static_cast<int>(cert.size()));
	X509* x509 = PEM_read_bio_X509(certBio, nullptr, nullptr, nullptr);
	BIO_free(certBio);
	if (!x509)
	{
		SSL_CTX_free(ctx);
		throw std::runtime_error("Failed to load X509 certificate");
	}
	if (SSL_CTX_use_certificate(ctx, x509) != 1)
	{
		X509_free(x509);
		SSL_CTX_free(ctx);
		throw std::runtime_error("Failed to assign certificate to SSL context");
	}
	BIO* keyBio = BIO_new_mem_buf(key.data(), static_cast<int>(key.size()));
	EVP_PKEY* pkey = PEM_read_bio_PrivateKey(keyBio, nullptr, nullptr, nullptr);
	BIO_free(keyBio);
	if (!pkey)
	{
		X509_free(x509);
		SSL_CTX_free(ctx);
		throw std::runtime_error("Failed to load private key");
	}
	if (SSL_CTX_use_PrivateKey(ctx, pkey) != 1)
	{
		EVP_PKEY_free(pkey);
		X509_free(x509);
		SSL_CTX_free(ctx);
		throw std::runtime_error("Failed to assign private key to SSL context");
	}
	if (SSL_CTX_check_private_key(ctx) != 1)
	{
		EVP_PKEY_free(pkey);
		X509_free(x509);
		SSL_CTX_free(ctx);
		throw std::runtime_error("Private key does not match certificate");
	}
	EVP_PKEY_free(pkey);
	X509_free(x509);
	return ctx;
}
