#ifndef _ZLADDERWARSRANKINGLISTOBOX_H
#define _ZLADDERWARSRANKINGLISTOBOX_H

#define NUM_DISPLAY_LADDERWARS_RANKING 5
#define INDEX_LADDERWARS_MY_RANKING 2			

struct ZPLAYERRANKINGITEM {
	char szCharName[MAX_CHARNAME+1];
	int nWins;
	int nLosses;;
	int nScore;
	int nDraws;
    int nRank;

	ZPLAYERRANKINGITEM() : nWins(0), nLosses(0), nScore(0), nRank(0) {
		szCharName[0] = 0;
	}
};

class ZLadderWarsRankingListBox : public MWidget {

	ZPLAYERRANKINGITEM m_rankingList[NUM_DISPLAY_LADDERWARS_RANKING];

	MBitmapR2* m_pBmpRankingItemBg;

	int m_nMyRankIndex;			

protected:
	virtual void	OnDraw( MDrawContext* pDC );

public:
	ZLadderWarsRankingListBox(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	~ZLadderWarsRankingListBox();

	void ClearAll();
	void SetRankInfo(unsigned int nIndex, const ZPLAYERRANKINGITEM& rankingItem);

	void LoadInterfaceImgs();
	void UnloadInterfaceImgs();

	void SetMyRankIndex(int myRankIndex) { m_nMyRankIndex = myRankIndex; }

};



#endif