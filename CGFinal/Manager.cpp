#include "stdafx.h"
#include "Manager.h"

Manager::Manager() {
}

Manager::~Manager() {
}

HRESULT Manager::InitD3D(HWND hWnd) {

	m_hWnd = hWnd;

	if (NULL == (m_pD3D = Direct3DCreate9(D3D_SDK_VERSION)))
		return E_FAIL;

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;

	if (FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &m_pd3dDevice))) {
		return E_FAIL;
	}

	m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

	OnInit();

	return S_OK;
}

void Manager::Render() {
	if (NULL == m_pd3dDevice) return;

	m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	if (SUCCEEDED(m_pd3dDevice->BeginScene())) {
		OnRender();

		m_pd3dDevice->EndScene();
	}

	m_pd3dDevice->Present(NULL, NULL, NULL, NULL);
}

void Manager::Update() {
	OnUpdate();
}

void Manager::Cleanup() {
	OnRelease();

	if (m_pd3dDevice != NULL) m_pd3dDevice->Release();

	if (m_pD3D != NULL) m_pD3D->Release();
}