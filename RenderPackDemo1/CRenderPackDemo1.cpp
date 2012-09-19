//////////////////////////////////////////////////////////////////////////
//		CRenderPackDemo2.cpp	
//		*******************************************************									
//
//		Definition of your main application class.
//		RenderPack Wizard generated file.
//		
//////////////////////////////////////////////////////////////////////////

#include "DllHeader.h"

#include "CRenderPackDemo1.h"

#include "Render/D3D11/Util/CommonConstants.h"

#include "Core/HLSLEx.h"

#include "Util\Logging\CWinConsoleListener.h"

#include "FTHelper.h"

//#include <WinDef.h>

#include <fstream>
#include <sstream>

#include "Util\CharUtil.h"

#include "NuiAPI.h"
#include "FaceTrackLib.h"





//float CRenderPackDemo1::m_MonitorHeight = 0;
//float CRenderPackDemo1::m_MonitorWidth = 0;
//float CRenderPackDemo1::m_KinectPosition = -1;

//////////////////////////////////////////////////////////////////////////
// CRenderPackDemo2

CRenderPackDemo1::CRenderPackDemo1()
{
}

CRenderPackDemo1::~CRenderPackDemo1()
{
}

bool CRenderPackDemo1::Init()
{
	if(!CSimpleApp11::Init())
		return false;


	flag = false;

	isTracked = false;
	//load scene from OBJ file.	
	CObjLoaderRef rObjLoader = new CObjLoader();
//	CSimpleLoaderRef rSimpleLoader;
	
	rObjLoader->SetLogWriter(GetLogWriter());

	CObjLoaderRef rObjLoaderF = new CObjLoader();
	rObjLoaderF->SetLogWriter(GetLogWriter());
	
	std::wstring path = L"Resources\\sponza_noflag.obj";
	std::wstring path2 = L"Resources\\candide1.obj";

	ParseObjInput();
	m_IsFaceTracked = false;
	float i;
	char *inname = "monitor.txt";
	std::ifstream infile(inname);


	int kinectPos = 0;
	int j = 0;
	while (infile >> i){
		if(j == 0) {
			m_MonitorWidth = (i);
		} else if (j == 1) {
			m_MonitorHeight = i;
		} else if (j == 2) {
				m_KinectPosition = i;
		}

		j++;
	}

	
	NUI_IMAGE_RESOLUTION depthRes = NUI_IMAGE_RESOLUTION_320x240;
	NUI_IMAGE_RESOLUTION colorRes = NUI_IMAGE_RESOLUTION_640x480;

	if(SUCCEEDED((m_FTHelper).Init(m_hWnd, FaceTracking, this, NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, depthRes, true, true,NUI_IMAGE_TYPE_COLOR , colorRes, true))) {
		SYSLOG("Init",1,"If m_FTHelper");
	}
	m_FTHelper.SetViewOffset(0,0);//###
	if(m_KinectPosition < 0) {
		//###m_FTHelper.SetViewOffset(0,0);
		m_currFTTranslationsXYZ = float3(0.f,0.161545,1.1f);//###
	} else if (m_KinectPosition > 0) {
		m_currFTTranslationsXYZ = float3(0.f,0.161545,1.1f);//###
	}
	//this is just to demonstrate how to write custom log messages
	SYSLOG("CRenderPackDemo2.Init", 1, "Loading scene from file."<<path2);
	
	m_rMeshResource = CTriMeshResource::RegisterResource(m_rResManager, L"Mesh", path.c_str(), rObjLoader);
	
	//CTriMesh* rMeshF;
	//CCachedFileResourceBase *foo;/// = new CCachedFileResourceBase(path2.c_str());
	
	//m_rMeshResourceFace = CTriMeshResource::RegisterResource(m_rResManager, L"MeshFace", path2.c_str(), rSimpleLoader);
	//bool CSimpleLoader::LoadTriMesh(const wchar_t* FileName, CTriMesh** ppMesh, class CCachedFileResourceBase* pResource)
	//rSimpleLoader->LoadTriMesh(path2.c_str(),(&rMeshF));

	if(!m_rResManager->BlockingUpdateAll(true))
		return false;

	SYSLOG("CRenderPackDemo2.Init", 1, "Scene loaded successfully");

	//scale the scene intp the unit cube
	
	//(rMeshF)->NormalizeToUnitCube();
	//size_t numVerts = (rMeshF)->GetNumVertices();
	//SYSLOG("CRPD1.InitI()",1,"GetNumVertices of rMeshF "<<numVerts);
	CTriMeshRef rMesh = m_rMeshResource->GetTriMesh();
	rMesh->NormalizeToUnitCube();

	m_reset = false;
	for(int i = 0; i < 6; i++) {
		m_TurnStatus[i] = false;
	}

	m_CurrTranslateSpeed = float3(0.f,0.f,0.f);
//	hlsl::float4x4 camTransform = hlsl::translation<float, 4, 4>(hlsl::float3(0.01f, 0.01f, 0.01f));
	//###hlsl::float4x4 camTransform = hlsl::translation<float, 4, 4>(hlsl::float3(0.16f, 0.33f, 0.01f)); 
	hlsl::float4x4 camFaceTransform = hlsl::translation<float, 4, 4>(hlsl::float3(0.16f, .1f, 0.01f));
	m_rCam = new CCamera(); 
	m_rCam->SetLocalTransform(camFaceTransform); 
	m_rCam->SetViewParams(hlsl::float3(0.0f, .1f, 0.f), hlsl::float3(0, 1.0f, 0)); 
	m_rCam->ForceUpVector(true);
	m_rCam->SetProjParams(1.6f, 0.78f, 0.04f, 100000.0f);
	
	hlsl::float4x4 camTransform = hlsl::translation<float, 4, 4>(hlsl::float3(0.f, -.2f, 2.f));
	m_rFaceCam = new CCamera(); 
	m_rFaceCam->SetLocalTransform(camTransform); 
	m_rFaceCam->SetViewParams(hlsl::float3(0.0f, -.2f, 0.f), hlsl::float3(0, 1.0f, 0)); 
	m_rFaceCam->ForceUpVector(true);
	m_rFaceCam->SetProjParams(1.6f, 0.78f, 0.04f, 100000.0f);


	z0 = 0.01f;
	z1 = 100000.f;
	y1 = m_MonitorHeight/2.0f;
	//SYSLOG("init",1,"ywert "<<y1);
	y0 = -y1;
	x1 = m_MonitorWidth/2.0f;
	x0 = -x1;
	// control the camera
	m_rFreeFlight = new CFreeFlightController();	
	m_rFreeFlight->SetControlTarget(m_rCam);
	m_rFreeFlight->SetInvertMouse(true);
	AddAnimatable(m_rFreeFlight);
	AddMessageHandler(m_rFreeFlight);
		
	return true;
}


void  CRenderPackDemo1::MonitorInputs(char *string) {
	SYSLOG("Monitor", 1, "input "<<string);

}


	HRESULT CRenderPackDemo1::OnCreateDevice(ID3D11Device* pDevice)
{
	HRESULT hr;

	//create renderable geometry
	CD3D11MeshFactoryRef rFactory = new CD3D11MeshFactory(pDevice);
	m_rRenderFullQuad = rFactory->CreateFullScreenQuad();

	//create pipeline states
	CD3D11RasterizerStateRef rRastWire = CD3D11RasterizerState::CreatePredefined(CD3D11RasterizerState::WIREFRAME_NO_CULL);
	V_RETURN_HR(rRastWire->CreateOnDevice(pDevice));

	CD3D11BlendStateRef rNoBlend = CD3D11BlendState::CreatePredefined(CD3D11BlendState::NO_BLEND);
	V_RETURN_HR(rNoBlend->CreateOnDevice(pDevice));

	CD3D11DepthStencilStateRef rDepthDisable = CD3D11DepthStencilState::CreatePredefined(CD3D11DepthStencilState::DISABLE_DEPTH);
	V_RETURN_HR(rDepthDisable->CreateOnDevice(pDevice));

	//declare Shaders
	VertexShaderResourceRef rVSFullQuad = UseVSResource(L"VSFullQuad", pDevice, L"Resources\\Shaders\\VS_FullQuad.hlsl", "VSMain");

	PixelShaderResourceRef rPSFullQuad = UsePSResource(L"PSFullQuad", pDevice, L"Resources\\Shaders\\PS_FullQuad.hlsl", "PSMain");
	
	const char* channelNames[3] = {"POSITION", "NORMAL", "TEXCOORD"};
	//const char* channelNamesF[1] = {"POSITION"};
	//UINT channelSizesF[1] = {12};
	UINT channelSizes[3] = {12, 12, 8}; //float3, float3, float2
	m_rRenderMesh = rFactory->CreateRenderMesh(m_rMeshResource, channelNames, channelSizes, 3, m_rResManager);
	//we don't need the data on the CPU anymore
	m_rMeshResource->GetTriMesh()->ReleaseGeometryData();


	unsigned int nVerts = numOfFaceVerts;// m_rMeshResourceFace->GetTriMesh()->GetNumVertices();

	//vertexbuffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd,sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(hlsl::float4)*numOfFaceVerts;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA InitDataV;
	ZeroMemory(&InitDataV, sizeof(InitDataV));
	InitDataV.pSysMem = m_vertexData;
	if(FAILED(pDevice->CreateBuffer(&bd, &InitDataV, &m_vertexBuffer)))
		return false;

	//indexbuffer
	D3D11_BUFFER_DESC bdi;
	ZeroMemory(&bdi,sizeof(bdi));
	bdi.Usage = D3D11_USAGE_DEFAULT;
	bdi.ByteWidth = sizeof(int)*numOfFaceFaces;
	bdi.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bdi.CPUAccessFlags = 0;
	bdi.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA InitDataI;
	ZeroMemory(&InitDataI, sizeof(InitDataI));
	InitDataI.pSysMem = m_faceIndices;

	if(FAILED(pDevice->CreateBuffer(&bdi, &InitDataI, &m_indexBuffer)))
		return false;
	
	// TO DO: fill up the init data
	
	ParseDataInput();

	SYSLOG("CRPD1.OCD!",1,"Parsedata 2 nVerts "<<nVerts<<" nAUs "<<nAUs);
	hlsl::float3 *pAnimPositions = new hlsl::float3[nVerts];//* (nAUs+1)s
	CD3D11StructuredDataBuffer::BUFFER_DESC bufDesc;
	bufDesc.NumElements = nVerts ;//* (nAUs+1)
	bufDesc.StructureStride = 3 * sizeof(float);
	//std::vector<hlsl::float4>& m_vertexData = m_rMeshResourceFace->GetTriMesh()->GetVertexData().GetChannels()["POSITION"].GetData();
	
	int tmpVertIdx = 0;
	for(unsigned int iVert = 0; iVert < nVerts; iVert++)
	{
		/*SYSLOG("CRPD1.OCD",1,"vertexData "<<vertexData[tmpVertIdx].xyz);*/
		pAnimPositions[iVert] = m_vertexData[tmpVertIdx++].xyz;
		
		if(tmpVertIdx == nVerts ) {
			tmpVertIdx = 0;
		}
	}

	int tmp_whatever = 0;
	for( int i = 0; i < nAUs; i++) {
		SYSLOG("CRPD1.OCD",1,"pAU_nVerts idx "<<i<<" v "<<pAU_nVerts[i].x);
		tmp_whatever += pAU_nVerts[i].x;
	}

	for(int j = 0; j < tmp_whatever; j++) {
		SYSLOG("CRPD1.OCD",1,"pAnimationUnits idx "<<j<<" x "<<pAnimationUnits[j].x<<" y "<<pAnimationUnits[j].y<<" z "<<pAnimationUnits[j].z<<" w "<<pAnimationUnits[j].w);
	}

	int tmp_nVertsAu = (pAU_nVerts[0]).x;
	for(unsigned int j = 0; j < tmp_nVertsAu; j++) {
		unsigned int tmp_idx = pAnimationUnits[j].w;
		float weight = -1.f;
		SYSLOG("CRPD1.OCD",1,"weight j "<<j<<" x "<<(pAnimationUnits[j]).x<<" y "<<(pAnimationUnits[j]).y<<" z "<<(pAnimationUnits[j]).z)
		pAnimPositions[tmp_idx].x = pAnimPositions[tmp_idx].x + weight*(pAnimationUnits[j]).x;
		pAnimPositions[tmp_idx].y = pAnimPositions[tmp_idx].y + weight*(pAnimationUnits[j]).y;
		pAnimPositions[tmp_idx].z = pAnimPositions[tmp_idx].z + weight*(pAnimationUnits[j]).z;
	}

	//int allNVerts = 0;
	//for(unsigned int i = 0; i < nAUs; i++) {//####nAUS
	//	int tmp_nVertsAU = ((pAU_nVerts)[i]).x;
	//	//SYSLOG("CRPD1.OCD",1,"tmp_nVertsAU "<<tmp_nVertsAU);
	//	for(unsigned int j = 0; j < tmp_nVertsAU; j++) {
	//		int idx = allNVerts + j;
	//		//unsigned int tmp_idx = ((pAnimationUnits)[idx]).w;
	//		unsigned int tmp_idx = nVerts*(i+1)+((pAnimationUnits)[idx]).w;
	//		//SYSLOG("CRPD1.OCD",1,"tmp_idx "<<tmp_idx);
	//		pAnimPositions[tmp_idx] = (pAnimationUnits)[idx].xyz;
	//	}
	//	allNVerts = allNVerts + tmp_nVertsAU;
	//}

	m_rAnimationData = new CD3D11StructuredDataBuffer(bufDesc, pAnimPositions);
	V_RETURN_HR(m_rAnimationData->CreateOnDevice(pDevice));
	

	CD3D11RasterizerStateRef rRastSolid = CD3D11RasterizerState::CreatePredefined(CD3D11RasterizerState::SOLID);
	V_RETURN_HR(rRastSolid->CreateOnDevice(pDevice));


	CD3D11DepthStencilStateRef rDepthEnable = CD3D11DepthStencilState::CreatePredefined(CD3D11DepthStencilState::ENABLE_DEPTH);
	V_RETURN_HR(rDepthEnable->CreateOnDevice(pDevice));

	//define rendering passes
	m_rFullScreenPass = new CD3D11RenderConfig(
		rVSFullQuad,
		NULL, NULL, NULL,
		rPSFullQuad,
		rRastSolid,
		rDepthDisable,
		rNoBlend
		);
	//create input layouts
	m_rFullQuadLayout = new CD3D11InputLayout(m_rRenderFullQuad, rVSFullQuad);


	//constant buffers
	m_rCBObjectTransform = new CD3D11ConstantBuffer(sizeof(CB_PER_OBJECT));
	m_rCBObjectTransform->SetDebugName("CB_PER_OBJECT");
	V_RETURN_HR(m_rCBObjectTransform->CreateOnDevice(pDevice));


	m_rCBObjectFaceAUs = new CD3D11ConstantBuffer(sizeof(CB_PER_OBJECT));
	m_rCBObjectFaceAUs->SetDebugName("CB_PER_OBJECT");
	V_RETURN_HR(m_rCBObjectFaceAUs->CreateOnDevice(pDevice));

	//samplers
	CD3D11SamplerStateRef rSamPoint = CD3D11SamplerState::CreatePredefined(CD3D11SamplerState::POINT_CLAMP);
	V_RETURN_HR(rSamPoint->CreateOnDevice(pDevice));

	CD3D11SamplerStateRef rSamLinear = CD3D11SamplerState::CreatePredefined(CD3D11SamplerState::LINEAR_WRAP);
	V_RETURN_HR(rSamLinear->CreateOnDevice(pDevice));

	//declare shaders
	VertexShaderResourceRef rVSTextured = UseVSResource(L"VSTextured", pDevice, L"Resources\\Shaders\\VS_Textured.hlsl", "VSMain");
	rVSTextured->BindConstantBuffer(0, m_rCBObjectTransform);

	PixelShaderResourceRef rPSTextured = UsePSResource(L"PSTextured", pDevice, L"Resources\\Shaders\\PS_Textured.hlsl", "PSMain");
	rPSTextured->BindSampler(0, rSamLinear);

	VertexShaderResourceRef rVSTexturedF = UseVSResource(L"VSTexturedF", pDevice, L"Resources\\Shaders\\VS_TexturedF.hlsl", "VSMain");
	rVSTexturedF->BindConstantBuffer(0, m_rCBObjectFaceAUs);


	m_rTexturedPass = new CD3D11RenderConfig(
		rVSTextured,
		NULL, NULL, NULL,
		rPSTextured,
		rRastSolid,
		rDepthEnable,
		rNoBlend
		);

	m_rFacePass = new CD3D11RenderConfig(
		rVSTexturedF,
		NULL, NULL, NULL, 
		rPSTextured,
		rRastWire,
		rDepthDisable,
		rNoBlend
		);

	//create input layouts
	D3D11_INPUT_ELEMENT_DESC layout[] = {{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}};
	UINT numElements = ARRAYSIZE(layout);
	//((CD3D11VertexShaderRef)rVSTexturedF)->CreateInputLayout(layout, numElements, &m_rMeshLayoutFace);
	CD3D11VertexShader vertexShader = static_cast<CD3D11VertexShader>( m_rFacePass->GetVertexShader());	
	vertexShader.CompileOnDevice(pDevice);
	vertexShader.CreateInputLayout(layout, numElements, &m_rMeshLayoutFace);
	m_rMeshLayout = new CD3D11InputLayout(m_rRenderMesh->GetRenderGeometry(), rVSTextured);
	//m_rMeshLayoutFace = new CD3D11InputLayout(m_rRenderMeshFace->GetRenderGeometry(), rVSTexturedF);

	//always call this in the last line! (this will setup the magic resource management behind the scenes)
	V_RETURN_HR( CSimpleApp11::OnCreateDevice(pDevice) );

	return S_OK;
}

void CRenderPackDemo1::OnDestroyDevice()
{
	m_rMeshResource = NULL;
	m_rRenderMesh = NULL;
	m_rMeshLayout = NULL;
	m_rTexturedPass = NULL;
	m_rFacePass = NULL;
	m_rCBObjectTransform = NULL;
	m_rRenderFullQuad = NULL;
	m_rFullScreenPass = NULL;
	m_rFullQuadLayout = NULL;
	m_currFTTranslationsXYZ = NULL;
	m_CurrTranslateSpeed = NULL;
	m_ftfrustum = NULL;
	m_rCBObjectFaceAUs = NULL;
	m_rMeshLayoutFace = NULL;
	m_rMeshResourceFace = NULL;
	m_rRenderMeshFace = NULL;
	m_TurnStatus[6] = NULL;
	translateOffset = NULL;
	m_rAnimationData = NULL;
	m_rColorTarget = NULL;
	m_rTexFaceColor = NULL;
	m_rTexColor = NULL;
	m_rTexDepth = NULL;
	pAnimationUnits = NULL;
	pAU_nVerts = NULL;
	
	m_hWnd = NULL;
	m_rCam  = NULL;
	m_rFaceCam = NULL;
	m_rFreeFlight = NULL;
	m_rCamControl = NULL;
	//SYSLOG("OnDestroyDevice",1,"reference Count m_FTHelper "<<m_FTHelper.;

	
	CSimpleApp11::OnDestroyDevice();

}

void CRenderPackDemo1::FacetrackingTranslations(FLOAT translationXYZ[3]) {

	m_IsFaceTracked = true;//###
		float yoffset = 0.001f*0.196545f;
	/*SYSLOG("FTTranslations",1, "x "<<translationXYZ[0]);
	SYSLOG("FTTranslations",1, "y "<<translationXYZ[1]);
	SYSLOG("FTTranslations",1, "z "<<translationXYZ[2]);*/
	m_currFTTranslationsXYZ[0] = (translationXYZ[0] - m_currFTTranslationsXYZ[0]);
	m_currFTTranslationsXYZ[1] = (translationXYZ[1] - m_currFTTranslationsXYZ[1]);
	m_currFTTranslationsXYZ[2] = (translationXYZ[2] - m_currFTTranslationsXYZ[2]);

	//SYSLOG("FTTranslations",1, "cx "<<m_currFTTranslationsXYZ[0]);
	//SYSLOG("FTTranslations",1, "cy "<<m_currFTTranslationsXYZ[1]);
	//SYSLOG("FTTranslations",1, "cz "<<m_currFTTranslationsXYZ[2]);

	float cali = .1f;// 0.015f;
	x0 -= cali*m_currFTTranslationsXYZ[0];
	x1 -= cali*m_currFTTranslationsXYZ[0];
	y0 -= cali*m_currFTTranslationsXYZ[1];
	y1 -= cali*m_currFTTranslationsXYZ[1];
	z0 -= cali*m_currFTTranslationsXYZ[2];
	z1 -= cali*m_currFTTranslationsXYZ[2];
	translateOffset[0] += cali*m_currFTTranslationsXYZ[0];
	translateOffset[1] += cali*m_currFTTranslationsXYZ[1];
	translateOffset[2] += cali*m_currFTTranslationsXYZ[2];
	m_currFTTranslationsXYZ[0] = translationXYZ[0];
	m_currFTTranslationsXYZ[1] = translationXYZ[1];
	m_currFTTranslationsXYZ[2] = translationXYZ[2];
	

}

void CRenderPackDemo1::ParseObjInput() {
	std::wstring path = L"Resources\\candide3.obj";
	hlsl::float3 tmp_Position;
	int nVerts = 0;
	int nFaces = 0;

	//file input
	char strMatFileName[MAX_PATH];
	char strCommand[MAX_PATH];

	std::string sPath = CCharUtil::FromWStringToString(path);

	std::ifstream inFile(sPath.c_str());

	std::stringstream buffer;
	buffer<<inFile.rdbuf();
	inFile.close();

	

	std::stringstream copyBuffer(buffer.str());
	nVerts = 0;
#pragma warning(disable: 4127)
	while(true)
#pragma warning(default: 4127)
	{
		if(copyBuffer.eof())
			break;
		copyBuffer>>strCommand;
		//SYSLOG("CRPD1.PDI",1,"CommandStream "<<strCommand);
		
		if(0 == strcmp( strCommand, "#")) {
			//comment
		} else if (0 == strcmp( strCommand, "vn")) {
			
			nVerts++;
			
		} else if (0 == strcmp(strCommand, "f")) {
			
			nFaces++;
		}
		copyBuffer.ignore( 1000, '\n');
	} 

	numOfFaceFaces = nFaces*3;
	numOfFaceVerts = nVerts;

	m_vertexData = new hlsl::float4[numOfFaceVerts];
	m_faceIndices = new int[numOfFaceFaces];
	
	
	int posIdx = 0;
	size_t auIdx = 0;

#pragma warning(disable: 4127)
	while(true)
#pragma warning(default: 4127)
	{
		buffer>>strCommand;
		if(!buffer)
			break;

		if(0 == strcmp( strCommand, "#")) {
			//comment
		} else if( 0 == strcmp( strCommand, "f")) {
			int l,m,n;
			buffer>>l>>m>>n;
			(m_faceIndices[auIdx++]) = l;
			(m_faceIndices[auIdx++]) = m;
			(m_faceIndices[auIdx++]) = n;
	//		SYSLOG("CRPD1.POI",1,"face von parseobjectinput "<<l<<" "<<m<<" "<<n);
	
		} else if (0 == strcmp( strCommand, "vn"))  {
		
			float x,y,z;
			buffer>>x>>y>>z;
			//SYSLOG("CRPD1.POI",1,"x "<<x<<" y "<<y<<" z "<<z<<" "<<posIdx);
			//idx = idx+1;
			((m_vertexData)[posIdx]).xyzw = hlsl::float4(x,y,z,(posIdx));
		//	SYSLOG("CRPD1.POI",1,"vertices von parseobjectinput "<<x<<" "<<y<<" "<<z<<" "<<posIdx);
			posIdx++;
			
		} 
	}

		char nextChar = buffer.peek();
		if(nextChar == ' ')
			buffer.get();

		
	
}

void CRenderPackDemo1::ParseDataInput() {
	std::wstring path = L"Resources\\candide1.wfm";
	hlsl::float3 tmp_Position;
	int tmp_amount;
	std::wstring tmp_AUName;
	int tmp_pAU = 0;
	int nVertsAU = 0;

	
	//file input
	char strMatFileName[MAX_PATH];
	char strCommand[MAX_PATH];

	std::string sPath = CCharUtil::FromWStringToString(path);

	std::ifstream inFile(sPath.c_str());

	std::stringstream buffer;
	buffer<<inFile.rdbuf();
	inFile.close();

	std::stringstream copyBuffer(buffer.str());
	nAUs = 0;
#pragma warning(disable: 4127)
	while(true)
#pragma warning(default: 4127)
	{
		if(copyBuffer.eof())
			break;
		copyBuffer>>strCommand;
		//SYSLOG("CRPD1.PDI",1,"CommandStream "<<strCommand);
		
		if (0 == strcmp( strCommand, "n")) {
		//	SYSLOG("CRPD1.PDI",1,"Count nAUs "<<nAUs);
			nAUs++;
			int tmp_nVertsAU;
			copyBuffer>>tmp_nVertsAU;
			nVertsAU += tmp_nVertsAU;
		}
		copyBuffer.ignore( 1000, '\n');
	} 
	(pAnimationUnits) = new hlsl::float4[nVertsAU * nAUs]; // bestehen aus den xyz-Werten und dem Indize
	(pAU_nVerts) = new hlsl::int1[nAUs];
	
	int tmp_amountVert;
	size_t posIdx = 0;
	size_t auIdx = 0;

#pragma warning(disable: 4127)
	while(true)
#pragma warning(default: 4127)
	{
		buffer>>strCommand;
		if(!buffer)
			break;

		if(0 == strcmp( strCommand, "#")) {
			//comment
		} else if( 0 == strcmp( strCommand, "n")) {
			buffer>>tmp_amountVert;
			(pAU_nVerts[auIdx++]).x = tmp_amountVert;
	
		} else if (0 == strcmp( strCommand, "v"))  {
			int idx;
			buffer>>idx;
			float x,y,z;
			buffer>>x>>y>>z;

			//idx = idx+1;
			((pAnimationUnits)[posIdx++]).xyzw = hlsl::float4(x,y,z,idx);
		} 
	}

		char nextChar = buffer.peek();
		if(nextChar == ' ')
			buffer.get();

}

void CRenderPackDemo1::FacetrackingAnimating(FLOAT *pCoefficients, unsigned int AUCount) {
	SYSLOG("CRPD1.FTA",1,"ppCoefficients 1 "<<(pCoefficients)[0]<<" pAUCount "<<AUCount);
	SYSLOG("CRPD1.FTA",1,"ppCoefficients 2 "<<(pCoefficients)[1]<<" pAUCount "<<AUCount);
	SYSLOG("CRPD1.FTA",1,"ppCoefficients 3 "<<(pCoefficients)[2]<<" pAUCount "<<AUCount);
	SYSLOG("CRPD1.FTA",1,"ppCoefficients 4 "<<(pCoefficients)[3]<<" pAUCount "<<AUCount);
	SYSLOG("CRPD1.FTA",1,"ppCoefficients 5 "<<(pCoefficients)[4]<<" pAUCount "<<AUCount);
	SYSLOG("CRPD1.FTA",1,"ppCoefficients 6 "<<(pCoefficients)[5]<<" pAUCount "<<AUCount);

}

void CRenderPackDemo1::FaceTracking(PVOID pVoid) {


	CRenderPackDemo1 * pApp = reinterpret_cast<CRenderPackDemo1*>(pVoid);
	if (pApp) {
		IFTResult *pResult = pApp->m_FTHelper.GetResult();
		FLOAT* pAU = NULL;
		UINT numAU;
		if(pResult && SUCCEEDED(pResult->GetStatus())) {
	
			
			pResult->GetAUCoefficients(&pAU, &numAU);
			FLOAT scale;
			FLOAT rotationXYZ[3];
			FLOAT translationXYZ[3];
			pResult->Get3DPose(&scale, rotationXYZ, translationXYZ);

	//###		pApp->FacetrackingTranslations(translationXYZ);
			pApp->FacetrackingFrustum(pApp->m_MonitorWidth,pApp->m_MonitorHeight, pApp->m_KinectPosition, translationXYZ);
			pApp->FacetrackingAnimating(pAU, numAU);
		}
	}

	//SYSLOG("ft",1,"track a face");
}

void CRenderPackDemo1::MsgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam, bool* pbNoFurtherProcessing) {
	m_hWnd = hWnd;
	CSimpleApp11::MsgProc(hWnd, uMsg, wParam, lParam, pbNoFurtherProcessing);

}

HRESULT CRenderPackDemo1::OnSwapChainResized(ID3D11Device* pDevice, IDXGISwapChain* pSwapChain,
                                          const DXGI_SURFACE_DESC* pBackBufferSurfaceDesc)
{
	HRESULT hr;

	V_RETURN_HR( CSimpleApp11::OnSwapChainResized(pDevice, pSwapChain, pBackBufferSurfaceDesc) );
	
	m_rCam->SetProjParams(float(pBackBufferSurfaceDesc->Width) / float(pBackBufferSurfaceDesc->Height), 1.28f, 0.02f, 1000.0f);

	
	//define viewport
	D3D11_VIEWPORT port =
	{
		0.f, 0.f,
	    pBackBufferSurfaceDesc->Width,
		pBackBufferSurfaceDesc->Height,
		0.0f,
		1.0f
	};

	//texture to store scene color
	CD3D11RenderTexture2D::TEXTURE_DESC colorDesc;
	//texture to store faces
	//CD3D11RenderTexture2D::TEXTURE_DESC faceDesc;

	//use default values
	colorDesc.Width = pBackBufferSurfaceDesc->Width;
	colorDesc.Height = pBackBufferSurfaceDesc->Height;

	m_rTexColor = new CD3D11RenderTexture2D(colorDesc);
	V_RETURN_HR(m_rTexColor->CreateOnDevice(pDevice));

	//unsigned int nVerts = m_rMeshResourceFace->GetTriMesh()->GetNumVertices();
	//
	//faceDesc.Width =  nVerts; 
	//faceDesc.Height = 1;

	//D3D11_SUBRESOURCE_DATA initData;
	//initData.SysMemPitch = nVerts*4*4; 
	//initData.SysMemSlicePitch = 1;
	//initData.pSysMem = &(m_rMeshResourceFace->GetTriMesh()->GetVertexData().GetChannels()["POSITION"].GetData()[0]);// initData.pSysMem = pointer to your data

 //	m_rTexFaceColor = new CD3D11RenderTexture2D(faceDesc, &initData);

	//V_RETURN_HR(m_rTexFaceColor->CreateOnDevice(pDevice));

	//Texture for depth buffer
	CD3D11DepthTexture2D::TEXTURE_DESC depthDesc;
	depthDesc.Width = pBackBufferSurfaceDesc->Width;
	depthDesc.Height = pBackBufferSurfaceDesc->Height;

	m_rTexDepth = new CD3D11DepthTexture2D(depthDesc);
	V_RETURN_HR(m_rTexDepth->CreateOnDevice(pDevice));

	// configure render targets
	ID3D11RenderTargetView* pRTVs[1] = {m_rTexColor->AsRTV()};//, m_rTexFaceColor->AsRTV()
	ID3D11DepthStencilView* pDSVs[1] = {m_rTexDepth->AsDSV()};
	m_rColorTarget = new CD3D11RenderTargetConfig(pRTVs, 1, pDSVs, 1, port);

	return S_OK;
}

void CRenderPackDemo1::OnSwapChainReleasing()
{

	m_rColorTarget = NULL;
	m_rTexFaceColor = NULL;
	m_rTexColor = NULL;
	m_rTexDepth = NULL;
	CSimpleApp11::OnSwapChainReleasing();
}

void CRenderPackDemo1::OnFrameMove(double Time, float ElapsedTime)
{
	translateOffset = float3(0.f,0.f,0.f);

	float currSpeed = length(m_CurrTranslateSpeed);
	float deltaSpeed = 1.0f * ElapsedTime;

	if(deltaSpeed < currSpeed)
		m_CurrTranslateSpeed -= deltaSpeed * hlsl::normalize(m_CurrTranslateSpeed);
	else
		m_CurrTranslateSpeed = hlsl::float3(0);
	
	if(m_TurnStatus[TURN_DOWN]) //k
	{
	
		translateOffset -= float3(0.f,(0.01f*ElapsedTime),0.f);
		y0 += .01f* ElapsedTime;
		y1 += .01f * ElapsedTime;
		SYSLOG("OnFrameMove:", 1, "down y0 "<<y0<<" y1 "<<y1<<br);
	}
	if(m_TurnStatus[TURN_UP]) //i
	{
		translateOffset += float3(0.f,(0.01f*ElapsedTime),0.f);
		y0 -= .01f* ElapsedTime;
		y1 -= .01f * ElapsedTime;
		SYSLOG("OnFrameMove:", 1, "up y0 "<<y0<<" y1 "<<y1<<br);
	}
	if(m_TurnStatus[TURN_LEFT]) //j
	{ 
		translateOffset -= float3((0.01f*ElapsedTime),0.f,0.f);
		x0 += .01f* ElapsedTime;
		x1 += .01f * ElapsedTime;
		SYSLOG("OnFrameMove:", 1, "left x0 "<<x0<<" x1 "<<x1<<br);
	}
	if(m_TurnStatus[TURN_RIGHT]) //l
	{
		translateOffset += float3((0.01f*ElapsedTime),0.f,0.f);
		x0 -= .01f* ElapsedTime;
		x1 -= .01f * ElapsedTime;
		SYSLOG("OnFrameMove:", 1, "right x0 "<<x0<<" x1 "<<x1<<br);
	}
	if(m_TurnStatus[TURN_AWAY]) {//u
		translateOffset -= float3(0.f,0.f,(0.01f*ElapsedTime));
		z0 += .01f * ElapsedTime;
		z1 += .01f * ElapsedTime;
		SYSLOG("OnFrameMove:", 1, "away z0 "<<z0<<" z1 "<<z1<<br);
	}
	if(m_TurnStatus[TURN_CLOSE]) {//o 
		if((z0 - 0.01f*ElapsedTime) > 0.0f){
			translateOffset += float3(0.f,0.f,(0.01f*ElapsedTime));
			z0 -= .01f* ElapsedTime;
			z1 -= .01f * ElapsedTime;
		}
		SYSLOG("OnFrameMove:", 1, "close z0 "<<z0<<" z1 "<<z1<<br);
	}

	m_CurrTranslateSpeed += translateOffset;
	if(length(m_CurrTranslateSpeed) > 1.0f)
		m_CurrTranslateSpeed = 1.f * hlsl::normalize(m_CurrTranslateSpeed);

	translateOffset = m_CurrTranslateSpeed * ElapsedTime;


	if(m_reset) {
		z0 = m_rCam->GetNear();
		z1 = m_rCam->GetFar();
		y1 = z0 * hlsl::tan(m_rCam->GetFovy()/2.f);
		y0 = -y1;
		x1 = m_rCam->GetAspect()*y1;
		x0 = -x1;

	}

	CSimpleApp11::OnFrameMove(Time, ElapsedTime);
	m_rCam->UpdateViewParams();
}


void CRenderPackDemo1::ProcessKinectIO() {



}

void CRenderPackDemo1::FacetrackingFrustum(float monitorWidth, float monitorHeight, float kinectPosition, FLOAT translationsKinectXYZ[3]) {
	m_IsFaceTracked = true;
	//SYSLOG("CRPD1::FTF",1,"monitorHeight "<<monitorHeight<<" monitorWidth "<<monitorWidth); 
	//SYSLOG("CRPD1::FTF",1," translationX "<<translationsKinectXYZ[0]<<" translationY "<<translationsKinectXYZ[1]<<" translationZ "<<translationsKinectXYZ[2]);

	float calibration = 2.75f;//slow down motions

	if(kinectPosition < 0) {
		y0 = -0.5*(monitorHeight + calibration*translationsKinectXYZ[1]);
		y1 = -0.5*(calibration*translationsKinectXYZ[1] - monitorHeight);
	} else if (kinectPosition > 0) {
		y0 = 0.5*(calibration*translationsKinectXYZ[1] - monitorHeight);
		y1 = 0.5*(calibration*translationsKinectXYZ[1] + monitorHeight);
	}
	x0 = -1*(monitorWidth/2 +  calibration*translationsKinectXYZ[0]);
	x1 = -1*(calibration*translationsKinectXYZ[0] - monitorWidth/2);
	if(( calibration*translationsKinectXYZ[2] - 2.f) < 0.01f) {
		z0 = 0.01f;
	} else {
		z0 =  calibration*translationsKinectXYZ[2] - 2.f;
	}
	//z1 = z0 + 5.0f;

	
	m_ftfrustum = hlsl::frustum(x0,x1,y0,y1,z0,z1);
	float xOffset = monitorWidth/2 - x1;
	float yOffset = monitorHeight/2 -y1;
	translateOffset = float3(xOffset, yOffset, z0);

	//SYSLOG("CRPD1::FTF",1,"x1-x0 "<<(x1-x0)<<" y1-y0 "<<(y1-y0)<<" y1/z0 "<<(y1/z0));
	//SYSLOG("CRenderPackDemo1-FrameRender",1,"onframerender "<<x0<<" "<<x1<<" "<<y0<<" "<<y1<<" "<<z0<<" "<<z1);
}

void CRenderPackDemo1::OnFrameRender(ID3D11Device* pDevice, ID3D11DeviceContext* pImmediateContext)
{
	CSimpleApp11::OnFrameRender(pDevice, pImmediateContext);
	
	// render the scene from the camera
	// update the matrices
	//translateOffset = float3(0,0.0f,0);
	hlsl::float4x4 mWorld, mView, mFView, mProj, mFProj, mFWorld;
	//m_rMeshResourceFace->GetTriMesh()->GetWorldTransform(&mFWorld);
	m_rMeshResource->GetTriMesh()->GetWorldTransform(&mWorld);
	mView = *m_rCam->GetViewMatrix();
	mFView = *m_rFaceCam->GetViewMatrix();
	hlsl::float4x4 scalerF = hlsl::scale<float,4,4>(hlsl::float3(1500.0,1500.0,1500.0));
	hlsl::float4x4 scaler = hlsl::scale<float,4,4>(hlsl::float3(10.0,10.0,10.0));
	hlsl::float4x4 translateMatrix = hlsl::translation<float,4,4>(translateOffset);
	mFView = mul(scalerF, mFView);
	mView = mul(scaler, mView);
	mProj = *m_rCam->GetProjMatrix();
	mFProj = *m_rFaceCam->GetProjMatrix();

	if(m_IsFaceTracked) {//####
		mProj =  m_ftfrustum;
	}//###
	//mView = hlsl::frustum(0.1f,0.3f,0.1f,0.3f,0.3f,0.2f);
	// use custom projection
	//	mProj = (hlsl::frustum(.1f,0.3f,.1f,.3f,.30f,.2f));
	//mProj  = hlsl::perspective(0.78f,1.33f,0.02f,5.0f);
	
	//mView = (hlsl::look_at(hlsl::vector<float,3>(0.0f,0.0f,-1.0f), hlsl::vector<float,3>(-.19f,.16f,.0f), hlsl::vector<float,3>(.0f,1.0f,.0f)));
	//mProj = hlsl::frustum(-.05f,.035f,-.035f,.035f,.09f,10.f);
	//mProj = (hlsl::frustum(0.0110f,-.0109f,0.00821f,-.00822f,-0.02f,-5.0f));
	//mProj = (hlsl::frustum(2.0f,4.0f,2.0f,4.0f, 3.0f,7.0f));

	/*if(length(translateOffset) != 0.f)
		SYSLOG("CRenderPackDemo1", 1, "mView, translateOffset"<<translateOffset);*/

	hlsl::float4x4 mWorldViewProj = mul(mul(mWorld, mView), mProj);
	hlsl::float4x4 mWorldViewProjF = mul(mul(mWorld, mFView), mFProj);

	//update constant buffer
	m_rCBObjectTransform->Map(pImmediateContext);
		CB_PER_OBJECT* pMappedData = (CB_PER_OBJECT*)m_rCBObjectTransform->GetDataPtr();
		pMappedData->mWorldViewProj = mWorldViewProj;
	m_rCBObjectTransform->Unmap(pImmediateContext);


	m_rCBObjectFaceAUs->Map(pImmediateContext);
		CB_PER_OBJECT* pMappedDataF = (CB_PER_OBJECT*)m_rCBObjectFaceAUs->GetDataPtr();
		pMappedDataF->mWorldViewProj = mWorldViewProjF;
	m_rCBObjectFaceAUs->Unmap(pImmediateContext);


	m_rContextManager->PushRenderTargets(m_rColorTarget);

	//clear render targets
	pImmediateContext->ClearRenderTargetView(m_rTexColor->AsRTV(), hlsl::float4(0.2f,0.3f, 0.2f, 1.0f));
	//pImmediateContext->ClearRenderTargetView(m_rTexFaceColor->AsRTV(), hlsl::float4(0.2f,0.3f, 0.2f, 1.0f));
	pImmediateContext->ClearDepthStencilView(m_rTexDepth->AsDSV(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	// render mesh
	m_rContextManager->SetConfig(m_rTexturedPass);
	pImmediateContext->IASetInputLayout(m_rMeshLayout->GetLayout());
	m_rRenderMesh->BeginDraw(pImmediateContext);
	
	for(UINT iSubset = 0; iSubset < m_rRenderMesh->GetNumSubsets(); iSubset++)
	{
		CD3D11RenderTexture2D* pTex = m_rRenderMesh->GetDiffuseTexture(iSubset);
		ID3D11ShaderResourceView* ppSRVs[1] = { pTex != NULL ? pTex->AsSRV() : NULL };
		pImmediateContext->PSSetShaderResources(0, 1, ppSRVs);
		m_rRenderMesh->DrawSubset(iSubset, pImmediateContext);
	}

	m_rRenderMesh->EndDraw(pImmediateContext);
	m_rContextManager->PopRenderTargets();

	m_rContextManager->PushRenderTargets(m_rColorTarget);
	//Set Vertex Buffer
	UINT stride = sizeof(hlsl::float4);
	UINT offset = 0;
	pImmediateContext->IASetVertexBuffers(0,1,&m_vertexBuffer,&stride, &offset);
	//
	//if(!flag) {
	//	SYSLOG("CRPD1.OFR",1,"numoffacefaces"<<numOfFaceFaces);
	//for(int i = 0; i < numOfFaceFaces; i++) {
	//	//UINT temp2 = dynamic_cast<UINT*>((&m_indexBuffer)[i*sizeof(UINT)]);
	//	int temp = m_faceIndices[i];
	//	SYSLOG("CRPD1.OFR",1,"m_indexBuffer "<<i<<" idx "<<temp);
	//}
	//flag = true;
	//}
	//Set Index Buffer
	pImmediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pImmediateContext->IASetInputLayout(m_rMeshLayoutFace);

	//render face
	m_rContextManager->SetConfig(m_rFacePass);

	pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	

	
	ID3D11ShaderResourceView* pFacePosSRV = m_rAnimationData->AsSRV();

	pImmediateContext->VSSetShaderResources(0,1, &pFacePosSRV);
	pImmediateContext->DrawIndexed(numOfFaceFaces,0,1);

	//pImmediateContext->IASetInputLayout(m_rMeshLayoutFace->GetLayout());
	//m_rRenderMeshFace->BeginDraw(pImmediateContext);
	//for(UINT iSubset = 0; iSubset < m_rRenderMeshFace->GetNumSubsets(); iSubset++)
	//{
	//	CD3D11RenderTexture2D* pTex = m_rRenderMeshFace->GetDiffuseTexture(iSubset);
	//	ID3D11ShaderResourceView* ppSRVs[1] = { pTex != NULL ? pTex->AsSRV() : NULL };

	//	ID3D11ShaderResourceView* pFacePosSRV = m_rAnimationData->AsSRV();
	//	pImmediateContext->PSSetShaderResources(0, 1, ppSRVs);
	//	pImmediateContext->VSSetShaderResources(0, 1, &pFacePosSRV);
	//	m_rRenderMeshFace->DrawSubset(iSubset, pImmediateContext);
	//	ppSRVs[0] = NULL;
	//	pImmediateContext->VSSetShaderResources(0, 1, ppSRVs);
	//}

	//m_rRenderMeshFace->EndDraw(pImmediateContext);


	m_rContextManager->PopRenderTargets();
	//
	
	//render full screen quad
	m_rContextManager->SetConfig(m_rFullScreenPass);
	pImmediateContext->IASetInputLayout(m_rFullQuadLayout->GetLayout());
	m_rRenderFullQuad->Draw(pImmediateContext);

	// TO DO: insert your rendering code here

	ID3D11ShaderResourceView* pSRVs[1] = {m_rTexColor->AsSRV()};//, m_rTexFaceColor->AsSRV()
	pImmediateContext->PSSetShaderResources(0, 1, pSRVs);

	m_rRenderFullQuad->Draw(pImmediateContext);

	pSRVs[0] = NULL;
	//pSRVs[1] = NULL;
	pImmediateContext->PSSetShaderResources(0, 1, pSRVs);

	FinishFrame();
}

void CRenderPackDemo1::OnKeyDown( WPARAM wParam, LPARAM lParam, bool *pNeedFurtherProcessing )
{
	char ch;
	BYTE keyStateArray[256];
	UINT scanCode = (UINT)lParam;
	WORD word;

	GetKeyboardState(keyStateArray);
	ToAscii((UINT)wParam, scanCode, keyStateArray, &word, 0);
	ch = (char)word;

	switch(ch)
	{
		case'k':
			m_TurnStatus[TURN_DOWN] = true;
			*pNeedFurtherProcessing = false;
			break;
		case'i':
			m_TurnStatus[TURN_UP] = true;
			*pNeedFurtherProcessing = false;
			break;
		case'j':
			m_TurnStatus[TURN_LEFT] = true;
			*pNeedFurtherProcessing = false;
			break;
		case'l':
			m_TurnStatus[TURN_RIGHT] = true;
			*pNeedFurtherProcessing = false;
			break;
		case'u':
			m_TurnStatus[TURN_AWAY] = true;
			*pNeedFurtherProcessing = false;
			break;
		case'o':
			m_TurnStatus[TURN_CLOSE] = true;
			*pNeedFurtherProcessing = false;
			break;
		case'b':
			m_reset = true;
			*pNeedFurtherProcessing = false;
			break;

	}

	CSimpleApp11::OnKeyDown(wParam, lParam, pNeedFurtherProcessing);
}


void CRenderPackDemo1::OnKeyUp( WPARAM wParam, LPARAM lParam, bool *pNeedFurtherProcessing )
{
	char ch;
	BYTE keyStateArray[256];
	UINT scanCode = (UINT)lParam;
	WORD word;

	GetKeyboardState(keyStateArray);
	ToAscii((UINT)wParam, scanCode, keyStateArray, &word, 0);
	ch = (char)word;

	switch(ch)
	{
	case'k':

		m_TurnStatus[TURN_DOWN] = false;
		*pNeedFurtherProcessing = false;
		break;
	case'i':

		m_TurnStatus[TURN_UP] = false;
		*pNeedFurtherProcessing = false;
		break;
	case'j':

		m_TurnStatus[TURN_LEFT] = false;
		*pNeedFurtherProcessing = false;
		break;
	case'l':

		m_TurnStatus[TURN_RIGHT] = false;
		*pNeedFurtherProcessing = false;
		break;
	case'u':
		m_TurnStatus[TURN_AWAY] = false;
		*pNeedFurtherProcessing = false;
		break;
	case'o':
		m_TurnStatus[TURN_CLOSE] = false;
		*pNeedFurtherProcessing = false;
		break;
	case'b':
		m_reset = false;
		*pNeedFurtherProcessing = false;
		break;

			}

	CSimpleApp11::OnKeyUp(wParam, lParam, pNeedFurtherProcessing);
}


//////////////////////////////////////////////////////////////////////////
