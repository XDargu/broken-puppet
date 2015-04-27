#ifndef INC_SSB_H_
#define INC_SSB_H_

class Cssb
{
public:
	Cssb();
	~Cssb();
	void bombClose();
	virtual void aidRequired()=0;
	virtual void partnerAttacked()=0;
	virtual void crewPatrol()=0;
};
#endif
