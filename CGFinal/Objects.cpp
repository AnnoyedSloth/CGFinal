#include"stdafx.h"
#include"Objects.h"
#include<d3dx9math.h>


Objects::Objects() {
}

Objects::~Objects() {
}

void Objects::OnInit() {
	int i;
	RECT rect;
	D3DVIEWPORT9 vp;
	GetClientRect(m_hWnd, &rect);

	vp.X = 0;
	vp.Y = 0;
	vp.Width = rect.right - rect.left;
	vp.Height = rect.bottom - rect.top;
	vp.MinZ = 0.0f;
	vp.MaxZ = 1.0f;

	m_Eye.x = 0.0f;
	m_Eye.y = 10.0f;
	m_Eye.z = -32.0f;

	m_At.x = 0.0f;
	m_At.y = 0.0f;
	m_At.z = 0.0f;

	m_Up.x = 0.0f;
	m_Up.y = 1.0f;
	m_Up.z = 0.0f;

	D3DXMatrixLookAtLH(&m_matView, &m_Eye, &m_At, &m_Up);
	m_pd3dDevice->SetTransform(D3DTS_VIEW, &m_matView);

	D3DXMatrixPerspectiveFovLH(&m_matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f);
	m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_matProj);
	m_pd3dDevice->SetViewport(&vp);

	D3DXCreateTeapot(m_pd3dDevice, &m_PlayerMesh, NULL); // 플레이어 메쉬 생성
	D3DXCreateCylinder(m_pd3dDevice, 2.0f, 2.0f, 10.0f, 15, 10, &m_PlayerBulletMesh, NULL);
	m_Ground.Create(m_pd3dDevice, 20, 6, 2.0f);

	m_Player.iLife = 10;
	m_Player.fScale = 0.7f;
	m_Player.fRotationY = -D3DXToRadian(90);
	m_Player.vPos = D3DXVECTOR3(0.0f, 0.0f, -9 * 2.0f);
	m_Player.fVelocity = 0.005f;
	m_Player.dwBulletFireTime = 400;
	m_Player.dwOldBulletFireTime = GetTickCount();

	D3DXMatrixScaling(&m_Player.matScale, m_Player.fScale, m_Player.fScale, m_Player.fScale);
	D3DXMatrixTranslation(&m_Player.matTranslation, m_Player.vPos.x, m_Player.vPos.y, m_Player.vPos.z);
	D3DXMatrixRotationY(&m_Player.matRotationY, m_Player.fRotationY);

	m_PlayerBulletProperty.fBulletVelocity = 0.01f;
	m_PlayerBulletProperty.fScale = 0.08f;
	D3DXMatrixScaling(&m_PlayerBulletProperty.matScale, m_PlayerBulletProperty.fScale, m_PlayerBulletProperty.fScale, m_PlayerBulletProperty.fScale);
}

void Objects::OnUpdate() {
	int i, j;
	static DWORD OldTime;
	DWORD dwCurTime = GetTickCount();
	DWORD dwElapsedTime = dwCurTime - OldTime;
	OldTime = dwCurTime;

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
					m_PlayerBullet[i].vPos.z = 1.0f;
					break;
				}
			}
		}
	}

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
	m_pd3dDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);

	for (int i = 0; i < 10; i++) {
		if (m_PlayerBullet[i].iLife > 0) {
			matWorld = m_PlayerBulletProperty.matScale * m_PlayerBullet[i].matTranslation;
			m_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);
			m_PlayerBulletMesh->DrawSubset(0);
		}
	}

	m_pd3dDevice->SetTransform(D3DTS_WORLD, &m_Player.matWorld);
	m_PlayerMesh->DrawSubset(0);
}

void Objects::OnRelease() {
	m_PlayerMesh->Release();
	m_PlayerBulletMesh->Release();
	m_Ground.OnRelease();
}