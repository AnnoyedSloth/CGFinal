#include"stdafx.h"
#include"Objects.h"
#include<d3dx9math.h>

D3DVIEWPORT9 vp;

HRESULT PLAYER::PlayerInit(LPDIRECT3DDEVICE9 g_pd3dDevice, LPD3DXMESH *m_PlayerMesh)
{
	LPD3DXBUFFER pD3DXMtrlBuffer;

	iLife = 10;
	fScale = 0.7f;
	fRotationY = -D3DXToRadian(180);
	vPos = D3DXVECTOR3(0.0f, 0.0f, -9 * 2.0f);
	fVelocity = 0.005f;
	dwBulletFireTime = 400;
	dwOldBulletFireTime = GetTickCount();

	// Load the mesh from the specified file
	if (FAILED(D3DXLoadMeshFromX(L"tiger.x", D3DXMESH_SYSTEMMEM,
		g_pd3dDevice, NULL,
		&pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
		m_PlayerMesh)))
	{
		// If model is not in current folder, try parent folder
		if (FAILED(D3DXLoadMeshFromX(L"..\\tiger.x", D3DXMESH_SYSTEMMEM,
			g_pd3dDevice, NULL,
			&pD3DXMtrlBuffer, NULL, &g_dwNumMaterials,
			m_PlayerMesh)))
		{
			MessageBox(NULL, L"Could not find tiger.x", L"Meshes.exe", MB_OK);
			return E_FAIL;
		}
	}

	// We need to extract the material properties and texture names from the 
	// pD3DXMtrlBuffer
	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	g_pMeshMaterials = new D3DMATERIAL9[g_dwNumMaterials];
	if (g_pMeshMaterials == NULL)
		return E_OUTOFMEMORY;
	g_pMeshTextures = new LPDIRECT3DTEXTURE9[g_dwNumMaterials];
	if (g_pMeshTextures == NULL)
		return E_OUTOFMEMORY;

	for (DWORD i = 0; i < g_dwNumMaterials; i++)
	{
		// Copy the material
		g_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

		// Set the ambient color for the material (D3DX does not do this)
		g_pMeshMaterials[i].Ambient = g_pMeshMaterials[i].Diffuse;

		g_pMeshTextures[i] = NULL;
		if (d3dxMaterials[i].pTextureFilename != NULL &&
			lstrlenA(d3dxMaterials[i].pTextureFilename) > 0)
		{
			// Create the texture
			if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice,
				d3dxMaterials[i].pTextureFilename,
				&g_pMeshTextures[i])))
			{
				// If texture is not in current folder, try parent folder
				const CHAR* strPrefix = "..\\";
				CHAR strTexture[MAX_PATH];
				strcpy_s(strTexture, MAX_PATH, strPrefix);
				strcat_s(strTexture, MAX_PATH, d3dxMaterials[i].pTextureFilename);
				// If texture is not in current folder, try parent folder
				if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice,
					strTexture,
					&g_pMeshTextures[i])))
				{
					MessageBox(NULL, L"Could not find texture map", L"Meshes.exe", MB_OK);
				}
			}
		}
	}

	// Done with the material buffer
	pD3DXMtrlBuffer->Release();

	return S_OK;
}

Objects::Objects() {
}

Objects::~Objects() {
}

void Objects::OnInit() {
	int i;
	RECT rect;
	GetClientRect(m_hWnd, &rect);

	vp.X = 0;
	vp.Y = 0;
	vp.Width = rect.right - rect.left;
	vp.Height = rect.bottom - rect.top;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;

	m_Eye.x = m_Player.vPos.x;
	m_Eye.y = m_Player.vPos.y + 10.0f;
	m_Eye.z = m_Player.vPos.z - 32.0f;
	//m_Eye.x = 0.0f;
	//m_Eye.y = 10.0f;
	//m_Eye.z = -32.0f;

	m_At.x = 0.0f;
	m_At.y = 0.0f;
	m_At.z = 0.0f;

	m_Up.x = 0.0f;
	m_Up.y = 1.0f;
	m_Up.z = 0.0f;

	D3DXMatrixLookAtLH(&m_matView, &m_Eye, &m_At, &m_Up); // 시각 설정
	m_pd3dDevice->SetTransform(D3DTS_VIEW, &m_matView); 

	D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_matProj);
	m_pd3dDevice->SetViewport(&vp);

	m_Player.PlayerInit(m_pd3dDevice, &m_PlayerMesh);
	//D3DXCreateTeapot(m_pd3dDevice, &m_PlayerMesh, NULL); // 플레이어 메쉬 생성
	D3DXCreateCylinder(m_pd3dDevice, 2.0f, 2.0f, 10.0f, 15, 10, &m_PlayerBulletMesh, NULL); // 총알 디폴트 생성	

	m_Ground.Create(m_pd3dDevice, 40, 6, 2.0f); // 지면 생성
	m_Player.dwPlayerDiffuse = D3DCOLOR_RGBA(255, 0, 0, 255); // 플레이어 색깔(구현안됨)

	//플레이어 셋팅값
	//m_Player.iLife = 10;
	//m_Player.fScale = 0.7f;
	//m_Player.fRotationY = -D3DXToRadian(90);
	//m_Player.vPos = D3DXVECTOR3(0.0f, 0.0f, -9 * 2.0f);
	//m_Player.fVelocity = 0.005f;
	//m_Player.dwBulletFireTime = 400;
	//m_Player.dwOldBulletFireTime = GetTickCount();

	D3DXMatrixScaling(&m_Player.matScale, m_Player.fScale, m_Player.fScale, m_Player.fScale); // 플레이어 크기
	D3DXMatrixTranslation(&m_Player.matTranslation, m_Player.vPos.x, m_Player.vPos.y, m_Player.vPos.z); // 플레이어 위치
	D3DXMatrixRotationY(&m_Player.matRotationY, m_Player.fRotationY); // 플레이어 방향

	m_PlayerBulletProperty.fBulletVelocity = 0.01f; // 총알 속도
	m_PlayerBulletProperty.fScale = 0.08f; // 총알 크기 설정
	D3DXMatrixScaling(&m_PlayerBulletProperty.matScale, m_PlayerBulletProperty.fScale, m_PlayerBulletProperty.fScale, m_PlayerBulletProperty.fScale); // 총알 크기 적용
}

void Objects::OnUpdate() {
	int i, j;


	// 업데이트 시간 설정
	static DWORD OldTime;
	DWORD dwCurTime = GetTickCount();
	DWORD dwElapsedTime = dwCurTime - OldTime;
	OldTime = dwCurTime;

	//키보드 입력
	if (GetAsyncKeyState(VK_LEFT) < 0) {
		if (m_Player.vPos.x - (dwElapsedTime * m_Player.fVelocity) >= -6.0f)
			m_Player.vPos.x -= dwElapsedTime * m_Player.fVelocity;
	}

	if (GetAsyncKeyState(VK_RIGHT) < 0) {
		if (m_Player.vPos.x + (dwElapsedTime * m_Player.fVelocity) <= 6.0f)
			m_Player.vPos.x += dwElapsedTime * m_Player.fVelocity;
	}

	if (GetAsyncKeyState(VK_UP) < 0) {
		if (m_Player.vPos.z + (dwElapsedTime * m_Player.fVelocity) <= 20.0f)
			m_Player.vPos.z += dwElapsedTime * m_Player.fVelocity;
	}

	if (GetAsyncKeyState(VK_DOWN) < 0) {
		if (m_Player.vPos.z - (dwElapsedTime * m_Player.fVelocity) >= -19.0f)
			m_Player.vPos.z -= dwElapsedTime * m_Player.fVelocity;
	}

	if (GetAsyncKeyState('S') < 0) {
		if (dwCurTime - m_Player.dwOldBulletFireTime >= m_Player.dwBulletFireTime) {
			m_Player.dwOldBulletFireTime = dwCurTime;

			for ( i = 0; i < 10; i++) {
				if (m_PlayerBullet[i].iLife <= 0) {
					m_PlayerBullet[i].iLife = 1;
					m_PlayerBullet[i].vPos = m_Player.vPos;
					m_PlayerBullet[i].vPos.z += 1.0f;
					break;
				}
			}
		}
	}

	m_Eye.x = m_Player.vPos.x;
	m_Eye.y = m_Player.vPos.y + 1.0f;
	m_Eye.z = m_Player.vPos.z - 5.0f;

	m_At.x = m_Player.vPos.x / 1.5;
	m_At.y = 0.0f;
	m_At.z = m_Player.vPos.z;

	m_Up.x = 0.0f;
	m_Up.y = 1.0f;
	m_Up.z = 0.0f;

	D3DXMatrixLookAtLH(&m_matView, &m_Eye, &m_At, &m_Up); // 시각 설정
	m_pd3dDevice->SetTransform(D3DTS_VIEW, &m_matView);

	D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_matProj);
	m_pd3dDevice->SetViewport(&vp);


	for (i = 0; i < 10; i++) {
		if (m_PlayerBullet[i].iLife > 0) {
			m_PlayerBullet[i].vPos.z += dwElapsedTime * m_PlayerBulletProperty.fBulletVelocity;
			if (m_PlayerBullet[i].vPos.z >= 20.0f)
				m_PlayerBullet[i].iLife = 0;
			else
				D3DXMatrixTranslation(&m_PlayerBullet[i].matTranslation, m_PlayerBullet[i].vPos.x, m_PlayerBullet[i].vPos.y, m_PlayerBullet[i].vPos.z);
		}
	}

	D3DXMatrixTranslation(&m_Player.matTranslation, m_Player.vPos.x, m_Player.vPos.y, m_Player.vPos.z);
	m_Player.matWorld = m_Player.matScale * m_Player.matRotationY * m_Player.matTranslation;
}

void Objects::OnRender() {
	int i;
	D3DXMATRIX matWorld;
	m_Ground.OnRender();

	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);


	for (int i = 0; i < 10; i++) {
		if (m_PlayerBullet[i].iLife > 0) {
			matWorld = m_PlayerBulletProperty.matScale * m_PlayerBullet[i].matTranslation;
			m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
			m_PlayerBulletMesh->DrawSubset(0);
		}
	}

	m_pd3dDevice->SetTransform(D3DTS_WORLD, &m_Player.matWorld);
	m_PlayerMesh->DrawSubset(0);
	//m_Player.Render();
}

void Objects::OnRelease() {
	m_PlayerMesh->Release();
	m_PlayerBulletMesh->Release();
	m_Ground.OnRelease();
}

void PLAYER::Render() {
	if (SUCCEEDED(g_pd3dDevice->BeginScene()))
	{
		// Setup the world, view, and projection matrices

		// Meshes are divided into subsets, one for each material. Render them in
		// a loop
		for (DWORD i = 0; i < g_dwNumMaterials; i++)
		{
			// Set the material and texture for this subset
			g_pd3dDevice->SetMaterial(&g_pMeshMaterials[i]);
			g_pd3dDevice->SetTexture(0, g_pMeshTextures[i]);

			// Draw the mesh subset
			g_pMesh->DrawSubset(i);
		}

		// End the scene
		g_pd3dDevice->EndScene();
	}
	
}