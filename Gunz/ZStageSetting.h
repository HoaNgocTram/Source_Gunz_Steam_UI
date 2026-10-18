#ifndef _ZSTAGESETTING_H
#define _ZSTAGESETTING_H

#include "ZPrerequisites.h"

class ZStageSetting
{
public:
	static void ShowStageSettingDialog( const MSTAGE_SETTING_NODE* pStageSetting, bool bShowAll);
	static void InitStageSettingDialog();
	static void ApplyStageSettingDialog();
	static void AdjustLimitTimeStageSettingDialog();
	static void PostDataToServer();
	static void InitStageSettingGameType();
	static void InitStageSettingGameFromGameType();
};

// for new UI
int			ZStageOptGetStepperCount();
const char* ZStageOptGetStepperItemName(int i);
void		ZStageOptRefreshAllSteppers();
int			ZStageOptGetStepperIndex(const char* szItem);
void		ZStageOptSetStepperIndex(const char* szItem, int nIndex);
bool		ZStageOptStepperOnClick(MWidget* pWidget);
void		ZStageOptMirrorCombos();
void		ZStageOptApplyCombos();
void		ZStageOptRememberCreateCombos();
void		ZStageOptApplyPendingCombos();

#endif