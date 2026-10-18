#ifndef _RMeshMgr_h
#define _RMeshMgr_h

//#if _MSC_VER > 1000
//#pragma once
//#endif // _MSC_VER > 1000

#include "RMesh.h"
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>

_NAMESPACE_REALSPACE2_BEGIN

typedef list<RMesh*>			r_mesh_list;
typedef r_mesh_list::iterator	r_mesh_node;

#define MAX_NODE_TABLE 1000

class RMeshMgr
{
public:

	RMeshMgr();
	~RMeshMgr();

	int		AddXml(char* name,char* modelname=NULL,bool autoload=true,bool namesort=false);
	int     AddXml(MXmlElement* pNode,char* Path,char* modelname=NULL,bool namesort=false);

#ifdef _DYNAMIC
	int     Add(char* name, char* modelname = NULL, bool namesort = false, bool autoLoad = false);
#endif

	int		LoadXmlList(const char* name,RFPROGRESSCALLBACK pfnProgressCallback=NULL, void *CallbackParam=NULL);

	void	Del(int id);
	void	Del(RMesh* pMesh);

	int		LoadList(char* name);
	int		SaveList(char* name);

	void	DelAll();

	void	ConnectMtrl();
	void	Render();
	void	Render(int id);

	void	RenderFast(int id,D3DXMATRIX* unit_mat);

//	void	Frame();
//	void	Frame(int id);

	RMesh*	GetFast(int id);
	RMesh*	Get(const char* name);
	RMesh*  GetByPath(const char* szPath);

	RMesh*	Load(const char* name);	// ¸ðµ¨ set À» ¸Þ¸ð¸®·Î ¿Ã¸°´Ù.
	void	UnLoad(const char* name);	// ¸ðµ¨ set À» ¸Þ¸ð¸®¿¡¼­ ³»¸°´Ù.

	void	LoadAll();
	void	UnLoadAll();		

	void	CheckUnUsed();
	void	UnLoadChecked();

	void ReloadAllAnimation();

#ifdef _DYNAMIC
	void GetPartsNode(RMeshPartsType parts, vector<RMeshNode*>& nodetable);
	RMeshNode* GetPartsNode(char* name, const char* eluName);
#endif

	void SetMtrlAutoLoad(bool b) {
		m_mtrl_auto_load = b;
	}

	bool GetMtrlAutoLoad() {
		return m_mtrl_auto_load;
	}

	void SetMapObject(bool b) {
		m_is_map_object = b;
	}

	bool GetMapObject() {
		return m_is_map_object;
	}
private:
	// Tra cứu Model bằng tên file 
	std::unordered_map<std::string, RMesh*> m_MeshMap;

	// Hàm hỗ trợ chuẩn hóa chuỗi (viết thường, đổi dấu \ thành /)
	std::string NormalizePath(const char* szPath);

public:

	r_mesh_list m_list;
	int			m_id_last;

	bool		m_mtrl_auto_load;
	bool		m_is_map_object;

	vector<RMesh*> m_node_table;
	DWORD	m_cur;

#ifdef _DYNAMIC
	bool Find(const char* name);
#endif

};

_NAMESPACE_REALSPACE2_END

#endif//_RMeshMgr_h
