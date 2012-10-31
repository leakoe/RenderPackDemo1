//////////////////////////////////////////////////////////////////////////
// CRenderPackDemo2.cpp
// *******************************************************
//
// Definition of your main application class.
// RenderPack Wizard generated file.
//
//////////////////////////////////////////////////////////////////////////

#include "DllHeader.h"

#include "CRenderPackDemo1.h"

#include "Render/D3D11/Util/CommonConstants.h"

#include "Core/HLSLEx.h"

#include "Util\Logging\CWinConsoleListener.h"

#include "FTHelper.h"

#include <fstream>
#include <sstream>

#include "Util\CharUtil.h"

#include "NuiAPI.h"
#include "FaceTrackLib.h"

struct CB_PER_AU {
	hlsl::float4 g_pAU_Weights[6];
};


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

	//load scene from OBJ file.
	CObjLoaderRef rObjLoader = new CObjLoader();

	std::wstring path = L"Resources\\sponza_noflag.obj";
	std::wstring path2 = L"Resources\\candide1.obj";

	ParseObjInput();
	m_IsFaceTracked = false;

	// Aus der Datei Monitor.txt werden die Breite und die Höhe ausgelesen außerdem wird die Kinectposition angegeben
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

	m_smoother = new hlsl::float3[10]; //zum Glätten der Bewegung
	smoothflag = false;
	pointToSmooth = 0;
	for (int n = 0; n < 10; n++) {
		m_smoother[n] = float3(0,0,0);
	}

	m_translateOffset2 = float3(0,0,0);

	NUI_IMAGE_RESOLUTION depthRes = NUI_IMAGE_RESOLUTION_320x240;
	NUI_IMAGE_RESOLUTION colorRes = NUI_IMAGE_RESOLUTION_640x480;

	if(SUCCEEDED((m_FTHelper).Init(m_hWnd, FaceTracking, this, NUI_IMAGE_TYPE_DEPTH_AND_PLAYER_INDEX, depthRes, true, true,NUI_IMAGE_TYPE_COLOR , colorRes, true))) {
		
	}
	m_FTHelper.SetViewOffset(0,0);

	m_rMeshResource = CTriMeshResource::RegisterResource(m_rResManager, L"Mesh", path.c_str(), rObjLoader);


	if(!m_rResManager->BlockingUpdateAll(true))
		return false;

	CTriMeshRef rMesh = m_rMeshResource->GetTriMesh();
	rMesh->NormalizeToUnitCube();

	m_reset = false;
	for(int i = 0; i < 6; i++) {
		m_TurnStatus[i] = false;
	}

	m_CurrTranslateSpeed = float3(0.f,0.f,0.f);
	hlsl::float4x4 camFaceTransform = hlsl::translation<float, 4, 4>(hlsl::float3(0.16f, .1f, 0.01f));
	// Kamera der Sponza-Szene
	m_rCam = new CCamera();
	m_rCam->SetLocalTransform(camFaceTransform);
	m_rCam->SetViewParams(hlsl::float3(0.0f, .1f, 0.f), hlsl::float3(0, 1.0f, 0));
	m_rCam->ForceUpVector(true);
	m_rCam->SetProjParams(1.6f, 0.78f, 0.04f, 100000.0f);

	// Kamera für das Gesicht
	hlsl::float4x4 camTransform = hlsl::translation<float, 4, 4>(hlsl::float3(0.f, -.2f, 2.f));
	m_rFaceCam = new CCamera();
	m_rFaceCam->SetLocalTransform(camTransform);
	m_rFaceCam->SetViewParams(hlsl::float3(0.0f, -.2f, 0.f), hlsl::float3(0, 1.0f, 0));
	m_rFaceCam->ForceUpVector(true);
	m_rFaceCam->SetProjParams(1.6f, 0.78f, 0.04f, 100000.0f);


	z0 = 0.01f;
	z1 = 100000.f;
	y1 = m_MonitorHeight/2.0f;
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
	UINT channelSizes[3] = {12, 12, 8}; 
	m_rRenderMesh = rFactory->CreateRenderMesh(m_rMeshResource, channelNames, channelSizes, 3, m_rResManager);
	//we don't need the data on the CPU anymore
	m_rMeshResource->GetTriMesh()->ReleaseGeometryData();

	m_rFaceRotation = float3(0,0,0);

	unsigned int nVerts = numOfFaceVerts;

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

	ParseDataInput();

	CD3D11StructuredDataBuffer::BUFFER_DESC bufDescAU;
	bufDescAU.NumElements = nVerts*nAUs;
	bufDescAU.StructureStride = 4 * sizeof(float);
	m_rSDBAnimationUnits = new CD3D11StructuredDataBuffer(bufDescAU, pAnimationUnits);
	m_rSDBAnimationUnits->SetDebugName("SDB_PER_AU1");
	V_RETURN_HR(m_rSDBAnimationUnits->CreateOnDevice(pDevice));

	hlsl::float3 *pAnimPositions = new hlsl::float3[nVerts];
	CD3D11StructuredDataBuffer::BUFFER_DESC bufDesc;
	bufDesc.NumElements = nVerts;
	bufDesc.StructureStride = 3 * sizeof(float);

	int tmpVertIdx = 0;
	for(unsigned int iVert = 0; iVert < nVerts; iVert++) {
		pAnimPositions[iVert] = m_vertexData[tmpVertIdx++].xyz;
		if(tmpVertIdx == nVerts )
			tmpVertIdx = 0;
	}

	m_rAnimationData = new CD3D11StructuredDataBuffer(bufDesc, pAnimPositions);
	m_rAnimationData->SetDebugName("Structured Positions");
	V_RETURN_HR(m_rAnimationData->CreateOnDevice(pDevice));


	CD3D11RasterizerStateRef rRastSolid = CD3D11RasterizerState::CreatePredefined(CD3D11RasterizerState::SOLID);
	V_RETURN_HR(rRastSolid->CreateOnDevice(pDevice));

	CD3D11RasterizerStateRef rRastSolidNoCull = CD3D11RasterizerState::CreatePredefined(CD3D11RasterizerState::SOLID_NO_CULL);
	V_RETURN_HR(rRastSolidNoCull->CreateOnDevice(pDevice));

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


	m_rCBObjectFaceTransform = new CD3D11ConstantBuffer(sizeof(CB_PER_OBJECT));
	m_rCBObjectFaceTransform->SetDebugName("CB_PER_OBJECT");
	V_RETURN_HR(m_rCBObjectFaceTransform->CreateOnDevice(pDevice));

	m_rCBAUs = new CD3D11ConstantBuffer(sizeof(CB_PER_AU));
	m_rCBAUs->SetDebugName("CB_PER_AU");
	V_RETURN_HR(m_rCBAUs->CreateOnDevice(pDevice));



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

	PixelShaderResourceRef rPSFace = UsePSResource(L"PSFace", pDevice, L"Resources\\Shaders\\PS_Face.hlsl", "PSMain");
	rPSTextured->BindSampler(0, rSamLinear);

	VertexShaderResourceRef rVSFace = UseVSResource(L"VSTexturedF", pDevice, L"Resources\\Shaders\\VS_Face.hlsl", "VSMain");
	rVSFace->BindConstantBuffer(0, m_rCBObjectFaceTransform);
	rVSFace->BindConstantBuffer(4, m_rCBAUs);

	m_rTexturedPass = new CD3D11RenderConfig(
		rVSTextured,
		NULL, NULL, NULL,
		rPSTextured,
		rRastSolid,
		rDepthEnable,
		rNoBlend
		);

	m_rFacePass = new CD3D11RenderConfig(
		rVSFace,
		NULL, NULL, NULL,
		rPSFace,
		rRastWire,
		rDepthDisable,
		rNoBlend
		);

	//create input layouts
	D3D11_INPUT_ELEMENT_DESC layout[] = {{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0}};
	UINT numElements = ARRAYSIZE(layout);

	CD3D11VertexShader vertexShader = static_cast<CD3D11VertexShader>( m_rFacePass->GetVertexShader());
	vertexShader.CompileOnDevice(pDevice);
	vertexShader.CreateInputLayout(layout, numElements, &m_rMeshLayoutFace);
	m_rMeshLayout = new CD3D11InputLayout(m_rRenderMesh->GetRenderGeometry(), rVSTextured);

	//always call this in the last line! (this will setup the magic resource management behind the scenes)
	V_RETURN_HR( CSimpleApp11::OnCreateDevice(pDevice) );

	return S_OK;
}

void CRenderPackDemo1::OnDestroyDevice()
{
	m_vertexBuffer = NULL;
	m_indexBuffer = NULL;
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
	m_rCBObjectFaceTransform = NULL;
	m_rCBAUs = NULL;
	m_rSDBAnimationUnits = NULL;
	m_rMeshLayoutFace = NULL;
	translateOffset = NULL;
	m_rAnimationData = NULL;
	m_rColorTarget = NULL;
	m_rTexColor = NULL;
	m_rTexDepth = NULL;
	pAnimationUnits = NULL;
	pAU_nVerts = NULL;
	m_hWnd = NULL;
	m_rCam = NULL;
	m_rFaceCam = NULL;
	m_rFreeFlight = NULL;
	m_rCamControl = NULL;

	ZeroMemory(m_TurnStatus, sizeof(m_TurnStatus));

	CSimpleApp11::OnDestroyDevice();

}

void CRenderPackDemo1::FacetrackingTranslations(FLOAT translationXYZ[3]) {

	m_IsFaceTracked = true;//###
	m_currFTTranslationsXYZ[0] = (translationXYZ[0] - m_currFTTranslationsXYZ[0]);
	m_currFTTranslationsXYZ[1] = (translationXYZ[1] - m_currFTTranslationsXYZ[1]);
	m_currFTTranslationsXYZ[2] = (translationXYZ[2] - m_currFTTranslationsXYZ[2]);

	float cali = .1f;
	x0 -= cali*m_currFTTranslationsXYZ[0];
	x1 -= cali*m_currFTTranslationsXYZ[0];
	y0 -= cali*m_currFTTranslationsXYZ[1];
	y1 -= cali*m_currFTTranslationsXYZ[1];
	z0 -= cali*m_currFTTranslationsXYZ[2];
	z1 -= cali*m_currFTTranslationsXYZ[2];

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


		} else if (0 == strcmp( strCommand, "vn")) {

			float x,y,z;
			buffer>>x>>y>>z;
			((m_vertexData)[posIdx]).xyzw = hlsl::float4(x,y,z,(posIdx));
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

		if (0 == strcmp( strCommand, "n")) {
			nAUs++;
			int tmp_nVertsAU;
			copyBuffer>>tmp_nVertsAU;
			nVertsAU += tmp_nVertsAU;
		}
		copyBuffer.ignore( 1000, '\n');
	}
	numOfAllVertsAU = nVertsAU;
	(pAnimationUnits) = new hlsl::float4[nAUs*numOfFaceVerts]; // bestehen aus den xyz-Werten und dem Index
	(pAU_nVerts) = new hlsl::int1[nAUs];
	pAnimationUnits[0] = float4(0.1f,0.1f,0.1f,1.0f);
	for (int j = 0; j < nAUs*numOfFaceVerts; j++) {
		pAnimationUnits[j] = float4(0,0,0,1.0f);
	}

	for (int i = 0; i < nAUs; i++) {
		pAU_weights[i] = 0;
	}
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

		} else if (0 == strcmp( strCommand, "v")) {
			int idx;
			buffer>>idx;
			float x,y,z;
			buffer>>x>>y>>z;

			(pAnimationUnits)[(auIdx-1)*numOfFaceVerts + idx].xyzw = hlsl::float4(x,y,z,idx);
		}
	}

	char nextChar = buffer.peek();
	if(nextChar == ' ')
		buffer.get();

}

void CRenderPackDemo1::FacetrackingAnimating(FLOAT *pCoefficients, unsigned int AUCount, FLOAT scale, FLOAT rotationXYZ[3], FLOAT translationXYZ[3] ) {

	m_rFaceRotation.xyz = float3(rotationXYZ[0],rotationXYZ[1],rotationXYZ[2]);

	for(int i = 0; i < AUCount; i++) {
		pAU_weights[i].x = pCoefficients[i];

	}

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

			pApp->FacetrackingFrustum(pApp->m_MonitorWidth,pApp->m_MonitorHeight, pApp->m_KinectPosition, translationXYZ);
			pApp->FacetrackingAnimating(pAU, numAU, scale, rotationXYZ, translationXYZ);

		}
	}

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

	//use default values
	colorDesc.Width = pBackBufferSurfaceDesc->Width;
	colorDesc.Height = pBackBufferSurfaceDesc->Height;

	m_rTexColor = new CD3D11RenderTexture2D(colorDesc);
	V_RETURN_HR(m_rTexColor->CreateOnDevice(pDevice));

	//Texture for depth buffer
	CD3D11DepthTexture2D::TEXTURE_DESC depthDesc;
	depthDesc.Width = pBackBufferSurfaceDesc->Width;
	depthDesc.Height = pBackBufferSurfaceDesc->Height;

	m_rTexDepth = new CD3D11DepthTexture2D(depthDesc);
	V_RETURN_HR(m_rTexDepth->CreateOnDevice(pDevice));

	// configure render targets
	ID3D11RenderTargetView* pRTVs[1] = {m_rTexColor->AsRTV()};
	ID3D11DepthStencilView* pDSVs[1] = {m_rTexDepth->AsDSV()};
	m_rColorTarget = new CD3D11RenderTargetConfig(pRTVs, 1, pDSVs, 1, port);

	return S_OK;
}

void CRenderPackDemo1::OnSwapChainReleasing()
{

	m_rColorTarget = NULL;
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
	}
	if(m_TurnStatus[TURN_UP]) //i
	{
		translateOffset += float3(0.f,(0.01f*ElapsedTime),0.f);
		y0 -= .01f* ElapsedTime;
		y1 -= .01f * ElapsedTime;
	}
	if(m_TurnStatus[TURN_LEFT]) //j
	{
		translateOffset -= float3((0.01f*ElapsedTime),0.f,0.f);
		x0 += .01f* ElapsedTime;
		x1 += .01f * ElapsedTime;
	}
	if(m_TurnStatus[TURN_RIGHT]) //l
	{
		translateOffset += float3((0.01f*ElapsedTime),0.f,0.f);
		x0 -= .01f* ElapsedTime;
		x1 -= .01f * ElapsedTime;
	}
	if(m_TurnStatus[TURN_AWAY]) {//u
		translateOffset -= float3(0.f,0.f,(0.01f*ElapsedTime));
		z0 += .01f * ElapsedTime;
		z1 += .01f * ElapsedTime;
	}
	if(m_TurnStatus[TURN_CLOSE]) {//o
		if((z0 - 0.01f*ElapsedTime) > 0.0f){
			translateOffset += float3(0.f,0.f,(0.01f*ElapsedTime));
			z0 -= .01f* ElapsedTime;
			z1 -= .01f * ElapsedTime;
		}
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


void CRenderPackDemo1::FacetrackingFrustum(float monitorWidth, float monitorHeight, float kinectPosition, FLOAT translationsKinectXYZ[3]) {
	m_IsFaceTracked = true;

	float calibration = 2.75f;//slow down motions
	float y0_tmp = y0;
	float y1_tmp = y1;
	float x0_tmp = x0;
	float x1_tmp = x1;
	float z0_tmp = z0;
	if(pointToSmooth > 9) {
		smoothflag = true;
		pointToSmooth = 0;
	}
	m_smoother[pointToSmooth++] = float3(translationsKinectXYZ[0],translationsKinectXYZ[1],translationsKinectXYZ[2]);

	if (smoothflag) {
		translationsKinectXYZ[0] = 0;
		translationsKinectXYZ[1] = 0;
		translationsKinectXYZ[2] = 0;
		for(int i =0; i < 10; i++) {
			translationsKinectXYZ[0] += m_smoother[i].x;
			translationsKinectXYZ[1] += m_smoother[i].y;
			translationsKinectXYZ[2] += m_smoother[i].z;
		}
		translationsKinectXYZ[0] = translationsKinectXYZ[0]/10.0;
		translationsKinectXYZ[1] = translationsKinectXYZ[1]/10.0;
		translationsKinectXYZ[2] = translationsKinectXYZ[2]/10.0;
	}


	if(kinectPosition < 0) {

		y0 = -0.5*(- monitorHeight + monitorHeight + 2*calibration*translationsKinectXYZ[1]) ;
		y1 = -0.5*(2*calibration*translationsKinectXYZ[1] - 2*monitorHeight) ;
	} else if (kinectPosition > 0) {
		y0 = -0.5*( 2*monitorHeight + 2*calibration*translationsKinectXYZ[1]) ;
		y1 = -0.5*(2*calibration*translationsKinectXYZ[1] ) ;
	}
	x0 = -1*(monitorWidth/2 + calibration*translationsKinectXYZ[0]);
	x1 = -1*(calibration*translationsKinectXYZ[0] - monitorWidth/2);
	if(( calibration*translationsKinectXYZ[2] - 2.f) < 0.01f) {
		z0 = 0.01f;
	} else {
		z0 = calibration*translationsKinectXYZ[2] - 2.f;
	}


	m_ftfrustum = hlsl::frustum(x0,x1,y0,y1,z0,z1);
	float xOffset = x1_tmp - monitorWidth/2 - x1;
	float yOffset = y1_tmp - monitorHeight/2 -y1;
	float zOffset = z0_tmp - z0;
	m_translateOffset2 = float3(xOffset, yOffset, zOffset);
}

void CRenderPackDemo1::OnFrameRender(ID3D11Device* pDevice, ID3D11DeviceContext* pImmediateContext)
{
	CSimpleApp11::OnFrameRender(pDevice, pImmediateContext);

	// render the scene from the camera
	// update the matrices
	hlsl::float4x4 mWorld, mView, mFView, mProj, mFProj, mFWorld;
	m_rMeshResource->GetTriMesh()->GetWorldTransform(&mWorld);
	mView = *m_rCam->GetViewMatrix();
	mFView = *m_rFaceCam->GetViewMatrix();
	hlsl::float4x4 scalerF = hlsl::scale<float,4,4>(hlsl::float3(1500.0,1500.0,1500.0));
	hlsl::float4x4 scaler = hlsl::scale<float,4,4>(hlsl::float3(10.0,10.0,10.0));
	hlsl::float4x4 translateMatrix = hlsl::translation<float,4,4>(m_translateOffset2);
	hlsl::float4x4 rotateMatrixX = hlsl::rotation_x<float, 4, 4>(m_rFaceRotation.x*(-1)*((2*3.14)/360));
	hlsl::float4x4 rotateMatrixY = hlsl::rotation_y<float, 4, 4>(m_rFaceRotation.y*(1)*((2*3.14)/360));
	hlsl::float4x4 rotateMatrixZ = hlsl::rotation_z<float, 4, 4>(m_rFaceRotation.z*(-1)*((3.14)/180));

	mFView = mul(scalerF, mFView);
	mFView = mul(rotateMatrixX, mFView);
	mFView = mul(rotateMatrixY, mFView);
	mFView = mul(rotateMatrixZ, mFView);

	mProj = *m_rCam->GetProjMatrix();

	mFProj = *m_rFaceCam->GetProjMatrix();
	translateOffset = float3(0,0.0f,0);
	if(m_IsFaceTracked) {
		mProj = m_ftfrustum;
		mView = mul(translateMatrix, mView);
	}

	mView = mul(scaler, mView);

	hlsl::float4x4 mWorldViewProj = mul(mul(mWorld, mView), mProj);
	hlsl::float4x4 mWorldViewProjF = mul(mul(mWorld, mFView), mFProj);

	//update constant buffer
	m_rCBObjectTransform->Map(pImmediateContext);
	CB_PER_OBJECT* pMappedData = (CB_PER_OBJECT*)m_rCBObjectTransform->GetDataPtr();
	pMappedData->mWorldViewProj = mWorldViewProj;
	m_rCBObjectTransform->Unmap(pImmediateContext);


	m_rCBObjectFaceTransform->Map(pImmediateContext);
	CB_PER_OBJECT* pMappedDataF = (CB_PER_OBJECT*)m_rCBObjectFaceTransform->GetDataPtr();
	pMappedDataF->mWorldViewProj = mWorldViewProjF;
	m_rCBObjectFaceTransform->Unmap(pImmediateContext);

	m_rCBAUs->Map(pImmediateContext);
	CB_PER_AU* pMappedDataFaceAUs = (CB_PER_AU*)m_rCBAUs->GetDataPtr();

	for( int auidx = 0; auidx < 6; auidx++) {

		pMappedDataFaceAUs->g_pAU_Weights[auidx] = pAU_weights[auidx];

	}

	m_rCBAUs->Unmap(pImmediateContext);

	m_rContextManager->PushRenderTargets(m_rColorTarget);
	float tester = 6.5f;
	int testerI = tester;
	
	pImmediateContext->ClearRenderTargetView(m_rTexColor->AsRTV(), hlsl::float4(0.2f,0.3f, 0.2f, 1.0f));
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


	//Set Index Buffer
	pImmediateContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	pImmediateContext->IASetInputLayout(m_rMeshLayoutFace);

	//render face
	m_rContextManager->SetConfig(m_rFacePass);

	pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



	ID3D11ShaderResourceView* pFacePosSRV[2] = {m_rAnimationData->AsSRV(), m_rSDBAnimationUnits->AsSRV()};


	pImmediateContext->VSSetShaderResources(0,2, pFacePosSRV);
	pImmediateContext->DrawIndexed(numOfFaceFaces,0,1);


	m_rContextManager->PopRenderTargets();


	//render full screen quad
	m_rContextManager->SetConfig(m_rFullScreenPass);
	pImmediateContext->IASetInputLayout(m_rFullQuadLayout->GetLayout());
	m_rRenderFullQuad->Draw(pImmediateContext);



	ID3D11ShaderResourceView* pSRVs[1] = {m_rTexColor->AsSRV()};
	pImmediateContext->PSSetShaderResources(0, 1, pSRVs);

	m_rRenderFullQuad->Draw(pImmediateContext);

	pSRVs[0] = NULL;
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