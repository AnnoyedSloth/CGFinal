#pragma once
#include"Manager.h"
#include<d3dx9math.h>
#include<d3dx9shape.h>
#include"Ground.h"


struct BULLET {
	int iLife;
	D3DXVECTOR3	vPos;
	D3DXMATRIX matTranslation;
};

struct BULLET_PROPERTY {
	float fBulletVelocity;
	float fScale;
	D3DXMATRIX matScale;
};

struct PLAYER {
	int iLife;
	D3DXVECTOR3 vPos;
	float fScale;
	float fRotationY;
	float fVelocity;

	DWORD dwBulletFireTime;
	DWORD dwOldBulletFireTime;

	D3DXMATRIX	matWorld;
	D3DXMATRIX	matScale;
	D3DXMATRIX	matRotationY;
	D3DXMATRIX	matTranslation;
};

class Objects : public Manager {
	virtual void OnInit();
	virtual void OnRender();
	virtual void OnUpdate();
	virtual void OnRelease();

	D3DXMATRIX m_matView;
	D3DXMATRIX m_matProj;
	D3DXVECTOR3 m_Eye, m_At, m_Up;

	CGround m_Ground;

	LPD3DXMESH m_PlayerMesh; // 주인공메쉬
	LPD3DXMESH m_PlayerBulletMesh; // 총알메쉬

	PLAYER m_Player;
	BULLET_PROPERTY m_PlayerBulletProperty;
	BULLET m_PlayerBullet[10];

public:
	Objects();
	~Objects();
};