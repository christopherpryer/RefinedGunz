#include "stdafx.h"

#ifdef _HSHIELD
#include "HShield/HShield.h"
#endif

#ifdef _XTRAP
#include "XTrap/XTrap.h"
#pragma comment ( lib, "XTrap/Multy C Type/XTrapIC.lib")
#endif

#include "ZPrerequisites.h"
#include "ZConfiguration.h"
#include "ZGameClient.h"
#include <windows.h>
#include <wingdi.h>
#include <mmsystem.h>
#include <shlwapi.h>
#include <shellapi.h>

#include "dxerr9.h"

#include "main.h"
#include "resource.h"
#include "VersionNo.h"

#include "Mint4R2.h"
#include "ZApplication.h"
#include "MDebug.h"
#include "ZMessages.h"
#include "MMatchNotify.h"
#include "RealSpace2.h"
#include "Mint.h"
#include "ZGameInterface.h"
#include "RFrameWork.h"
#include "ZButton.h"
#include "ZDirectInput.h"
#include "ZActionDef.h"
#include "MRegistry.h"
#include "ZInitialLoading.h"
#include "MDebug.h"
#include "MCrashDump.h"
#include "ZEffectFlashBang.h"
#include "ZMsgBox.h"
#include "ZSecurity.h"
#include "ZStencilLight.h"
#include "ZReplay.h"
#include "ZUtil.h"
#include "ZOptionInterface.h"

#ifdef USING_VERTEX_SHADER
#include "RShaderMgr.h"
#endif

//#include "mempool.h"
#include "RLenzFlare.h"
#include "ZLocale.h"
#include "MSysInfo.h"

#include "MTraceMemory.h"
#include "ZInput.h"
#include "Mint4Gunz.h"

#include "RGMain.h"

#ifdef _DEBUG
RMODEPARAMS	g_ModeParams={640,480,false,D3DFMT_R5G6B5};
//RMODEPARAMS	g_ModeParams={1024,768,false,RPIXELFORMAT_565};
#else
RMODEPARAMS	g_ModeParams={800,600,true,D3DFMT_R5G6B5};
#endif

#ifndef _DEBUG
#define SUPPORT_EXCEPTIONHANDLING
#endif


RRESULT RenderScene(void *pParam);

#define RD_STRING_LENGTH 512
char cstrReleaseDate[512];// = "ReleaseDate : 12/22/2003";

ZApplication	g_App;
MDrawContextR2* g_pDC = NULL;
MFontR2*		g_pDefFont = NULL;
ZDirectInput	g_DInput;
ZInput*			g_pInput = NULL;
Mint4Gunz		g_Mint;


HRESULT GetDirectXVersionViaDxDiag( DWORD* pdwDirectXVersionMajor, DWORD* pdwDirectXVersionMinor, TCHAR* pcDirectXVersionLetter );



void _ZChangeGameState(int nIndex)
{
	GunzState state = GunzState(nIndex);

	if (ZApplication::GetGameInterface())
	{
		ZApplication::GetGameInterface()->SetState(state);
	}
}

//list<HANDLE>	g_FontMemHandles;

RRESULT OnCreate(void *pParam)
{
	g_App.PreCheckArguments();

	g_RGMain->OnCreateDevice();

	RCreateLenzFlare("System/LenzFlare.xml");
	RGetLenzFlare()->Initialize();

	mlog("main : RGetLenzFlare()->Initialize() \n");

	RBspObject::CreateShadeMap("sfx/water_splash.bmp");
	//D3DCAPS9 caps;
	//RGetDevice()->GetDeviceCaps( &caps );
	//if( caps.VertexShaderVersion < D3DVS_VERSION(1, 1) )
	//{
	//	RGetShaderMgr()->mbUsingShader				= false;
	//	RGetShaderMgr()->shader_enabled				= false;
	//	mlog("main : VideoCard Dosen't support Vertex Shader...\n");
	//}
	//else
	//{
	//	mlog("main : VideoCard support Vertex Shader...\n");
	//}

//	sprintf_safe( cstrReleaseDate, "Release Date : %s", __DATE__ );
	sprintf_safe( cstrReleaseDate, "");				// 삭제.
	g_DInput.Create(g_hWnd, FALSE, FALSE);
	g_pInput = new ZInput(&g_DInput);
	/*
	for(int i=0; i<ZApplication::GetFileSystem()->GetFileCount(); i++){
		const char* szFileName = ZApplication::GetFileSystem()->GetFileName(i);
		size_t nStrLen = strlen(szFileName);
		if(nStrLen>3){
			if(_stricmp(szFileName+nStrLen-3, "ttf")==0){
				int nFileLenth = ZApplication::GetFileSystem()->GetFileLength(i);
				char* pFileData = new char[nFileLenth];
				ZApplication::GetFileSystem()->ReadFile(szFileName, pFileData, nFileLenth);
				int nInstalled = 0;
				HANDLE hFontMem = AddFontMemResourceEx(pFileData, 1, 0, &nInstalled);
				g_FontMemHandles.insert(g_FontMemHandles.end(), hFontMem);
				delete[] pFileData;
			}
		}
	}
	*/
	RSetGammaRamp(Z_VIDEO_GAMMA_VALUE);
	RSetRenderFlags(RRENDER_CLEAR_BACKBUFFER);

	ZGetInitialLoading()->Initialize(  1, 0, 0, RGetScreenWidth(), RGetScreenHeight(), 0, 0, 1024, 768 );

	mlog("main : ZGetInitialLoading()->Initialize() \n");

	struct _finddata_t c_file;
	intptr_t hFile;
	char szFileName[256];
#define FONT_DIR	"Font/"
#define FONT_EXT	"ttf"
	if( (hFile = _findfirst(FONT_DIR "*." FONT_EXT, &c_file )) != -1L ){
		do{
			strcpy_safe(szFileName, FONT_DIR);
			strcat(szFileName, c_file.name);
			AddFontResource(szFileName);
		}while( _findnext( hFile, &c_file ) == 0 );
		_findclose(hFile);
	}

	g_pDefFont = new MFontR2;

	if( !g_pDefFont->Create("Default", Z_LOCALE_DEFAULT_FONT, 9, 1.0f) )
//	if( !g_pDefFont->Create("Default", RGetDevice(), "FONTb11b", 9, 1.0f, true, false) )
//	if( !g_pDefFont->Create("Default", RGetDevice(), "FONTb11b", 14, 1.0f, true, false) )
	{
		mlog("Fail to Create defualt font : MFontR2 / main.cpp.. onCreate\n" );
		g_pDefFont->Destroy();
		SAFE_DELETE( g_pDefFont );
		g_pDefFont	= NULL;
	}
	//pDefFont->Create("Default", RGetDevice(), "FONTb11b", 10, 1.0f, true, false);
	//pDefFont->Create("Default", RGetDevice(), "FONTb11b", 16, 1.0f, true, false, -1, 4);
	//pDefFont->Create("Default", RGetDevice(), "-2002", 10, 1.0f, false, false, -1, 1);
	//pDefFont->Create("Default", RGetDevice(), "HY수평선L", 12, 1.0f, false, false, -1, 2);

	//MLoadDesignerMode();
	// 기본 800x600 디자인으로 생성하고, 나중에 Resize를 화면 크기로 해준다.

	g_pDC = new MDrawContextR2(RGetDevice());

#ifndef _FASTDEBUG
	if( ZGetInitialLoading()->IsUseEnable() )
	{
		if( ZGetLocale()->IsTeenMode() )
		{
			ZGetInitialLoading()->AddBitmap( 0, "Interface/Default/LOADING/loading_teen.jpg" );
		}
		else
		{
			ZGetInitialLoading()->AddBitmap( 0, "Interface/Default/LOADING/loading_adult.jpg" );
		}
		ZGetInitialLoading()->AddBitmapBar( "Interface/Default/LOADING/loading.bmp" );
		ZGetInitialLoading()->SetText( g_pDefFont, 10, 30, cstrReleaseDate );

		ZGetInitialLoading()->SetPercentage( 0.0f );
		ZGetInitialLoading()->Draw( MODE_FADEIN, 0 , true );
	}
#endif

//	ZGetInitialLoading()->SetPercentage( 10.0f );
//	ZGetInitialLoading()->Draw( MODE_DEFAULT, 0 , true );

	g_Mint.Initialize(800, 600, g_pDC, g_pDefFont);
	Mint::GetInstance()->SetHWND(RealSpace2::g_hWnd);

	mlog("main : g_Mint.Initialize() \n");

//	ZGetConfiguration()->LoadHotKey(FILENAME_CONFIG);

	ZLoadingProgress appLoading("application");
	if(!g_App.OnCreate(&appLoading))
	{
		ZGetInitialLoading()->Release();
		return R_ERROR_LOADING;
	}

//	ZGetInitialLoading()->SetPercentage( 50.0f );
//	ZGetInitialLoading()->Draw( MODE_DEFAULT, 0, true );
	
	mlog("main : g_App.OnCreate() \n");

	ZGetSoundEngine()->SetEffectVolume(Z_AUDIO_EFFECT_VOLUME);
	ZGetSoundEngine()->SetMusicVolume(Z_AUDIO_BGM_VOLUME);
	ZGetSoundEngine()->SetEffectMute(Z_AUDIO_EFFECT_MUTE);
	ZGetSoundEngine()->SetMusicMute(Z_AUDIO_BGM_MUTE);

	g_Mint.SetWorkspaceSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	g_Mint.GetMainFrame()->SetSize(g_ModeParams.nWidth, g_ModeParams.nHeight);
	ZGetOptionInterface()->Resize(g_ModeParams.nWidth, g_ModeParams.nHeight);

//	ZGetInitialLoading()->SetPercentage( 80.f );
//	ZGetInitialLoading()->Draw( MODE_DEFAULT, 0, true );
    
	// Default Key
	for(int i=0; i<ZACTION_COUNT; i++){
//		g_Mint.RegisterActionKey(i, ZGetConfiguration()->GetKeyboard()->ActionKeys[i].nScanCode);
		ZACTIONKEYDESCRIPTION& keyDesc = ZGetConfiguration()->GetKeyboard()->ActionKeys[i];
		g_pInput->RegisterActionKey(i, keyDesc.nVirtualKey);
		if(keyDesc.nVirtualKeyAlt!=-1)
			g_pInput->RegisterActionKey(i, keyDesc.nVirtualKeyAlt);
	}

	g_App.SetInitialState();

//	ParseParameter(g_szCmdLine);

	ZGetFlashBangEffect()->SetDrawCopyScreen(true);

	static const char *szDone = "Done.";
	ZGetInitialLoading()->SetLoadingStr(szDone);
	if( ZGetInitialLoading()->IsUseEnable() )
	{
#ifndef _FASTDEBUG
		ZGetInitialLoading()->SetPercentage( 100.f );
		ZGetInitialLoading()->Draw( MODE_FADEOUT, 0 ,true  );
#endif
		ZGetInitialLoading()->Release();
	}

	mlog("main : OnCreate() done\n");

	SetFocus(g_hWnd);

	return R_OK;
}

RRESULT OnDestroy(void *pParam)
{
	mlog("main : OnDestroy()\n");

	g_App.OnDestroy();

	SAFE_DELETE(g_pDefFont);

	g_Mint.Finalize();

	mlog("main : g_Mint.Finalize()\n");

	SAFE_DELETE(g_pInput);
	g_DInput.Destroy();

	mlog("main : g_DInput.Destroy()\n");

	RGetShaderMgr()->Release();

//	g_App.OnDestroy();

	mlog("main : g_App.OnDestroy()\n");

	ZGetConfiguration()->Destroy();

	mlog("main : ZGetConfiguration()->Destroy()\n");

	delete g_pDC;

	struct _finddata_t c_file;
	intptr_t hFile;
	char szFileName[256];
#define FONT_DIR	"Font/"
#define FONT_EXT	"ttf"
	if( (hFile = _findfirst(FONT_DIR" *." FONT_EXT, &c_file )) != -1L ){
		do{
			strcpy_safe(szFileName, FONT_DIR);
			strcat(szFileName, c_file.name);
			RemoveFontResource(szFileName);
		}while( _findnext( hFile, &c_file ) == 0 );
		_findclose(hFile);
	}

	MFontManager::Destroy();
	MBitmapManager::Destroy();
	MBitmapManager::DestroyAniBitmap();

	mlog("main : MBitmapManager::DestroyAniBitmap()\n");

	/*
	for(list<HANDLE>::iterator i=g_FontMemHandles.begin(); i!=g_FontMemHandles.end(); i++){
		RemoveFontMemResourceEx(*i);
	}
	*/

	//ReleaseMemPool(RealSoundEffectPlay);
	//UninitMemPool(RealSoundEffectPlay);

	//ReleaseMemPool(RealSoundEffect);
	//UninitMemPool(RealSoundEffect);

	//ReleaseMemPool(RealSoundEffectFx);
	//UninitMemPool(RealSoundEffectFx);

	//mlog("main : UninitMemPool(RealSoundEffectFx)\n");

	// 메모리풀 헤제
	ZBasicInfoItem::Release(); // 할당되어 있는 메모리 해제
//	ZHPInfoItem::Release();

	ZGetStencilLight()->Destroy();
	LightSource::Release();

//	ZStencilLight::GetInstance()->Destroy();

	RBspObject::DestroyShadeMap();
	RDestroyLenzFlare();
	RAnimationFileMgr::GetInstance()->Destroy();
	ZStringResManager::FreeInstance();

	mlog("main : OnDestroy() done\n");

	return R_OK;
}

RRESULT OnUpdate(void* pParam)
{
	__BP(100, "main::OnUpdate");

	g_pInput->Update();

	g_App.OnUpdate();

#ifndef _DEBUG
#ifdef _XTRAP
	// XTrap 주기적인 체크
#define XTRAP_INTERVAL 5000
	static DWORD xTrapLastTime = timeGetTime();
	DWORD currentTime=timeGetTime();
	if(xTrapLastTime+XTRAP_INTERVAL<currentTime)
	{
		xTrapLastTime=currentTime;

			char szMsgBuf[500] = {
#ifdef LOCALE_KOREA
				"비정상적인 행위가 감지되었습니다. 게임을 종료합니다.\n"
#else
				"An abnormal behavior is detected. Terminating game.\n"
#endif
			};

			///////////////////////////////////////////////////////////////////
			// 디폴트 탐지 메시지 루틴

			if (g_bApiMal			== TRUE ||
				g_bMemoryMdl		== TRUE ||
				g_bAutoMousMdl		== TRUE ||
				g_bAutoKeybMdl		== TRUE ||
				g_bMalMdl			== TRUE ||
				g_bSpeedMdl			== TRUE ||
				g_bFileMdl			== TRUE ||
				g_bApiHookMdl		== TRUE ||
				g_bDebugModMdl		== TRUE ||
				g_bMemoryCrack		== TRUE ||
				g_bFileCrack		== TRUE ||
				g_bApiHookCrack		== TRUE)
			{
				mlog("xtrap error : ");
				if (g_bApiMal			== TRUE) mlog("ApiMal");
				if (g_bMemoryMdl		== TRUE) mlog("MemoryMdl");
				if (g_bAutoMousMdl		== TRUE) mlog("AutoMousMdl");
				if (g_bAutoKeybMdl		== TRUE) mlog("AutoKeybMdl");
				if (g_bMalMdl			== TRUE) mlog("MalMdl");
				if (g_bSpeedMdl			== TRUE) mlog("SpeedMdl");
				if (g_bFileMdl			== TRUE) mlog("FileMdl");
				if (g_bApiHookMdl		== TRUE) mlog("ApiHookMdl");
				if (g_bDebugModMdl		== TRUE) mlog("DebugModMdl");
				if (g_bMemoryCrack		== TRUE) mlog("MemoryCrack");
				if (g_bFileCrack		== TRUE) mlog("FileCrack");
				if (g_bApiHookCrack		== TRUE) mlog("ApiHookCrack");
				mlog("\n");


				if (ZGetGameClient()) ZGetGameClient()->Disconnect();
//				AfxMessageBox(szMsgBuf); 
				mlog(szMsgBuf);
	            PostQuitMessage(0);
				//
				// CloseSocket and Exit Game Client
				//
			}

			/* Version 0xA5001069 */
			if (g_bOsMdl			== TRUE)
			{
				if (ZGetGameClient()) ZGetGameClient()->Disconnect();
#ifdef LOCALE_KOREA
//				AfxMessageBox("파일 속성에서 호환성 모드가 설정되었습니다. 설정을 해제하십시오."); 
				mlog("파일 속성에서 호환성 모드가 설정되었습니다. 설정을 해제하십시오.\n"); 
#else
//				AfxMessageBox("The compatibility mode is activated in the file property. Please deactivated it."); 
				mlog("The compatibility mode is activated in the file property. Please deactivated it.\n"); 
#endif 
				PostQuitMessage(0);
				//
				// CloseSocket and Exit Game Clinet
				//
			}

			if (g_bPatchMdl			== TRUE)
			{
				if (ZGetGameClient()) ZGetGameClient()->Disconnect();
#ifdef LOCALE_KOREA
//				AfxMessageBox("패치 서버 접속에 문제가 있습니다. 네트워크 상황을 확인하십시오."); 
				mlog("패치 서버 접속에 문제가 있습니다. 네트워크 상황을 확인하십시오.\n"); 
#else
//				AfxMessageBox("There is a trouble connecting to the patch server. Please check the network condition."); 
				mlog("There is a trouble connecting to the patch server. Please check the network condition.\n"); 
#endif 
				PostQuitMessage(0);
				//
				// CloseSocket and Exit Game Clinet
				//
			}

			if (g_bStartXTrap		== FALSE)
			{
				if (ZGetGameClient()) ZGetGameClient()->Disconnect();
#ifdef LOCALE_KOREA
//				AfxMessageBox("게임 보안모듈을 실행할수 없습니다."); 
				mlog("게임 보안모듈을 실행할수 없습니다.\n"); 
#else
//				AfxMessageBox("Can not run the game security module."); 
				mlog("Can not run the game security module.\n"); 
#endif
				PostQuitMessage(0);

				//
				// CloseSocket and Exit Game Clinet
				//
			}
	}
#endif // of xtrap
#endif
	__EP(100);

	return R_OK;
}

static void GetRotAniMat(RAnimationNode& node, const matrix* parent_base_inv, int frame, matrix& mat);
static void GetPosAniMat(RAnimationNode& node, const matrix* parent_base_inv, int frame, matrix& mat);

matrix GetHeadMatrix(const matrix& World, float y, MMatchSex Sex, ZC_STATE_LOWER v, int Frame)
{
	extern ZANIMATIONINFO g_AnimationInfoTableLower[ZC_STATE_LOWER_END];
	auto Ani = ZGetMeshMgr()->Get("herowoman1")->m_ani_mgr.GetAnimation(g_AnimationInfoTableLower[v].Name, ZGetGame()->m_pMyCharacter->m_pVMesh->GetSetectedWeaponMotionID());
	//DMLog("Motion type: %d\n", ZGetGame()->m_pMyCharacter->m_pVMesh->GetSetectedWeaponMotionID());

	/*if (!Ani)
	{
	MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "GetHeadPosition -- Can't find animation!");
	return v3(0, 0, 0);
	}

	auto Node = Ani->m_pAniData->GetNode("Bip01 Head");

	if (!Node)
	{
	MGetMatchServer()->LogF(MMatchServer::LOG_ALL, "GetHeadPosition -- Can't find head node!");
	return v3(0, 0, 0);
	}*/

	static const char* Hierarchy[] = { "Bip01", "Bip01 Pelvis", "Bip01 Spine", "Bip01 Spine1", "Bip01 Spine2", "Bip01 Neck", "Bip01 Head" };
	static const RMeshPartsPosInfoType HierarchyParts[] = { eq_parts_pos_info_Root, eq_parts_pos_info_Pelvis,
		eq_parts_pos_info_Spine, eq_parts_pos_info_Spine1, eq_parts_pos_info_Spine2, eq_parts_pos_info_Neck, eq_parts_pos_info_Head };

	matrix last_mat;
	matrix last_mat_inv;
	matrix mat;
	bool parent = false;
	for (size_t i = 0; i < ArraySize(Hierarchy); i++)
	{
		auto& cur = *Ani->m_pAniData->GetNode(Hierarchy[i]);

		rmatrix* last_mat_inv_ptr = parent ? &last_mat_inv : nullptr;
		D3DXMatrixIdentity(&mat);
		GetRotAniMat(cur, last_mat_inv_ptr, Frame, mat);
		GetPosAniMat(cur, last_mat_inv_ptr, Frame, mat);

		float ratio = 0;

		switch (HierarchyParts[i])
		{
		case eq_parts_pos_info_Head:
			ratio = 0.3;
			break;
		case eq_parts_pos_info_Spine1:
			ratio = 0.6;
			break;
		case eq_parts_pos_info_Spine2:
			ratio = 0.5;
			break;
		default:
			goto no_calc_lookat;
		};

		{
			float y_clamped = y;

#define MAX_YA_FRONT	50.f
#define MAX_YA_BACK		-70.f

			if (y_clamped > MAX_YA_FRONT)	y_clamped = MAX_YA_FRONT;
			if (y_clamped < MAX_YA_BACK)		y_clamped = MAX_YA_BACK;

			auto my = RGetRotY(y_clamped * ratio);

			/*DMLog("my for %f * %f:\n", y_clamped, ratio);
			DLogMatrix(my);

			DMLog("pre mat:\n");
			DLogMatrix(mat);*/

			mat *= my;

			/*DMLog("post mat:\n");
			DLogMatrix(mat);*/

			//DMLog("y_clamped: %f\n", y_clamped);
		}

	no_calc_lookat:

		//DMLog("Trans %d: %f, %f, %f\n", i, mat(3, 0), mat(3, 1), mat(3, 2));

		if (parent)
			mat *= last_mat;

		/*DMLog("%d:\n", i);
		DLogMatrix(mat);

		DMLog("%s:\n", Hierarchy[i]);
		DLogMatrix(cur.m_mat_base);*/

		parent = true;
		last_mat = mat;
		RMatInv(last_mat_inv, cur.m_mat_base);
	}

	return mat * World;
}

static void GetRotAniMat(RAnimationNode& node, const matrix* parent_base_inv, int frame, matrix& mat)
{
	D3DXMATRIX buffer, Inv;

	bool bAni = false;

	if (node.m_rot_cnt)
		bAni = true;

	if (bAni) {
		D3DXQUATERNION out = node.GetRotValue(frame);

		D3DXMatrixRotationQuaternion(&mat, &out);
	}
	else {

		D3DXMatrixIdentity(&buffer);

		if (parent_base_inv) {
			D3DXMatrixMultiply(&buffer, &node.m_mat_base, parent_base_inv);
		}
		else {
			memcpy(&buffer, &node.m_mat_base, sizeof(D3DXMATRIX));
		}

		buffer._41 = buffer._42 = buffer._43 = 0;
		mat *= buffer;
	}
}

static void GetPosAniMat(RAnimationNode& node, const matrix* parent_base_inv, int frame, matrix& mat)
{
	D3DXMATRIX buffer, Inv;

	bool bAni = false;

	if (node.m_pos_cnt)
	{
		bAni = true;
	}

	if (bAni) {
		auto pos = node.GetPosValue(frame);

		for (int i = 0; i < 3; i++)
			mat(3, i) = pos[i];
	}
	else {

		D3DXMatrixIdentity(&buffer);

		if (parent_base_inv) {
			D3DXMatrixMultiply(&buffer, &node.m_mat_base, parent_base_inv);
		}
		else {
			memcpy(&buffer, &node.m_mat_base, sizeof(D3DXMATRIX));
		}

		for (int i = 0; i < 3; i++)
			mat(3, i) = buffer(3, i);
	}
}

static void GetRotAniMat(RMeshNode& node, const matrix* parent_base_inv, int frame, matrix& mat);
static void GetPosAniMat(RMeshNode& node, const matrix* parent_base_inv, int frame, matrix& mat);

matrix GetHeadMatrix(const matrix& World, float y, MMatchSex Sex, ZC_STATE_LOWER v, int Frame, RMeshNode& base_node)
{
	static const char* Hierarchy[] = { "Bip01", "Bip01 Pelvis", "Bip01 Spine", "Bip01 Spine1", "Bip01 Spine2", "Bip01 Neck", "Bip01 Head" };
	static const RMeshPartsPosInfoType HierarchyParts[] = { eq_parts_pos_info_Root, eq_parts_pos_info_Pelvis,
		eq_parts_pos_info_Spine, eq_parts_pos_info_Spine1, eq_parts_pos_info_Spine2, eq_parts_pos_info_Neck, eq_parts_pos_info_Head };

	RMeshNode* nodes[7];
	auto& p = base_node;
	nodes[6] = &base_node;
	for (int i = 5; i >= 0; i--)
	{
		nodes[i] = nodes[i + 1]->m_pParent;
	}

	matrix last_mat;
	matrix last_mat_inv;
	matrix mat;
	bool parent = false;
	for (size_t i = 0; i < ArraySize(Hierarchy); i++)
	{
		auto& cur = *nodes[i];

		rmatrix* last_mat_inv_ptr = parent ? &last_mat_inv : nullptr;
		D3DXMatrixIdentity(&mat);
		GetRotAniMat(cur, last_mat_inv_ptr, Frame, mat);
		GetPosAniMat(cur, last_mat_inv_ptr, Frame, mat);

		float ratio = 0;

		switch (HierarchyParts[i])
		{
		case eq_parts_pos_info_Head:
			ratio = 0.3;
			break;
		case eq_parts_pos_info_Spine1:
			ratio = 0.6;
			break;
		case eq_parts_pos_info_Spine2:
			ratio = 0.5;
			break;
		default:
			goto no_calc_lookat;
		};

		{
			float y_clamped = y;

#define MAX_YA_FRONT	50.f
#define MAX_YA_BACK		-70.f

			if (y_clamped > MAX_YA_FRONT)	y_clamped = MAX_YA_FRONT;
			if (y_clamped < MAX_YA_BACK)		y_clamped = MAX_YA_BACK;

			auto my = RGetRotY(y_clamped * ratio);

			mat *= my;
		}

	no_calc_lookat:

		if (parent)
			mat *= last_mat;

		/*DMLog("%d:\n", i);
		DLogMatrix(mat);

		DMLog("%s:\n", Hierarchy[i]);
		DLogMatrix(cur.m_mat_base);*/

		parent = true;
		last_mat = mat;
		RMatInv(last_mat_inv, mat);
	}

	return mat * World;
}

static void GetRotAniMat(RMeshNode& node, const matrix* parent_base_inv, int frame, matrix& mat)
{
	D3DXMATRIX buffer, Inv;

	bool bAni = false;

	auto* anode = node.m_pAnimationNode;

	if (anode && anode->m_rot_cnt)
		bAni = true;

	if (bAni) {
		D3DXQUATERNION out = anode->GetRotValue(frame);

		D3DXMatrixRotationQuaternion(&mat, &out);
	}
	else {

		D3DXMatrixIdentity(&buffer);

		if (parent_base_inv) {
			D3DXMatrixMultiply(&buffer, &node.m_mat_base, parent_base_inv);
		}
		else {
			memcpy(&buffer, &node.m_mat_base, sizeof(D3DXMATRIX));
		}

		buffer._41 = buffer._42 = buffer._43 = 0;
		mat *= buffer;
	}
}

static void GetPosAniMat(RMeshNode& node, const matrix* parent_base_inv, int frame, matrix& mat)
{
	D3DXMATRIX buffer, Inv;

	bool bAni = false;
	
	auto* anode = node.m_pAnimationNode;

	if (anode && anode->m_pos_cnt)
	{
		bAni = true;
	}

	if (bAni) {
		auto pos = anode->GetPosValue(frame);

		for (int i = 0; i < 3; i++)
			mat(3, i) = pos[i];
	}
	else {

		D3DXMatrixIdentity(&buffer);

		if (parent_base_inv) {
			D3DXMatrixMultiply(&buffer, &node.m_mat_base, parent_base_inv);
		}
		else {
			memcpy(&buffer, &node.m_mat_base, sizeof(D3DXMATRIX));
		}

		for (int i = 0; i < 3; i++)
			mat(3, i) = buffer(3, i);
	}
}

RRESULT OnRender(void *pParam)
{
	__BP(101, "main::OnRender");
	if( !RIsActive() && RIsFullScreen() )
	{
		__EP(101);
		return R_NOTREADY;
	}

	g_App.OnDraw();

	int nFPSLimit = ZGetConfiguration()->GetFPSLimit(); // Static object, FPS limit is set in ctor
	if (nFPSLimit > 0)
	{
		unsigned __int64 nTPS, CurTime;
		QueryPerformanceFrequency((PLARGE_INTEGER)&nTPS);
		QueryPerformanceCounter((PLARGE_INTEGER)&CurTime);

		static int nFrames = 0;
		static unsigned __int64 LastSec = 0;

		if (CurTime - LastSec > nTPS){
			QueryPerformanceCounter((PLARGE_INTEGER)&LastSec);
			nFrames = 0;
		}

		double fActual = double(nFrames) / (nFPSLimit - 1);
		double fGoal = double((CurTime - LastSec) % nTPS) / nTPS;
		int nSleep = int((fActual - fGoal) * 1000);
		if (nSleep > 0){
			if (nSleep > 250)
				MLog("Large sleep %d!\n", nSleep);
			else
				Sleep(nSleep);
		}

		nFrames++;
	}

#ifdef _SMOOTHLOOP
	Sleep(10);
#endif

//#ifndef _PUBLISH

	if(g_pDefFont) {
		char __buffer[256];

		sprintf_safe(__buffer, "FPS: %3.3f", g_fFPS);
		g_pDefFont->m_Font.DrawText( MGetWorkspaceWidth()-150,0,__buffer );

#ifdef _DEBUG
		if (ZGetGame() && ZGetGame()->m_pMyCharacter)
		{
			v3 pos = ZGetGame()->m_pMyCharacter->GetPosition();
			sprintf_safe(__buffer, "Pos: %d, %d, %d", int(pos.x), int(pos.y), int(pos.z));
			g_pDefFont->m_Font.DrawText(MGetWorkspaceWidth() - 200, 30, __buffer);

			if (ZGetGame()->m_pMyCharacter->m_pVMesh)
			{
				pos = ZGetGame()->m_pMyCharacter->m_pVMesh->GetHeadPosition();
				sprintf_safe(__buffer, "Head pos: %d, %d, %d", int(pos.x), int(pos.y), int(pos.z));
				g_pDefFont->m_Font.DrawText(MGetWorkspaceWidth() - 200, 60, __buffer);

#if 1
				[&]()
				{
					rmatrix mat;

					D3DXMatrixIdentity(&mat);

					auto pVMesh = ZGetGame()->m_pMyCharacter->m_pVMesh;
					auto pMesh = pVMesh->m_pMesh;
					auto pNode = pVMesh->m_pMesh->FindNode(eq_parts_pos_info_Head);

					if (!pNode)
						return;

					auto frame = pNode->GetNodeAniSetFrame();

					auto pANode = pNode->m_pAnimationNode;

					auto Pos = pANode->GetPosValue(frame);
					auto Rot = pANode->GetRotValue(frame);

					/*DMLog("%d, %d\n", pANode->m_pos_cnt, pANode->m_rot_cnt);

					DMLog("Pos: %f, %f, %f\n", Pos.x, Pos.y, Pos.z);
					DMLog("pNode->->m_Name = %s\n", pNode->m_Name.c_str());
					DMLog("pNode->m_pParent->m_Name = %s\n", pNode->m_pParent->m_Name.c_str());*/

					D3DXMatrixRotationQuaternion(&mat, &Rot);

					matrix buffer, Inv;
					//D3DXMatrixIdentity(&buffer);

					//if (pMesh->m_isNPCMesh && (pNode->m_WeaponDummyType != weapon_dummy_etc)) // 총구용 더미면
					//{
					//	memcpy(&buffer, &pNode->m_mat_local, sizeof(D3DXMATRIX));

					//}
					//else {

					//	if (pNode->m_pParent) {
					//		RMatInv(Inv, pNode->m_pParent->m_mat_base);
					//		D3DXMatrixMultiply(&buffer, &pNode->m_mat_base, &Inv);
					//	}
					//	else {
					//		memcpy(&buffer, &pNode->m_mat_local, sizeof(D3DXMATRIX));
					//	}
					//}

					//buffer._41 = buffer._42 = buffer._43 = 0;
					//D3DXMatrixMultiply(&mat, &mat, &buffer);

					if (pANode->m_pos_cnt)
						for (int i = 0; i < 3; i++)
							mat(3, i) = Pos[i];
					else
					{
						D3DXMatrixIdentity(&buffer);

						//DMLog("%d\n", pMesh->m_isNPCMesh);

						auto Neck = pMesh->m_ani_mgr.GetAnimation("idle");

						auto Pos = Neck->m_pAniData->GetNode("Bip01 Neck")->GetPosValue(frame);

						//DMLog("Idle neck pos: %f, %f, %f\n", Pos.x, Pos.y, Pos.z);

						//		if( pNode->m_pParentMesh->m_isNPCMesh && pNode->m_WeaponDummyType != weapon_dummy_etc ) // 총구용 더미면
						if (pMesh->m_isNPCMesh && pNode->m_WeaponDummyType != weapon_dummy_etc) // 총구용 더미면
						{
							buffer = pNode->m_mat_local;

						}
						else {
							if (pNode->m_pParent) {
								buffer = pNode->m_mat_base * pNode->m_pParent->m_mat_inv;
							}
							else {
								buffer = pNode->m_mat_local;
							}
						}

						mat._41 = buffer._41;
						mat._42 = buffer._42;
						mat._43 = buffer._43;

						/*DMLog("Base:\n");
						for (int i = 0; i < 4; i++)
						{
							for (int j = 0; j < 4; j++)
							{
								DMLog("%f ", pNode->m_mat_base(i, j));
							}

							DMLog("\n");
						}

						DMLog("Parent base:\n");
						for (int i = 0; i < 4; i++)
						{
							for (int j = 0; j < 4; j++)
							{
								DMLog("%f ", pNode->m_pParent->m_mat_base(i, j));
							}

							DMLog("\n");
						}

						DMLog("Parent inv:\n");
						for (int i = 0; i < 4; i++)
						{
							for (int j = 0; j < 4; j++)
							{
								DMLog("%f ", pNode->m_pParent->m_mat_inv(i, j));
							}

							DMLog("\n");
						}*/
					}

					float rot_y = (ZGetGame()->m_pMyCharacter->GetDirection().z + 0.05) * 50;

#define MAX_YA_FRONT	50.f
#define MAX_YA_BACK		-70.f
					if (rot_y > MAX_YA_FRONT)	rot_y = MAX_YA_FRONT;
					if (rot_y < MAX_YA_BACK)		rot_y = MAX_YA_BACK;

					auto RotY = RGetRotY(rot_y * 0.3);

					mat *= RotY;
					
					if (pNode->m_pParent)
						mat *= pNode->m_pParent->m_mat_result;

					/*auto p = pNode;

					int i = 0;
					while (p)
					{
						DMLog("%d: %s, %d\n", i, p->GetName(), p->m_PartsPosInfoType);
						p = p->m_pParent;
						i++;
					}*/

					matrix World;
					MakeWorldMatrix(&World, ZGetGame()->m_pMyCharacter->GetPosition(), ZGetGame()->m_pMyCharacter->m_vProxyDirection, v3(0, 0, 1));

					mat *= World;

					pos = v3(mat(3, 0), mat(3, 1), mat(3, 2));

					sprintf_safe(__buffer, "Head pos2: %d, %d, %d", int(pos.x), int(pos.y), int(pos.z));
					g_pDefFont->m_Font.DrawText(MGetWorkspaceWidth() - 200, 90, __buffer);

					D3DXMatrixIdentity(&mat);
					pVMesh->m_pMesh->_RGetRotAniMat(pNode, frame, mat);
					pVMesh->m_pMesh->_RGetPosAniMat(pNode, frame, mat);
					pVMesh->m_pMesh->CalcLookAtParts(mat, pNode, pVMesh);

					if (pNode->m_pParent)
						mat *= pNode->m_pParent->m_mat_result;

					mat *= World;

					pos = v3(mat(3, 0), mat(3, 1), mat(3, 2));

					sprintf_safe(__buffer, "Head pos3: %d, %d, %d", int(pos.x), int(pos.y), int(pos.z));
					g_pDefFont->m_Font.DrawText(MGetWorkspaceWidth() - 200, 120, __buffer);

					mat = GetHeadMatrix(World, rot_y, (MMatchSex)!ZGetGame()->m_pMyCharacter->IsMan(), ZGetGame()->m_pMyCharacter->GetStateLower(), frame);

					pos = v3(mat(3, 0), mat(3, 1), mat(3, 2));

					sprintf_safe(__buffer, "Head pos4: %d, %d, %d", int(pos.x), int(pos.y), int(pos.z));
					g_pDefFont->m_Font.DrawText(MGetWorkspaceWidth() - 200, 150, __buffer);

					mat = GetHeadMatrix(World, rot_y, (MMatchSex)!ZGetGame()->m_pMyCharacter->IsMan(), ZGetGame()->m_pMyCharacter->GetStateLower(), frame, *pNode);

					pos = v3(mat(3, 0), mat(3, 1), mat(3, 2));

					sprintf_safe(__buffer, "Head pos5: %d, %d, %d", int(pos.x), int(pos.y), int(pos.z));
					g_pDefFont->m_Font.DrawText(MGetWorkspaceWidth() - 200, 180, __buffer);

					/*if (pNode->m_pParent)
					{
						for (int i = 0; i < 4; i++)
						{
							for (int j = 0; j < 4; j++)
							{
								DMLog("%f ", pNode->m_pParent->m_mat_result(i, j));
							}

							DMLog("\n");
						}
					}*/
				}();
#endif
			}
		}
#endif
	}

	g_RGMain->OnRender();

//#endif

	__EP(101);

	return R_OK;
}

RRESULT OnInvalidate(void *pParam)
{
	MBitmapR2::m_dwStateBlock=NULL;

	g_App.OnInvalidate();
	
	return R_OK;
}

RRESULT OnRestore(void *pParam)
{
	for(int i=0; i<MBitmapManager::GetCount(); i++){
		MBitmapR2* pBitmap = (MBitmapR2*)MBitmapManager::Get(i);
		pBitmap->OnLostDevice();
	}

	g_App.OnRestore();

	return R_OK;
}

RRESULT OnActivate(void *pParam)
{
	if (ZGetGameInterface() && ZGetGameClient() && Z_ETC_BOOST)
		ZGetGameClient()->PriorityBoost(true);
	return R_OK;
}

RRESULT OnDeActivate(void *pParam)
{
	if (ZGetGameInterface() && ZGetGameClient())
		ZGetGameClient()->PriorityBoost(false);
	return R_OK;
}

RRESULT OnError(void *pParam)
{
	mlog("RealSpace::OnError(%d) \n", RGetLastError());

	switch (RGetLastError())
	{
	case RERROR_INVALID_DEVICE:
		{
			D3DADAPTER_IDENTIFIER9 *ai=RGetAdapterID();
			char szLog[512];
			ZTransMsg( szLog, MSG_DONOTSUPPORT_GPCARD, 1, ai->Description);

			int ret=MessageBox(NULL, szLog, ZMsg( MSG_WARNING), MB_YESNO);
			if(ret!=IDYES)
				return R_UNKNOWN;
		}
		break;
	case RERROR_CANNOT_CREATE_D3D:
		{
			ShowCursor(TRUE);

			char szLog[512];
			sprintf_safe(szLog, ZMsg( MSG_DIRECTX_NOT_INSTALL));

			int ret=MessageBox(NULL, szLog, ZMsg( MSG_WARNING), MB_YESNO);
			if(ret==IDYES)
			{
				ShellExecute(g_hWnd, "open", ZMsg( MSG_DIRECTX_DOWNLOAD_URL), NULL, NULL, SW_SHOWNORMAL); 
			}
		}
		break;

	};

	return R_OK;
}

void SetModeParams()
{
#ifdef _PUBLISH
	g_ModeParams.bFullScreen = ZGetConfiguration()->GetVideo()->bFullScreen;
#else
	#ifdef _DEBUG
		g_ModeParams.bFullScreen = false;
	#else
		g_ModeParams.bFullScreen = ZGetConfiguration()->GetVideo()->bFullScreen;
	#endif
#endif
	
	g_ModeParams.nWidth = ZGetConfiguration()->GetVideo()->nWidth;
	g_ModeParams.nHeight = ZGetConfiguration()->GetVideo()->nHeight;
	/*ZGetConfiguration()->GetVideo()->nColorBits == 32 ? 
	g_ModeParams.PixelFormat = D3DFMT_X8R8G8B8 : g_ModeParams.PixelFormat = D3DFMT_R5G6B5 ;*/

}

// 느려도 관계없다~~ -.-

int FindStringPos(char* str,char* word)
{
	if(!str || str[0]==0)	return -1;
	if(!word || word[0]==0)	return -1;

	int str_len = (int)strlen(str);
	int word_len = (int)strlen(word);

	char c;
	bool bCheck = false;

	for(int i=0;i<str_len;i++) {
		c = str[i];
		if(c == word[0]) {

			bCheck = true;

			for(int j=1;j<word_len;j++) {
				if(str[i+j]!=word[j]) {
					bCheck = false;
					break;
				}
			}

			if(bCheck) {
				return i;
			}
		}
	}
	return -1;
}

bool FindCrashFunc(char* pName)
{
//	Function Name
//	File Name 
	if(!pName) return false;

	FILE *fp;
	fp = fopen( "mlog.txt", "r" );
	if(fp==NULL)  return false;

	fseek(fp,0,SEEK_END);
	int size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	char* pBuffer = new char [size];

	fread(pBuffer,1,size,fp);

	fclose(fp);

	// 우리 쏘스에서 찾는다.
	int posSource = FindStringPos(pBuffer,"ublish");
	if(posSource==-1) return false;

	int posA = FindStringPos(pBuffer+posSource,"Function Name");
//	int posB = FindStringPos(pBuffer,"File Name");	
	// filename 이 없는 경우도 있어서 이렇게 바꿨다
	int posB = posA + FindStringPos(pBuffer+posSource+posA,"\n");

	if(posA==-1) return false;
	if(posB==-1) return false;

	posA += 16;

//	int memsize = posB-posA-6;
	int memsize = posB-posA;
	memcpy(pName,&pBuffer[posA+posSource],memsize);

	pName[memsize] = 0;

	delete [] pBuffer;

	for(int i=0;i<memsize;i++) {
		if(pName[i]==':') {
			pName[i] = '-';
		}
	}

	return true;
}

void HandleExceptionLog()
{
	#define ERROR_REPORT_FOLDER	"ReportError"

	extern char* logfilename;	// Instance on MDebug.cpp

	// ERROR_REPORT_FOLDER 존재하는지 검사하고, 없으면 생성
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile(ERROR_REPORT_FOLDER, &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE) {
		if (!CreateDirectory("ReportError", NULL)) {
			MessageBox(g_hWnd, "ReportError 폴더를 생성할 수 없습니다.", APPLICATION_NAME , MB_ICONERROR|MB_OK);
			return;
		}
	} else 	{
		FindClose(hFind);
	}


	// mlog.txt 를 ERROR_REPORT_FOLDER 로 복사

	//acesaga_0928_911_moanus_rslog.txt
	//USAGE_EX) BAReport app=acesaga;addr=moon.maiet.net;port=21;id=ftp;passwd=ftp@;gid=10;user=moanus;localfile=rslog.txt;remotefile=remote_rslog.txt;
/*
	if(ZGetCharacterManager()) {
		ZGetCharacterManager()->OutputDebugString_CharacterState();
	}
*/	
	ZGameClient* pClient = (ZGameClient*)ZGameClient::GetInstance();

	char* pszCharName = NULL;
	MUID uidChar;
	MMatchObjCache* pObj;
	char szPlayer[128];

	if( pClient ) {

		uidChar = pClient->GetPlayerUID();
		pObj = pClient->FindObjCache(uidChar);
		if (pObj)
			pszCharName = pObj->GetName();

		wsprintf(szPlayer, "%s(%d%d)", pszCharName?pszCharName:"Unknown", uidChar.High, uidChar.Low);
	}
	else { 
		wsprintf(szPlayer, "Unknown(-1.-1)");
	}


//	if (pClient) {

		time_t currtime;
		time(&currtime);
		struct tm* pTM = localtime(&currtime);

		char cFuncName[1024];

		if(FindCrashFunc(cFuncName)==false) {
			strcpy_safe(cFuncName,"Unknown Error");
		}

		char szFileName[_MAX_DIR], szDumpFileName[_MAX_DIR];
		wsprintf(szFileName, "%s_%s_%.2d%.2d_%.2d%.2d_%s_%s", cFuncName,
				APPLICATION_NAME, pTM->tm_mon+1, pTM->tm_mday, pTM->tm_hour, pTM->tm_min, szPlayer, "mlog.txt");
		wsprintf(szDumpFileName, "%s.dmp", szFileName);

		char szFullFileName[_MAX_DIR], szDumpFullFileName[_MAX_DIR];
		wsprintf(szFullFileName, "%s/%s", ERROR_REPORT_FOLDER, szFileName);
		wsprintf(szDumpFullFileName, "%s/%s", ERROR_REPORT_FOLDER, szDumpFileName);

		if (CopyFile("mlog.txt", szFullFileName, TRUE))
		{
			CopyFile("Gunz.dmp", szDumpFullFileName, TRUE);

			// BAReport 실행
			char szCmd[4048];
			char szRemoteFileName[_MAX_DIR], szRemoteDumpFileName[_MAX_DIR];
			wsprintf(szRemoteFileName, "%s/%s/%s", ZGetConfiguration()->GetBAReportDir(), "gunzlog", szFileName);
			wsprintf(szRemoteDumpFileName, "%s/%s/%s", ZGetConfiguration()->GetBAReportDir(), "gunzlog", szDumpFileName);

			wsprintf(szCmd, "BAReport app=%s;addr=%s;port=21;id=ftp;passwd=ftp@;user=%s;localfile=%s,%s;remotefile=%s,%s", 
				APPLICATION_NAME, ZGetConfiguration()->GetBAReportAddr(), szPlayer, szFullFileName, szDumpFullFileName, szRemoteFileName, szRemoteDumpFileName);

			WinExec(szCmd, SW_SHOW);

			FILE *file = fopen("bareportpara.txt","w+");
			fprintf(file,szCmd);
			fclose(file);
		}	
//	}
}

LONG_PTR FAR PASCAL WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_SYSCHAR:
			if(ZIsLaunchDevelop() && wParam==VK_RETURN)
			{
#ifndef _PUBLISH
				RFrame_ToggleFullScreen();
#endif
				return 0;
			}
			break;

		case WM_CREATE:
			if (strlen(Z_LOCALE_HOMEPAGE_TITLE) > 0)
			{
				ShowIExplorer(false, Z_LOCALE_HOMEPAGE_TITLE);
			}
			break;
		case WM_DESTROY:
			if (strlen(Z_LOCALE_HOMEPAGE_TITLE) > 0)
			{
				ShowIExplorer(true, Z_LOCALE_HOMEPAGE_TITLE);
			}
			break;
		case WM_SETCURSOR:
			if(ZApplication::GetGameInterface())
				ZApplication::GetGameInterface()->OnResetCursor();
			return TRUE; // prevent Windows from setting cursor to window class cursor

			/*
		case  WM_LBUTTONDOWN:
			SetCapture(hWnd);
			return TRUE;
		case WM_LBUTTONUP:
			ReleaseCapture();
			return TRUE;
			*/
		case WM_KEYDOWN:
			{
				bool b = false;
			}
	}

	if(Mint::GetInstance()->ProcessEvent(hWnd, message, wParam, lParam)==true)
		return 0;

	// thread safe하기위해 넣음
	if (message == WM_CHANGE_GAMESTATE)
	{
		_ZChangeGameState(wParam);
	}


	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*
class mtrl {
public:

};

class node {
public:
	int		m_nIndex[5];
};


class _map{
public:
	mtrl* GetMtrl(node* node,int index) { return GetMtrl(node->m_nIndex[index]); }
	mtrl* GetMtrl(int id) { return m_pIndex[id]; }

	mtrl*	m_pIndex[5];
};

class game {
public:
	_map m_map;	
};

game _game;
game* g_game;
*/

void ClearTrashFiles()
{
}

bool CheckFileList()
{
	return true;
	MZFileSystem *pfs=ZApplication::GetFileSystem();
	MZFile mzf;
	if(!mzf.Open("system/filelist.xml",pfs))
		return false;

	char *buffer;
	buffer=new char[mzf.GetLength()+1];
	mzf.Read(buffer,mzf.GetLength());
	buffer[mzf.GetLength()]=0;

	MXmlDocument aXml;
	aXml.Create();
	if(!aXml.LoadFromMemory(buffer))
	{
		delete buffer;
		return false;
	}

	delete buffer;

	int iCount, i;
	MXmlElement		aParent, aChild;
	aParent = aXml.GetDocumentElement();
	iCount = aParent.GetChildNodeCount();

	char szTagName[256];
	for (i = 0; i < iCount; i++)
	{
		aChild = aParent.GetChildNode(i);
		aChild.GetTagName(szTagName);
		if(_stricmp(szTagName,"FILE")==0)
		{
			char szContents[256],szCrc32[256];
			aChild.GetAttribute(szContents,"NAME");
			aChild.GetAttribute(szCrc32,"CRC32");

			if(_stricmp(szContents,"config.xml")!=0)
			{
				unsigned int crc32_list = pfs->GetCRC32(szContents);
				unsigned int crc32_current;
				sscanf(szCrc32,"%x",&crc32_current);

				if(crc32_current!=crc32_list)
				{
					mlog("crc error , file %s ( current = %x, original = %x ).\n",szContents,crc32_current,crc32_list);
					// 모든 파일을 검사는 한다
					//	return false; 
				}
			}
		}
	}

	return true;
}

enum RBASE_FONT{
	RBASE_FONT_GULIM = 0,
	RBASE_FONT_BATANG = 1,

	RBASE_FONT_END
};

static int g_base_font[RBASE_FONT_END];
static char g_UserDefineFont[256];

bool _GetFileFontName(char* pUserDefineFont)
{
	if(pUserDefineFont==NULL) return false;

	FILE* fp = fopen("_Font", "rt");
	if (fp) {
		fgets(pUserDefineFont,256, fp);
		fclose(fp);
		return true;
	}
	return false;
}


bool CheckFont()
{
	char FontPath[MAX_PATH];
	char FontNames[MAX_PATH+100];

	::GetWindowsDirectory(FontPath, MAX_PATH);

	strcpy_safe(FontNames,FontPath);
	strcat(FontNames, "\\Fonts\\gulim.ttc");

	if (_access(FontNames,0) != -1)	{ g_base_font[RBASE_FONT_GULIM] = 1; }
	else							{ g_base_font[RBASE_FONT_GULIM] = 0; }

	strcpy_safe(FontNames,FontPath);
	strcat(FontNames, "\\Fonts\\batang.ttc");

	if (_access(FontNames,0) != -1)	{ g_base_font[RBASE_FONT_BATANG] = 1; }
	else							{ g_base_font[RBASE_FONT_BATANG] = 0; }

	//	strcpy_safe(FontNames,FontPath);
	//	strcat(FontNames, "\\Fonts\\System.ttc");
	//	if (_access(FontNames,0) != -1)	{ g_font[RBASE_FONT_BATANG] = 1; }
	//	else							{ g_font[RBASE_FONT_BATANG] = 0; }

	if(g_base_font[RBASE_FONT_GULIM]==0 && g_base_font[RBASE_FONT_BATANG]==0) {//둘다없으면..

		if( _access("_Font",0) != -1) { // 이미 기록되어 있다면..
			_GetFileFontName( g_UserDefineFont );
		}
		else {

			int hr = IDOK;

			//hr = ::MessageBox(NULL,"귀하의 컴퓨터에는 건즈가 사용하는 (굴림,돋움) 폰트가 없는 것 같습니다.\n 다른 폰트를 선택 하시겠습니까?","알림",MB_OKCANCEL);
			//hr = ::MessageBox(NULL,"귀하의 컴퓨터에는 건즈가 사용하는 (굴림,돋움) 폰트가 없는 것 같습니다.\n 계속 진행 하시겠습니까?","알림",MB_OKCANCEL);

			if(hr==IDOK) {
				/*			
				CFontDialog dlg;
				if(dlg.DoModal()==IDOK) {
				CString facename = dlg.GetFaceName();
				lstrcpy_safe((LPSTR)g_UserDefineFont,(LPSTR)facename.operator const char*());

				hr = ::MessageBox(NULL,"선택하신 폰트를 저장 하시겠습니까?","알림",MB_OKCANCEL);

				if(hr==IDOK)
				_SetFileFontName(g_UserDefineFont);
				}
				*/
				return true;
			}
			else {
				return false;
			}
		}
	}
	return true;
}

#include "shlobj.h"

void CheckFileAssociation()
{
#define GUNZ_REPLAY_CLASS_NAME	"GunzReplay"

	// 체크해봐서 등록이 안되어있으면 등록한다. 사용자에게 물어볼수도 있겠다.
	char szValue[256];
	if (!MRegistry::Read(HKEY_CLASSES_ROOT, "." GUNZ_REC_FILE_EXT, NULL, szValue))
	{
		MRegistry::Write(HKEY_CLASSES_ROOT, "." GUNZ_REC_FILE_EXT, NULL, GUNZ_REPLAY_CLASS_NAME);

		char szModuleFileName[_MAX_PATH] = { 0, };
		GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);

		char szCommand[_MAX_PATH];
		sprintf_safe(szCommand, "\"%s\" \"%%1\"", szModuleFileName);

		MRegistry::Write(HKEY_CLASSES_ROOT, GUNZ_REPLAY_CLASS_NAME "\\shell\\open\\command", NULL, szCommand);

		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_FLUSH, NULL, NULL);
	}
}

void UpgradeMrsFile()
{
	char temp_path[ 1024];
	sprintf_safe( temp_path,"*");

	FFileList file_list;
	GetFindFileListWin(temp_path,".mrs",file_list);
	file_list.UpgradeMrs();
}

HANDLE Mutex = 0;

#ifdef _HSHIELD
int __stdcall AhnHS_Callback(long lCode, long lParamSize, void* pParam);
#endif

DWORD g_dwMainThreadID;

int PASCAL WinMain(HINSTANCE this_inst, HINSTANCE prev_inst, LPSTR cmdline, int cmdshow)
{
	//MessageBox(0, "hi", "hi", 0);

	SetUnhandledExceptionFilter(static_cast<LONG(__stdcall *)(_EXCEPTION_POINTERS *)>(
		[](_EXCEPTION_POINTERS *p) -> LONG {
		return CrashExceptionDump(p, "Gunz.dmp");
	}
	));

	_set_invalid_parameter_handler([](const wchar_t* expression,
		const wchar_t* function,
		const wchar_t* file,
		unsigned int line,
		uintptr_t pReserved)
	{
		MLog("Invalid parameter detected in function %s.\nFile: %s, line: %d.\nExpression: %s.\n", function, file, line, expression);
	});

	g_dwMainThreadID = GetCurrentThreadId();
	
#ifdef _MTRACEMEMORY
	MInitTraceMemory();
#endif

//	_CrtSetBreakAlloc(25483);

	// Current Directory를 맞춘다.
	char szModuleFileName[_MAX_DIR] = {0,};
	GetModuleFileName(NULL, szModuleFileName, _MAX_DIR);
	PathRemoveFileSpec(szModuleFileName);
	SetCurrentDirectory(szModuleFileName);

#ifdef _PUBLISH
	Mutex = CreateMutex(NULL, TRUE, "RGunz");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(0, "Refined Gunz is already running", "RGunz", 0);
		exit(-1);
		return 0;
	}
#endif

	InitLog(MLOGSTYLE_DEBUGSTRING | MLOGSTYLE_FILE);

	ClearTrashFiles();

	srand((unsigned int)time(nullptr));


	mlog("Refined Gunz version %d launched. Build date: " __DATE__ " " __TIME__ "\n", RGUNZ_VERSION);

	char szDateRun[128]="";
	char szTimeRun[128]="";
	_strdate( szDateRun );
	_strtime( szTimeRun );
	mlog("Log time (%s %s)\n", szDateRun, szTimeRun);


#ifndef _PUBLISH
	mlog("cmdline = %s\n",cmdline);
#endif

#ifndef _LAUNCHER
	// Don't know why this is here
	//UpgradeMrsFile();// mrs1 이라면 mrs2로 업그래이드 한다..
#endif

	MSysInfoLog();

//	if (CheckVideoAdapterSupported() == false)
//		return 0;

	CheckFileAssociation();

	// Initialize MZFileSystem - MUpdate 
	MRegistry::szApplicationName=APPLICATION_NAME;

	g_App.InitFileSystem();
//	mlog("CheckSum: %u \n", ZApplication::GetFileSystem()->GetTotalCRC());

	//if(!InitializeMessage(ZApplication::GetFileSystem())) {
	//	MLog("Check Messages.xml\n");
	//	return 0;
	//}

//	넷마블 버전은 구분해야함... 넷마블 버전은 MZIPREADFLAG_MRS1 도 읽어야함...

#ifdef _PUBLISH
//	#ifndef NETMARBLE_VERSION
		MZFile::SetReadMode( MZIPREADFLAG_MRS2 );
//	#endif
#endif

	// StringRes먼저 로드하고 그다음에 Config을 로드한다.
	ZGetConfiguration()->Load();

	g_RGMain = new RGMain;

	ZStringResManager::MakeInstance();
	if( !ZApplication::GetInstance()->InitLocale() )
	{
		MLog("Locale Init error !!!\n");
		return false;
	}

	// 여기서 메크로 컨버팅... 먼가 구리구리~~ -by SungE.
	if( !ZGetConfiguration()->LateStringConvert() )
	{
		MLog( "main.cpp - Late string convert fail.\n" );
		return false;
	}

	DWORD ver_major = 0;
	DWORD ver_minor = 0;
	TCHAR ver_letter = ' ';

	// 의미없음 ... 외부에서 dll 이 없다고 먼저뜸...

/*_
	bool DXCheck = false;

	if( SUCCEEDED( GetDirectXVersionViaDxDiag( &ver_major, &ver_minor, &ver_letter ) ) ) {
		if(ver_major >= 8)
			DXCheck = true;
	} // 성공 못한 경우 알수없으므로 실패~

	if(DXCheck==false) {
		::MessageBox(NULL,"DirectX 8.0 이상을 설치하고 다시 실행해 주시기 바랍니다.","알림",MB_OK);
	}
*/

	if (ZApplication::GetInstance()->ParseArguments(cmdline) == false)
	{
		// Korean or Japan Version
		if ((ZGetLocale()->GetCountry() == MC_KOREA) || (ZGetLocale()->GetCountry() == MC_JAPAN))
		{
			mlog("Routed to Website \n");

			ShellExecute(NULL, "open", ZGetConfiguration()->GetLocale()->szHomepageUrl, NULL, NULL, SW_SHOWNORMAL);

			char szMsgWarning[128]="";
			char szMsgCertFail[128]="";
			ZTransMsg(szMsgWarning,MSG_WARNING);
			ZTransMsg(szMsgCertFail,MSG_REROUTE_TO_WEBSITE);
//			MessageBox(g_hWnd, szMsgCertFail, szMsgWarning, MB_OK);

			mlog(szMsgWarning);
			mlog(" : ");
			mlog(szMsgCertFail);

			return 0;
		}
		else
		{
			return 0;
		}
	}

//#ifdef _PUBLISH
	//if(!CheckFileList()) {
	//	// 종료하는것은 일단 보류
	//	// int ret=MessageBox(NULL, "파일이 손상되었습니다.", "중요!", MB_OK);
	//	// return 0;
	//}
//#endif

//	if (ZCheckHackProcess() == true)
//	{
////		MessageBox(NULL,
////			ZMsg(MSG_HACKING_DETECTED), ZMsg( MSG_WARNING), MB_OK);
//		mlog(ZMsg(MSG_HACKING_DETECTED));
//		mlog("\n");
//		return 0;
//	}

	if(!InitializeNotify(ZApplication::GetFileSystem())) {
		MLog("Check notify.xml\n");
		return 0;
	}

	// font 있는가 검사..

	if(CheckFont()==false) {
		MLog("폰트가 없는 유저가 폰트 선택을 취소\n");
		return 0;
	}

	RSetFunction(RF_CREATE	,	OnCreate);
	RSetFunction(RF_RENDER	,	OnRender);
	RSetFunction(RF_UPDATE	,	OnUpdate);
	RSetFunction(RF_DESTROY ,	OnDestroy);
	RSetFunction(RF_INVALIDATE,	OnInvalidate);
	RSetFunction(RF_RESTORE,	OnRestore);
	RSetFunction(RF_ACTIVATE,	OnActivate);
	RSetFunction(RF_DEACTIVATE,	OnDeActivate);
	RSetFunction(RF_ERROR,		OnError);

	SetModeParams();

//	while(ShowCursor(FALSE)>0);

	int nReturn = RMain(APPLICATION_NAME, this_inst, prev_inst, cmdline, cmdshow, &g_ModeParams, WndProc, IDI_ICON1);

#ifdef _MTRACEMEMORY
	MShutdownTraceMemory();
#endif
	return nReturn;

#ifdef _PUBLISH
	if (Mutex != 0) CloseHandle(Mutex);
#endif

	return 0;
}