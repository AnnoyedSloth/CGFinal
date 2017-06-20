#include<d3dx9math.h>
#include"Objects.h"

Objects::Objects() {
}

Objects::~Objects() {
}

void Objects::OnInit() {
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

}