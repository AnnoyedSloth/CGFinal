#include"stdafx.h"
#include"Objects.h"
#include<d3dx9math.h>

const int SIZE_OF_ROW = 40;

D3DVIEWPORT9 vp;

Objects::Objects() {
}

Objects::~Objects() {
}

HRESULT PLAYER::PlayerInit(LPDIRECT3DDEVICE9 g_pd3dDevice, LPD3DXMESH *m_PlayerMesh)
{
	LPD3DXBUFFER pD3DXMtrlBuffer;

	iLife = 10;
	fScale = 0.3f;
	fRotationY = -D3DXToRadian(180);
	vPos = D3DXVECTOR3(0.0f, 1.0f, -9 * 2.0f);
	fVelocity = 0.005f;
	dwBulletFireTime = 400;
	dwOldBulletFireTime = GetTickCount();

	// Load the mesh from the specified file
	if (FAILED(D3DXLoadMeshFromX(L"tiger.x", D3DXMESH_SYSTEMMEM,
		g_pd3dDevice, NULL,
		&pD3DXMtrlBuffer, NULL, &m_dwNumMaterials,
		m_PlayerMesh)))
	{
		// If model is not in current folder, try parent folder
		if (FAILED(D3DXLoadMeshFromX(L"..\\tiger.x", D3DXMESH_SYSTEMMEM,
			g_pd3dDevice, NULL,
			&pD3DXMtrlBuffer, NULL, &m_dwNumMaterials,
			m_PlayerMesh)))
		{
			MessageBox(NULL, L"Could not find tiger.x", L"Meshes.exe", MB_OK);
			return E_FAIL;
		}
	}

	// We need to extract the material properties and texture names from the 
	// pD3DXMtrlBuffer
	D3DXMATERIAL* d3dxMaterials = (D3DXMATERIAL*)pD3DXMtrlBuffer->GetBufferPointer();
	m_pMeshMaterials = new D3DMATERIAL9[m_dwNumMaterials];
	if (m_pMeshMaterials == NULL)
		return E_OUTOFMEMORY;
	m_pMeshTextures = new LPDIRECT3DTEXTURE9[m_dwNumMaterials];
	if (m_pMeshTextures == NULL)
		return E_OUTOFMEMORY;

	for (DWORD i = 0; i < m_dwNumMaterials; i++)
	{
		// Copy the material
		m_pMeshMaterials[i] = d3dxMaterials[i].MatD3D;

		// Set the ambient color for the material (D3DX does not do this)
		m_pMeshMaterials[i].Ambient = m_pMeshMaterials[i].Diffuse;

		m_pMeshTextures[i] = NULL;
		if (d3dxMaterials[i].pTextureFilename != NULL &&
			lstrlenA(d3dxMaterials[i].pTextureFilename) > 0)
		{
			// Create the texture
			if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice,
				d3dxMaterials[i].pTextureFilename,
				&m_pMeshTextures[i])))
			{
				// If texture is not in current folder, try parent folder
				const CHAR* strPrefix = "..\\";
				CHAR strTexture[MAX_PATH];
				strcpy_s(strTexture, MAX_PATH, strPrefix);
				strcat_s(strTexture, MAX_PATH, d3dxMaterials[i].pTextureFilename);
				// If texture is not in current folder, try parent folder
				if (FAILED(D3DXCreateTextureFromFileA(g_pd3dDevice,
					strTexture,
					&m_pMeshTextures[i])))
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

	m_Player.PlayerInit(m_pd3dDevice, &m_PlayerMesh); // 플레이어 생성
	D3DXCreateCylinder(m_pd3dDevice, 3.0f, 2.0f, 10.0f, 15, 10, &m_PlayerBulletMesh, NULL); // 총알 디폴트 생성
	D3DXCreateBox(m_pd3dDevice, 1.0f, 1.0f, 1.0f, &m_EnemyBoxMesh, NULL); // 적 박스메쉬
	D3DXCreateSphere(m_pd3dDevice, 0.2f, 20, 10, &m_EnemyBulletMesh, NULL); // 적 총알메쉬
	m_Ground.Create(m_pd3dDevice, SIZE_OF_ROW, 6, 2.0f); // 지면 생성
	m_Player.dwPlayerDiffuse = D3DCOLOR_RGBA(255, 0, 0, 255); // 플레이어 색깔(구현안됨)

	for (i = 0; i < 10; i++) {
		m_PlayerBullet[i].iLife = 0;
	}

	D3DXMatrixScaling(&m_Player.matScale, m_Player.fScale, m_Player.fScale, m_Player.fScale); // 플레이어 크기
	D3DXMatrixTranslation(&m_Player.matTranslation, m_Player.vPos.x, m_Player.vPos.y, m_Player.vPos.z); // 플레이어 위치
	D3DXMatrixRotationY(&m_Player.matRotationY, m_Player.fRotationY); // 플레이어 방향


	m_PlayerBulletProperty.fBulletVelocity = 0.01f; // 총알 속도
	m_PlayerBulletProperty.fScale = 0.08f; // 총알 크기 설정
	D3DXMatrixScaling(&m_PlayerBulletProperty.matScale, m_PlayerBulletProperty.fScale, m_PlayerBulletProperty.fScale, m_PlayerBulletProperty.fScale); // 총알 크기 적용

	m_EnemyProperty.fScale = 2.0f;
	m_EnemyProperty.fMoveVelocity = 0.003f;
	m_EnemyProperty.dwFireTime = 1200;
	D3DXMatrixScaling(&m_EnemyProperty.matScale, m_EnemyProperty.fScale, m_EnemyProperty.fScale, m_EnemyProperty.fScale);

	int nRandomNum;

	for (int i = 0; i < 100; i++) {
		m_Enemy[i].iLife = 0;
		m_Enemy[i].dwAppearTime = 2000 + (i * 800);
		m_Enemy[i].dwOldFireTime = GetTickCount();
		nRandomNum = rand() % 7;
		if (nRandomNum % 2)
			m_Enemy[i].vPos.x = -1.0f * nRandomNum;
		else
			m_Enemy[i].vPos.x = (float)nRandomNum;

		m_Enemy[i].vPos.y = 1.0f;
		m_Enemy[i].vPos.z = 40.0f;

		D3DXMatrixTranslation(&m_Enemy[i].matTranslation, m_Enemy[i].vPos.x, m_Enemy[i].vPos.y+1.0f, m_Enemy[i].vPos.z);
	}

	m_EnemyBulletProperty.fBulletVelocity = 0.01f;
	m_EnemyBulletProperty.fScale = 0.9f;
	D3DXMatrixScaling(&m_EnemyBulletProperty.matScale, m_EnemyBulletProperty.fScale, m_EnemyBulletProperty.fScale, m_EnemyBulletProperty.fScale);

	for (i = 0; i < 100; i++)
		m_EnemyBullet[i].iLife = 0;

	m_Player.iLife = 3;
	m_dwGameStartTime = GetTickCount();

	D3DXVECTOR3 *pVertices;
	m_PlayerMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVertices);
	D3DXComputeBoundingBox(pVertices, m_PlayerMesh->GetNumVertices(), D3DXGetFVFVertexSize(m_PlayerMesh->GetFVF()), &m_Player.vMin, &m_Player.vMax);
	m_PlayerMesh->UnlockVertexBuffer();

	D3DXVECTOR3 vMin1, vMax1;
	float fTemp;
	D3DXVec3TransformCoord(&vMin1, &m_Player.vMin, &m_Player.matRotationY);
	D3DXVec3TransformCoord(&vMax1, &m_Player.vMax, &m_Player.matRotationY);

	if (vMin1.x, vMax1.x) {
		fTemp = vMin1.x;
		vMin1.x = vMax1.x;
		vMax1.x = fTemp;
	}

	if (vMin1.y > vMax1.y) {
		fTemp = vMin1.y;
		vMin1.y = vMax1.y;
		vMax1.y = fTemp;
	}

	if (vMin1.z > vMax1.z) {
		fTemp = vMin1.z;
		vMin1.z = vMax1.z;
		vMax1.z = fTemp;
	}

	m_Player.vMin = vMin1;
	m_Player.vMax = vMax1;

	// 주인공 미사일 충돌 정점
	m_PlayerBulletMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVertices);
	D3DXComputeBoundingBox(pVertices, m_PlayerBulletMesh->GetNumVertices(), D3DXGetFVFVertexSize(m_PlayerBulletMesh->GetFVF()), &m_PlayerBulletProperty.vMin, &m_PlayerBulletProperty.vMax);
	m_PlayerBulletMesh->UnlockVertexBuffer();

	m_EnemyBoxMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVertices);
	D3DXComputeBoundingBox(pVertices, m_EnemyBoxMesh->GetNumVertices(), 
							D3DXGetFVFVertexSize(m_EnemyBoxMesh->GetFVF()), &m_EnemyProperty.vMin, &m_EnemyProperty.vMax);
	m_EnemyBoxMesh->UnlockVertexBuffer();

	m_EnemyBulletMesh->LockVertexBuffer(D3DLOCK_READONLY, (void**)&pVertices);
	D3DXComputeBoundingBox(pVertices, m_EnemyBulletMesh->GetNumVertices(), D3DXGetFVFVertexSize(m_EnemyBulletMesh->GetFVF()), &m_EnemyBulletProperty.vMin, &m_EnemyBulletProperty.vMax);
	m_EnemyBulletMesh->UnlockVertexBuffer();
}

void Objects::OnUpdate() {
	int i, j;


	// 업데이트 시간 설정
	static DWORD OldTime;
	DWORD dwCurTime = GetTickCount();
	DWORD m_dwElapsedTime = dwCurTime - OldTime;
	OldTime = dwCurTime;
	D3DXMATRIX matWorld;

	m_dwGameElapsedTime = dwCurTime - m_dwGameStartTime;

	//키보드 입력
	if (GetAsyncKeyState(VK_LEFT) < 0) {
		if (m_Player.vPos.x - (m_dwElapsedTime * m_Player.fVelocity) >= -6.0f)
			m_Player.vPos.x -= m_dwElapsedTime * m_Player.fVelocity;
	}

	if (GetAsyncKeyState(VK_RIGHT) < 0) {
		if (m_Player.vPos.x + (m_dwElapsedTime * m_Player.fVelocity) <= 6.0f)
			m_Player.vPos.x += m_dwElapsedTime * m_Player.fVelocity;
	}

	if (GetAsyncKeyState(VK_UP) < 0) {
		if (m_Player.vPos.z + (m_dwElapsedTime * m_Player.fVelocity) <= SIZE_OF_ROW)
			m_Player.vPos.z += m_dwElapsedTime * m_Player.fVelocity;
	}

	if (GetAsyncKeyState(VK_DOWN) < 0) {
		if (m_Player.vPos.z - (m_dwElapsedTime * m_Player.fVelocity) >= -19.0f)
			m_Player.vPos.z -= m_dwElapsedTime * m_Player.fVelocity;
	}

	if (GetAsyncKeyState(VK_SPACE) < 0) {
		if (dwCurTime - m_Player.dwOldBulletFireTime >= m_Player.dwBulletFireTime) {
			m_Player.dwOldBulletFireTime = dwCurTime;

			for (i = 0; i < 10; i++) {
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
	m_Eye.z = m_Player.vPos.z - 3.0f;

	m_At.x = m_Player.vPos.x / 1.2;
	m_At.y = 1.0f;
	m_At.z = m_Player.vPos.z;

	m_Up.x = 0.0f;
	m_Up.y = 1.0f;
	m_Up.z = 0.0f;

	D3DXMatrixLookAtLH(&m_matView, &m_Eye, &m_At, &m_Up); // 시각 설정
	m_pd3dDevice->SetTransform(D3DTS_VIEW, &m_matView);

	D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_matProj);
	m_pd3dDevice->SetViewport(&vp);


	for (i = m_iEnemyIndex; i < 100; i++) {
		if (m_Enemy[i].dwAppearTime <= m_dwGameElapsedTime) {
			m_Enemy[i].iLife = 1;
			m_iEnemyIndex++;
		}
		else {
			break;
		}
	}

	for (i = 0; i < m_iEnemyIndex; i++) {
		if (m_Enemy[i].iLife) {
			m_Enemy[i].vPos.z -= m_dwElapsedTime * m_EnemyProperty.fMoveVelocity;
			if (m_Enemy[i].vPos.z <= -20.0f) {
				m_Enemy[i].iLife = 0;
				m_iEnemyCount--;
				continue;
			}

			D3DXMatrixTranslation(&m_Enemy[i].matTranslation, m_Enemy[i].vPos.x, m_Enemy[i].vPos.y, m_Enemy[i].vPos.z);

			if (dwCurTime - m_Enemy[i].dwOldFireTime >= m_EnemyProperty.dwFireTime) {
				m_Enemy[i].dwOldFireTime = dwCurTime;
				for (j = 0; j < 100; j++) {
					if (!m_EnemyBullet[j].iLife) {
						m_EnemyBullet[j].iLife = 1;
						m_EnemyBullet[j].vPos = m_Enemy[i].vPos;
						m_EnemyBullet[j].vPos.z -= 0.5f;
						break;
					}
				}
			}
		}
	}

	for (i = 0; i < 100; i++) {
		if (m_EnemyBullet[i].iLife > 0) {
			m_EnemyBullet[i].vPos.z -= m_dwElapsedTime * m_EnemyBulletProperty.fBulletVelocity;
			if (m_EnemyBullet[i].vPos.z <= -20.0f)
				m_EnemyBullet[i].iLife = 0;
			else
				D3DXMatrixTranslation(&m_EnemyBullet[i].matTranslation, m_EnemyBullet[i].vPos.x, m_EnemyBullet[i].vPos.y, m_EnemyBullet[i].vPos.z);
		}
	}


	for (i = 0; i < 20; i++) {
		if (m_PlayerBullet[i].iLife > 0) {
			m_PlayerBullet[i].vPos.z += m_dwElapsedTime * m_PlayerBulletProperty.fBulletVelocity;
			if (m_PlayerBullet[i].vPos.z >= SIZE_OF_ROW)
				m_PlayerBullet[i].iLife = 0;
			else
				D3DXMatrixTranslation(&m_PlayerBullet[i].matTranslation, m_PlayerBullet[i].vPos.x, m_PlayerBullet[i].vPos.y, m_PlayerBullet[i].vPos.z);
		}
	}

	D3DXMatrixTranslation(&m_Player.matTranslation, m_Player.vPos.x, m_Player.vPos.y, m_Player.vPos.z);
	m_Player.matWorld = m_Player.matScale * m_Player.matRotationY * m_Player.matTranslation;


	D3DXVECTOR3 vMin1, vMax1, vMin2, vMax2;

	D3DXVec3TransformCoord(&vMin1, &m_Player.vMin, &m_Player.matTranslation);
	D3DXVec3TransformCoord(&vMax1, &m_Player.vMax, &m_Player.matTranslation);

	for (i = 0; i < m_iEnemyIndex; i++) {
		if (m_Enemy[i].iLife > 0) {
			D3DXMatrixTranslation(&m_Enemy[i].matTranslation, m_Enemy[i].vPos.x, m_Enemy[i].vPos.y, m_Enemy[i].vPos.z);
			matWorld = m_EnemyProperty.matScale * m_Enemy[i].matTranslation;

			D3DXVec3TransformCoord(&vMin2, &m_EnemyProperty.vMin, &matWorld);
			D3DXVec3TransformCoord(&vMax2, &m_EnemyProperty.vMax, &matWorld);

			if (CheckCubeIntersection(&vMin1, &vMax1, &vMin2, &vMax2)) {
				m_Enemy[i].iLife = 0;
				m_Player.iLife--;
			}
		}
	}

	for (i = 0; i < 100; i++) {
		if (m_EnemyBullet[i].iLife > 0) {
			D3DXMatrixTranslation(&m_EnemyBullet[i].matTranslation, m_EnemyBullet[i].vPos.x, m_EnemyBullet[i].vPos.y, m_EnemyBullet[i].vPos.z);
			matWorld = m_EnemyBulletProperty.matScale * m_EnemyBullet[i].matTranslation;
			D3DXVec3TransformCoord(&vMin2, &m_EnemyBulletProperty.vMin, &matWorld);
			D3DXVec3TransformCoord(&vMax2, &m_EnemyBulletProperty.vMax, &matWorld);

			if (CheckCubeIntersection(&vMin1, &vMax1, &vMin2, &vMax2)) {
				m_EnemyBullet[i].iLife = 0;
				m_Player.iLife;
			}
		}
	}

	// Note: 주인공 미사일과 적 캐릭터 충돌
	for (i = 0; i < 20; i++)
	{
		if (m_PlayerBullet[i].iLife > 0) // 살아있는 주인공 미사일만
		{
			D3DXMatrixTranslation(&m_PlayerBullet[i].matTranslation,
				m_PlayerBullet[i].vPos.x, m_PlayerBullet[i].vPos.y, m_PlayerBullet[i].vPos.z);
			matWorld = m_PlayerBulletProperty.matScale * m_PlayerBullet[i].matTranslation;
			D3DXVec3TransformCoord(&vMin1, &m_PlayerBulletProperty.vMin, &matWorld);
			D3DXVec3TransformCoord(&vMax1, &m_PlayerBulletProperty.vMax, &matWorld);

			// Note: 적 캐릭터와 충돌 체크
			for (j = 0; j < m_iEnemyIndex; j++)
			{
				if (m_Enemy[j].iLife > 0) // 살아있는 적 캐릭터만
				{
					D3DXMatrixTranslation(&m_Enemy[j].matTranslation, m_Enemy[j].vPos.x,
						m_Enemy[j].vPos.y, m_Enemy[j].vPos.z);
					matWorld = m_EnemyProperty.matScale * m_Enemy[j].matTranslation;
					D3DXVec3TransformCoord(&vMin2, &m_EnemyProperty.vMin, &matWorld);
					D3DXVec3TransformCoord(&vMax2, &m_EnemyProperty.vMax, &matWorld);

					if (CheckCubeIntersection(&vMin1, &vMax1, &vMin2, &vMax2))
					{
						m_Enemy[j].iLife = 0;
						m_PlayerBullet[i].iLife = 0;
						break;
					}
				}
			}

			// Note: 주인공 미사일과 적 미사일 충돌 체크
			if (m_PlayerBullet[i].iLife > 0)
			{
				for (j = 0; j < 100; j++)
				{
					if (m_EnemyBullet[j].iLife > 0)
					{
						D3DXMatrixTranslation(&m_EnemyBullet[j].matTranslation,
							m_EnemyBullet[j].vPos.x, m_EnemyBullet[j].vPos.y, m_EnemyBullet[j].vPos.z);
						matWorld = m_EnemyBulletProperty.matScale *
							m_EnemyBullet[j].matTranslation;
						D3DXVec3TransformCoord(&vMin2, &m_EnemyBulletProperty.vMin,
							&matWorld);
						D3DXVec3TransformCoord(&vMax2, &m_EnemyBulletProperty.vMax,
							&matWorld);
						if (CheckCubeIntersection(&vMin1, &vMax1, &vMin2, &vMax2))
						{
							m_EnemyBullet[j].iLife = 0;
							m_PlayerBullet[i].iLife = 0;
							break;
						}
					}
				}
			}
		}
	}
	D3DXMatrixTranslation(&m_Player.matTranslation, m_Player.vPos.x,
		m_Player.vPos.y, m_Player.vPos.z);
	m_Player.matWorld = m_Player.matScale * m_Player.matRotationY *
		m_Player.matTranslation;

}

void Objects::OnRender() {
	int i;
	D3DXMATRIX matWorld;
	m_Ground.OnRender();

	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);


	for (int i = 0; i < 10; i++) {
		if (m_PlayerBullet[i].iLife > 0) {
			matWorld = m_PlayerBulletProperty.matScale * m_PlayerBullet[i].matTranslation;
			m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
			m_PlayerBulletMesh->DrawSubset(0);
		}
	}

	for (i = 0; i < 100; i++) {
		if (m_EnemyBullet[i].iLife > 0) {
			matWorld = m_EnemyBulletProperty.matScale * m_EnemyBullet[i].matTranslation;
			m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
			m_EnemyBulletMesh->DrawSubset(0);
		}
	}

	for (i = 0; i < m_iEnemyIndex; i++) {
		if (m_Enemy[i].iLife) {
			matWorld = m_EnemyProperty.matScale * m_Enemy[i].matTranslation;
			m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
			m_EnemyBoxMesh->DrawSubset(0);
		}
	}

	if (SUCCEEDED(m_pd3dDevice->BeginScene()))
	{
		// Setup the world, view, and projection matrices

		// Meshes are divided into subsets, one for each material. Render them in
		// a loop
		for (DWORD i = 0; i < m_Player.m_dwNumMaterials; i++)
		{
			// Set the material and texture for this subset
			m_pd3dDevice->SetMaterial(&m_Player.m_pMeshMaterials[i]);
			m_pd3dDevice->SetTexture(0, m_Player.m_pMeshTextures[i]);

			// Draw the mesh subset
			m_Player.m_pMesh->DrawSubset(i);
		}

		// End the scene
		m_pd3dDevice->EndScene();
	}

	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	m_pd3dDevice->SetTransform(D3DTS_WORLD, &m_Player.matWorld);
	m_PlayerMesh->DrawSubset(0);
	m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	//m_Player.Render();
}

void Objects::OnRelease() {
	m_EnemyBoxMesh->Release();
	m_EnemyBulletMesh->Release();
	m_PlayerMesh->Release();
	m_PlayerBulletMesh->Release();
	m_Ground.OnRelease();
}

void PLAYER::Render() {
	if (SUCCEEDED(m_pd3dDevice->BeginScene()))
	{
		// Setup the world, view, and projection matrices

		// Meshes are divided into subsets, one for each material. Render them in
		// a loop
		for (DWORD i = 0; i <m_dwNumMaterials; i++)
		{
			// Set the material and texture for this subset
			m_pd3dDevice->SetMaterial(&m_pMeshMaterials[i]);
			m_pd3dDevice->SetTexture(0, m_pMeshTextures[i]);
			// Draw the mesh subset
			m_pMesh->DrawSubset(i);
		}

		// End the scene
		m_pd3dDevice->EndScene();
	}	
}

BOOL Objects::CheckCubeIntersection(D3DXVECTOR3* vMin1, D3DXVECTOR3* vMax1, D3DXVECTOR3* vMin2, D3DXVECTOR3* vMax2) {

	if (vMin1->x <= vMax2->x && vMax1->x >= vMin2->x &&
		vMin1->y <= vMax2->y && vMax1->y >= vMin2->y &&
		vMin1->z <= vMax2->z && vMax1->z >= vMin2->z)
		return TRUE;
	else return FALSE;
}