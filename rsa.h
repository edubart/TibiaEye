#ifndef RSA_H
#define RSA_H

class RSA : public Singleton<RSA>
{
public:
	RSA();
	virtual ~RSA();

	void setEncryptKey(const char* mod);
	void encrypt(char *msg, uint32 size);

	void setDecryptKey(const char* p, const char* q, const char* d);
	void decrypt(char* msg, uint32 size);

private:
	mpz_t m_p, m_q, m_u, m_d, m_dp, m_dq, m_e, m_mod, m_mod2;
};

#endif // RSA_H
