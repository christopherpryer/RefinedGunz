#ifndef MLABEL_H
#define MLABEL_H

#include "MWidget.h"
#include "MDrawContext.h"
#include "MLookNFeel.h"

class MLabel;

/// Label�� Draw �ڵ尡 �ִ� Ŭ����, �� Ŭ������ ��ӹ޾Ƽ� Ŀ���ҷ��� ���� �� �ִ�.
class MLabelLook{
public:
	virtual void OnDraw(MLabel* pLabel, MDrawContext* pDC);
	virtual MRECT GetClientRect(MLabel* pLabel, MRECT& r);
};

/// Label
class MLabel : public MWidget{
protected:
	MCOLOR			m_TextColor;
	MAlignmentMode	m_AlignmentMode;

	DECLARE_LOOK(MLabelLook)
	DECLARE_LOOK_CLIENT()

protected:

public:
	MLabel(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);

	/// �ؽ�Ʈ �÷� ����
	void SetTextColor(MCOLOR color);
	/// �ؽ�Ʈ �÷� ���
	MCOLOR GetTextColor(void);

	/// ���� ���
	MAlignmentMode GetAlignment(void);
	/// ���� ����
	MAlignmentMode SetAlignment(MAlignmentMode am);


#define MINT_LABEL	"Label"
	virtual const char* GetClassName(void){ return MINT_LABEL; }
};


#endif