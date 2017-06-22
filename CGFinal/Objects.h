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
	D3DXVECTOR3	vMin, vMax; // �浹��ǥ
};

struct PLAYER {
	int iLife;

	float fScale;
	float fRotationY;
	float fVelocity;

	DWORD dwBulletFireTime;
	DWORD dwOldBulletFireTime;
	DWORD dwPlayerDiffuse;

	D3DXMATRIX	matWorld;
	D3DXMATRIX	matScale;
	D3DXMATRIX	matRotationY;
	D3DXMATRIX	matTranslation;

	LPDIRECT3D9         m_pD3D = NULL; // D3D ����̽� ������ �ʿ�
	LPDIRECT3DDEVICE9   m_pd3dDevice = NULL; // ������ ����̽�

	LPD3DXMESH          m_pMesh = NULL; // �޽� ������Ʈ
	D3DMATERIAL9*       m_pMeshMaterials = NULL; // ���͸���
	LPDIRECT3DTEXTURE9* m_pMeshTextures = NULL; // �ؽ�ó
	DWORD               m_dwNumMaterials = 0L;   // �޽� ���͸��� ����

	D3DXVECTOR3	vMin, vMax;

public:
	D3DXVECTOR3 vPos;
	HRESULT PlayerInit(LPDIRECT3DDEVICE9 g_pd3dDevice, LPD3DXMESH *m_PlayerMesh);
	void Render();
};

struct ENEMY {
	int iLife;
	DWORD dwAppearTime;
	D3DXVECTOR3	vPos;

	DWORD dwOldFireTime;
	D3DXMATRIX matTranslation;
};

struct ENEMY_PROPERTY {
	float fMoveVelocity;
	float fScale;

	DWORD dwFireTime;
	D3DXMATRIX matScale;
	D3DXVECTOR3 vMin, vMax;
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

	LPD3DXMESH m_PlayerMesh; // ���ΰ��޽�
	LPD3DXMESH m_PlayerBulletMesh; // �Ѿ˸޽�
	LPD3DXMESH m_EnemyBoxMesh;
	LPD3DXMESH m_EnemyBulletMesh;

	PLAYER m_Player;
	BULLET_PROPERTY m_PlayerBulletProperty;
	BULLET m_PlayerBullet[20];


	ENEMY_PROPERTY m_EnemyProperty;
	ENEMY m_Enemy[100];
	BULLET_PROPERTY m_EnemyBulletProperty;
	BULLET m_EnemyBullet[100];
	int m_iEnemyIndex;

	DWORD m_dwElapsedTime;

	DWORD m_dwGameStartTime;
	DWORD m_dwGameElapsedTime;

	int m_iEnemyCount;

public:

	BOOL CheckCubeIntersection(D3DXVECTOR3* vMin1, D3DXVECTOR3* vMax1, D3DXVECTOR3* vMin2, D3DXVECTOR3* vMax2);

	Objects();
	~Objects();
};