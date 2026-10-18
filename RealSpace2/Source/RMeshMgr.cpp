#include "stdafx.h"
#include "MXml.h"

#include <string>

#include "RealSpace2.h"

#include "RMeshMgr.h"

#include "MZFileSystem.h"

#include "MDebug.h"

_USING_NAMESPACE_REALSPACE2

_NAMESPACE_REALSPACE2_BEGIN

RMeshMgr::RMeshMgr()
{
	m_id_last = 0;
	m_cur	  = 0;

	m_mtrl_auto_load = true;
	m_is_map_object = false;

	//m_node_table.resize(MAX_NODE_TABLE); // FIX
	m_node_table.reserve(MAX_NODE_TABLE);//±âº»

	//for(int i=0;i<MAX_NODE_TABLE;i++)
	//	m_node_table[i] = NULL;

}

RMeshMgr::~RMeshMgr()
{
	DelAll();
}
#ifdef _DYNAMIC
std::string RMeshMgr::NormalizePath(const char* szPath) {
	if (!szPath) return "";
	std::string path = szPath;
	std::replace(path.begin(), path.end(), '\\', '/'); // Đổi \ thành /
	std::transform(path.begin(), path.end(), path.begin(), ::tolower); // Chuyển hết về chữ thường
	return path;
}

int RMeshMgr::Add(char* name, char* modelname, bool namesort, bool autoLoad)
{
	std::string safePath = NormalizePath(name);

	// BƯỚC 1: KIỂM TRA - Nếu Map đã có, trả về ngay lập tức
	auto it = m_MeshMap.find(safePath);
	if (it != m_MeshMap.end()) {
		return it->second->m_id; // Trả về ID cũ, thoát hàm luôn, không nạp thêm!
	}

	// BƯỚC 2: NẾU CHƯA CÓ THÌ MỚI NẠP
	RMesh* node = new RMesh;
	if (node == NULL) return -1;

	node->SetMtrlAutoLoad(m_mtrl_auto_load);
	node->SetMapObject(m_is_map_object);
#ifdef _DYNAMIC
	node->SetModelAutoLoad(autoLoad);
#endif

	if (!node->ReadElu((char*)safePath.c_str())) {
		mlog("Error: Loading %s failed!\n", safePath.c_str());
		delete node; // Giải phóng bộ nhớ ngay
		return -1;
	}

	if (modelname) node->SetName(modelname);
	node->CalcBox();

	if (m_MeshMap.size() > MAX_NODE_TABLE) {
		mlog("Warning: We currently have over %d different models in stock, so be careful not to run out of RAM!\n", MAX_NODE_TABLE);
	}

	// BƯỚC 3: ĐƯA VÀO KHO LƯU TRỮ
	node->m_id = m_id_last++;
	m_list.push_back(node);
	m_node_table.push_back(node);
	m_MeshMap[safePath] = node; // Lưu vào Map để lần sau Find thấy ngay

	return node->m_id;
}
#endif

//#define	LOAD_TEST

#ifdef	LOAD_TEST

#define __BP(i,n)	MBeginProfile(i,n);
#define __EP(i)		MEndProfile(i);

#else

#define __BP(i,n) ;
#define __EP(i) ;

#endif

int	RMeshMgr::LoadXmlList(const char* name,RFPROGRESSCALLBACK pfnProgressCallback, void *CallbackParam)
{
	__BP(2007,"RMeshMgr::LoadXmlList");

	MXmlDocument	XmlDoc;
	MXmlElement		PNode,Node,SNode;

	XmlDoc.Create();

//	if( !XmlDoc.LoadFromFile(name) ) 
//		return -1;

//	<-----------------

	char *buffer;
	MZFile mzf;

	if(g_pFileSystem) {
		if(!mzf.Open(name,g_pFileSystem)) {
			if(!mzf.Open(name))
				return -1;
		}
	} else {
		if(!mzf.Open(name))
			return -1;
	}

	buffer = new char[mzf.GetLength()+1];
	buffer[mzf.GetLength()] = 0;

	mzf.Read(buffer,mzf.GetLength());

	if(!XmlDoc.LoadFromMemory(buffer))
		return false;

	delete[] buffer;

	mzf.Close();

//	<------------------

	char Path[256];
	Path[0] = NULL;

	GetPath(name,Path);

	PNode = XmlDoc.GetDocumentElement();

	int nCnt = PNode.GetChildNodeCount();
	
	char NodeName[256];
	char IDName[256];
	char FileName[256];
	char WeaponMotionType[256];
//	char WeaponType[256];
	char LitModel[256];
	char NameSort[256];
	char AutoLoad[256];

	RMesh* pMesh = NULL;

	__BP(2008,"RMeshMgr::loop");

	for (int i=0; i<nCnt; i++) {
		
		// ÄÝ¹éÀÌ ÀÖÀ¸¸é ÁøÇà»óÈ²¿¡ ´ëÇÑ Á¤º¸¸¦ ¾Ë·ÁÁØ´Ù
		if(pfnProgressCallback)
			pfnProgressCallback(CallbackParam,float(i)/float(nCnt));

		Node = PNode.GetChildNode(i);
		Node.GetTagName(NodeName);

		if (NodeName[0] == '#') continue;

		if (strcmp(NodeName, "AddXml")==0) {
			Node.GetAttribute(IDName, "name");
			Node.GetAttribute(FileName, "filename");
			Node.GetAttribute(AutoLoad, "autoload");

			bool bAutoLoad = true;

			if(AutoLoad[0])	{
				if(strncmp(AutoLoad,"false",5)==0)
					bAutoLoad = false;
			}

			if(AddXml(FileName,IDName,bAutoLoad)==-1) {
				mlog("%s not found\n",IDName);
				XmlDoc.Destroy();
				return -1;
			}
		}
		else if(strcmp(NodeName, "AddElu")==0) {

			Node.GetAttribute(IDName, "name");
			Node.GetAttribute(FileName, "filename");

			if(Add(FileName,IDName)==-1) {
				mlog("%s not found\n",IDName);
				XmlDoc.Destroy();
				return -1;
			}
		}
		else if(strcmp(NodeName, "AddWeaponElu")==0) {

			Node.GetAttribute(IDName, "name");
			Node.GetAttribute(WeaponMotionType, "weapon_motion_type");

			int id = AddXml(&Node,NULL,IDName,false);

			if(id==-1) {
				mlog("%s not found\n",IDName);
				XmlDoc.Destroy();
				return -1;
			}
			else {

				int motion_id = atoi(WeaponMotionType);

				pMesh = GetFast(id);

				if(pMesh) {
					pMesh->SetMeshWeaponMotionType((RWeaponMotionType)motion_id);
				}
			}
		}		

		else if( stricmp( NodeName, "MeshInfo" ) == 0 )
		{
			int nChild = Node.GetChildNodeCount();
			MXmlElement ChildNode;
			for( int iwi = 0; iwi < nChild; ++iwi )
			{
				ChildNode = Node.GetChildNode( iwi );
				ChildNode.GetTagName( NodeName );

				if( NodeName[0] == '#' ) continue;
				
				if( stricmp( NodeName, "AddWorldItemElu" ) == 0 )
				{
					ChildNode.GetAttribute( IDName, "name" );
					//ChildNode.GetAttribute( FileName, "filename" );
					//int id = Add( FileName, IDName );
					int id = AddXml(&ChildNode, "", IDName, false );
					if( id == -1 )
					{
						mlog("%s not found\n",IDName);
						XmlDoc.Destroy();
						return -1;
					}
					else
					{
						RMesh* pMesh = GetFast(id);
						pMesh->m_LitVertexModel = true;
					}
				}
			}
		}
		else if(strcmp(NodeName, "AddEffectElu")==0) {

			Node.GetAttribute(IDName, "name");
//			Node.GetAttribute(FileName, "filename");
			Node.GetAttribute(LitModel, "lit_model");
			Node.GetAttribute(NameSort, "name_sort");

			// alpha type µîÀ» ÁöÁ¤ÇÒ ¼ö ÀÖ´Ù..
			int name_sort = 0;
			int litmodel = 1;

			if(NameSort[0]) {
				name_sort = atoi(NameSort);
			}

			int id = AddXml(&Node,Path,IDName,name_sort ? true:false);

			if(id==-1) {
				mlog("%s not found\n",IDName);
				XmlDoc.Destroy();
				return -1;
			}
			else {

				if( LitModel[0] ) {
					litmodel = atoi(LitModel);
				}

				pMesh = GetFast(id);

				if(pMesh) {
					pMesh->m_LitVertexModel = litmodel? true : false;
				}
			}
		}		
	}

	__EP(2008);

	XmlDoc.Destroy();

	__EP(2007);

	return 1;
}

int RMeshMgr::AddXml(MXmlElement* pNode, char* Path, char* modelname, bool namesort)
{
	// 1. TẠO KHÓA KÉP (KẾT HỢP PATH + MODELNAME)
	std::string key = "";
	if (Path) key += Path;
	if (modelname) key += modelname;

	// Chuẩn hóa Key: Viết thường và đổi dấu xuyệt để không bị lệch
	std::transform(key.begin(), key.end(), key.begin(), ::tolower);
	std::replace(key.begin(), key.end(), '\\', '/');

	// 2. KIỂM TRA TRONG MAP (DÙNG KHÓA KÉP)
	if (!key.empty()) {
		auto it = m_MeshMap.find(key);
		if (it != m_MeshMap.end()) {
			return it->second->m_id; // Khớp cả đường dẫn lẫn tên thì mới trả về!
		}
	}

	// 3. NẾU THỰC SỰ CHƯA CÓ THÌ MỚI NEW
	RMesh* node = new RMesh;
	if (node == NULL) return -1;

	if (namesort)
		node->m_bEffectSort = namesort;

	if (!node->ReadXmlElement(pNode, Path)) {
		delete node; // CỨU LẤY RAM: Phải xóa node nếu nạp lỗi!
		return -1;
	}

	if (modelname) {
		node->SetName(modelname);
	}

	node->CalcBox();

	if (m_id_last > MAX_NODE_TABLE) {
		// Chỉ mlog khi id chia hết cho 100, hoặc chỉ mlog đúng 1 lần
		if (m_id_last % 100 == 0) {
			mlog("Notice: Model count is high (%d)...\n", m_id_last);
		}
	}
	// 4. LƯU VÀO HỆ THỐNG
	node->m_id = m_id_last++;
	m_node_table.push_back(node);
	m_list.push_back(node);

	// Lưu vào Map để lần sau gọi lại model này là có ngay
	if (!key.empty()) {
		m_MeshMap[key] = node;
	}

	return node->m_id;
}

int RMeshMgr::AddXml(char* name,char* modelname,bool AutoLoad,bool namesort)
{
	RMesh* node;
	node = new RMesh;

	if(namesort)//ÀÌÆåÆ®ÀÇ ³»ºÎ³ëµå¸¦ ÀÌ¸§¼øÀ¸·Î Á¤·Ä..·ÎµùÀü¿¡ ÇØ¾ßÇÔ.
		node->m_bEffectSort = namesort;

	if(AutoLoad) {
		if (!node->ReadXml(name)) {
			mlog("xml %s file loading failure !!!\n",name);
			return -1;
		}
	}
	else {
		if(name)
			node->SetFileName(name);
	}

	if(modelname) {
		node->SetName(modelname);
	}

	node->CalcBox();

	m_node_table.push_back(node);

	node->m_id = m_id_last;

	if(m_id_last > MAX_NODE_TABLE)
		mlog("MeshNode It would be good to increase the reservation size 3...\n");

	m_list.push_back(node);
	m_id_last++;

	return m_id_last-1;
}

int	RMeshMgr::LoadList(char* fname)
{
	return 1;
}

int	RMeshMgr::SaveList(char* name)
{
	return 1;
}

///////////////////////////////////
// »èÁ¦

void RMeshMgr::Del(RMesh* pMesh)
{
	if(m_list.empty()) return;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();) {

		if((*node) == pMesh) {
			(*node)->ClearMtrl();					//material clear
			delete (*node);
			node = m_list.erase(node);
		}
		else ++node;
	}
}

void RMeshMgr::Del(int id)
{
	if(m_list.empty()) return;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();) {

		if((*node)->m_id == id) {
			(*node)->ClearMtrl();					//material clear
			delete (*node);
			node = m_list.erase(node);
		}
		else ++node;
	}
}

///////////////////////////////////
// ¸ðµÎÁ¦°Å

void RMeshMgr::DelAll()
{
	if(m_list.empty()) return;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end(); ) {

		(*node)->ClearMtrl();						//material clear
		delete (*node);
		(*node) = NULL;
		node = m_list.erase(node);// ++node
	}

	m_node_table.clear();//¹öÆÛ´Â ³²¾Æ ÀÖ´Ù..

	m_id_last = 0;
}

void RMeshMgr::ConnectMtrl()
{
	if(m_list.empty()) return;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();  ++node) {
		(*node)->ConnectMtrl();
	}
}

//////////////////////////////
// all

void RMeshMgr::Render()
{
	if(m_list.empty()) return;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();  ++node) {
		(*node)->Render(0,NULL);
	}
}

void RMeshMgr::ReloadAllAnimation()
{
	if(m_list.empty()) return;

	r_mesh_node node;

	RMesh* pMesh = NULL;
	int cnt = 0;

	for(node = m_list.begin(); node != m_list.end();  ++node) {
		pMesh = (*node);
		pMesh->ReloadAnimation();
		cnt++;
	}
}

void RMeshMgr::Render(int id)
{
	if(m_list.empty()) return;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();) {

		if((*node)->m_id == id) {
			(*node)->Render(0,NULL);
			return;
		}
		else ++node;
	}
}

void RMeshMgr::RenderFast(int id,D3DXMATRIX* unit_mat)
{
	if(m_list.empty()) return;

	if(id == -1) return;
	_ASSERT(m_node_table[id]);
	m_node_table[id]->Render(unit_mat);
}

RMesh* RMeshMgr::GetFast(int id)
{
	if(m_list.empty()) return NULL;

//	_ASSERT(m_node_table[id]);
//	if(id == -1) return NULL;
	if(id < 0)			return NULL;
	if(id > m_id_last)	return NULL;

	return m_node_table[id];
}

RMesh* RMeshMgr::Get(const char* name)
{
	if(m_list.empty()) return NULL;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end(); ++node) {
		if( (*node)->CmpName(name) )
			return (*node);
	}
	return NULL;
}

// ¿ì¼±Àº xml ¸¸ Áö¿øÇÑ´Ù.

RMesh*	RMeshMgr::Load(const char* name)	// ¸ðµ¨ set À» ¸Þ¸ð¸®·Î ¿Ã¸°´Ù.
{
	RMesh* pMesh = Get(name);

	if(pMesh) {

		if(pMesh->m_isMeshLoaded==false) {
		
			if (!pMesh->ReadXml( pMesh->GetFileName() )) {
				mlog("xml %s file loading failure !!!\n",name);
				return NULL;
			}

		}

		pMesh->m_bUnUsededCheck = true;
	}

	return pMesh;
}

void RMeshMgr::UnLoad(const char* name)	// ¸ðµ¨ set À» ¸Þ¸ð¸®¿¡¼­ ³»¸°´Ù.
{
	RMesh* pMesh = Get(name);

	if(pMesh) {

		string filename = pMesh->GetFileName();
		string modelname = pMesh->GetName();

		Del(pMesh);

		AddXml((char*)filename.c_str(),(char*)modelname.c_str(),false,false);
	}
}

void RMeshMgr::LoadAll()
{
	vector<string> t_vec;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();  ++node) {
		t_vec.push_back( (*node)->GetName() );
	}

	int cnt = (int)t_vec.size();

	for(int i=0;i<cnt;i++) {
		Load( (char*)t_vec[i].c_str() );
	}

}

void RMeshMgr::UnLoadAll()
{
	vector<string> t_vec;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();  ++node) {
		t_vec.push_back( (*node)->GetName() );
	}

	int cnt = (int)t_vec.size();

	for(int i=0;i<cnt;i++) {
		UnLoad( (char*)t_vec[i].c_str() );
	}

}

// »ç¿ë¾ÈÇÏ´Â°É·Î ¸ðµÎ Ã¼Å©

void RMeshMgr::CheckUnUsed()
{
	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();  ++node) {
		(*node)->m_bUnUsededCheck = false;
	}

}

void RMeshMgr::UnLoadChecked()
{
	vector<string> t_vec;

	r_mesh_node node;

	for(node = m_list.begin(); node != m_list.end();  ++node) {

		if((*node)->m_bUnUsededCheck == false )
			t_vec.push_back( (*node)->GetName() );
	}

	int cnt = (int)t_vec.size();

	for(int i=0;i<cnt;i++) {
		UnLoad( (char*)t_vec[i].c_str() );
	}
}


#ifdef _DYNAMIC
void RMeshMgr::GetPartsNode(RMeshPartsType parts, vector<RMeshNode*>& nodetable)
{
	r_mesh_node node;
	RMesh* pMesh = NULL;
	RMeshNode* pMeshNode = NULL;

	for (node = m_list.begin(); node != m_list.end(); ++node)
	{
		pMesh = (*node);
		pMesh->GetMeshData(parts, nodetable);
	}
}
RMeshNode* RMeshMgr::GetPartsNode(char* name, const char* eluName)
{
	r_mesh_node node;
	RMesh* pMesh = NULL;
	RMeshNode* pMeshNode = NULL;

	for (node = m_list.begin(); node != m_list.end(); ++node)
	{
		pMesh = (*node);
		pMeshNode = pMesh->GetMeshData(name, eluName);
		if (pMeshNode)
			return pMeshNode;
	}
	return NULL;
}
bool RMeshMgr::Find(const char* name)
{
	if (!name || name[0] == '\0') return false;

	for (r_mesh_node node = m_list.begin(); node != m_list.end(); ++node)
	{
		// 1. KIỂM TRA NULL AN TOÀN (Sửa lỗi crash Access Violation)
		if (*node == nullptr) continue;

		const string& fileName = (*node)->m_FileName;
		if (fileName.empty()) continue;

		// 2. LẤY TÊN FILE AN TOÀN (Xử lý cả dấu slash / và backslash \)
		size_t pos = fileName.find_last_of("/\\");
		string modelName = (pos != string::npos) ? fileName.substr(pos + 1) : fileName;

		// 3. SO SÁNH TÊN (Khuyên dùng _stricmp để không phân biệt chữ hoa/thường)
		if (_stricmp(modelName.c_str(), name) == 0)
		{
			return true;
		}
	}

	return false;
}
#endif
_NAMESPACE_REALSPACE2_END
