#include "pch.h"
#include "Player.h"
#include "GameObject.h"
#include "Component.h"
#include "Collider.h"
#include "BoxCollider.h"
#include "Rigidbody.h"
#include "ChangeObject.h"
#include "AttakObject.h"
#include "AttackEvent.h"
#include "DebugWindow.h"
#include "SceneChanger.h"
#include "StageMaker.h"
#include "ColorButton.h"

float Lerp(float start, float end, float t)
{
	return start + ((end - start) * t);
}

void Player::UpdateAnim(bool isOneTime)
{
	m_ar->SetOneTime(isOneTime);
	if (m_arrAnim[(int)m_mode][(int)m_arrow][(int)m_state].identity != m_ar->GetCurrentAnim().identity)
	{
		m_ar->ChangeAnim(m_arrAnim[(int)m_mode][(int)m_arrow][(int)m_state]);
	}
}

void Player::FlyAction()
{
	if (!m_rig->GetIsOnLand())
	{
		if (GetAsyncKeyState(m_atkKey)) //��������
		{
			m_state = PlayerAState::idle;
			m_rig->Velocity().y = 0.0f;
			UpdateAnim(false);
			return;
		}
		if (GetAsyncKeyState(m_jumpKey)) //���� ����
		{
			m_flyTimer.tick();
			if (m_flyTimer.getTotalDeltaTime() > m_dblTime * 2)
			{
				m_rig->Velocity().y = -m_speed_fly;
				m_flyTimer.resetTotalDeltaTime();
			}
		}
		if (GetAsyncKeyState(m_rightKey))//���� fly �̵�
		{
			m_rig->Velocity() = { m_speed / 2, m_rig->Velocity().y };
			m_arrow = Arrow::right;
		}
		else if (GetAsyncKeyState(m_leftKey))//���� fly �̵�
		{
			m_rig->Velocity() = { -m_speed / 2, m_rig->Velocity().y };
			m_arrow = Arrow::left;
		}
		else //fly idle
		{
			m_rig->Velocity() = { 0.0f, m_rig->Velocity().y };
		}
	}
	else
	{
		m_state = PlayerAState::idle;
		m_rig->Velocity() = { 0.0f, 0.0f };
	}
	UpdateAnim(false);
}

void Player::MoveLeft()
{
	if (m_rig->GetIsOnLand())
	{
		m_arrow = Arrow::left;
		UpdateAnim(false);
		if ((m_leftKeyTimer.getDeltaTime() <= m_dblTime &&
			(m_state == PlayerAState::walk ||
				m_state == PlayerAState::jump ||
				m_state == PlayerAState::idle)) ||
			m_state == PlayerAState::run)  //Run_Left
		{
			m_curXSpeed = -m_speed_run;
			m_state = PlayerAState::run;
		}
		else if (m_state == PlayerAState::eat_idle ||
			m_state == PlayerAState::eat_jump ||
			m_state == PlayerAState::eat_move)//eat_move_left
		{
			m_curXSpeed = -m_eatSpeed;
			m_state = PlayerAState::eat_move;
		}
		else//Walk_Left
		{
			m_curXSpeed = -m_speed;
			m_state = PlayerAState::walk;
		}
		UpdateAnim(false);
		m_rig->Velocity() = { m_curXSpeed, m_rig->Velocity().y };
	}
}

void Player::MoveRight()
{
	if (m_rig->GetIsOnLand())
	{
		m_arrow = Arrow::right;
		UpdateAnim(false);
		if ((m_rightKeyTimer.getDeltaTime() <= m_dblTime &&
				(m_state == PlayerAState::walk || 
				m_state == PlayerAState::jump || 
				m_state == PlayerAState::idle )) ||
			m_state == PlayerAState::run)  //Run_Right
		{
			m_curXSpeed = m_speed_run;
			m_state = PlayerAState::run;
		}
		else if (m_state == PlayerAState::eat_idle ||
			m_state == PlayerAState::eat_jump ||
			m_state == PlayerAState::eat_move)//eat_move_right
		{
			m_curXSpeed = m_eatSpeed;
			m_state = PlayerAState::eat_move;
		}
		else //Walk_Right
		{
			m_curXSpeed = m_speed;
			m_state = PlayerAState::walk;
		}
		UpdateAnim(false);
		m_rig->Velocity() = { m_curXSpeed, m_rig->Velocity().y };
	}
}

void Player::JumpAction()
{
	//����
	m_flyTimer.tick();
	if (m_rig->GetIsOnLand())
	{
		if (m_state == PlayerAState::eat_idle ||
			m_state == PlayerAState::eat_move ||
			m_state == PlayerAState::eat_jump)
		{
			m_rig->Velocity() = { m_curXSpeed, -m_JumpV * 0.5f  };
			m_state = PlayerAState::eat_jump;
		}
		else
		{
			m_rig->Velocity() = { m_curXSpeed, -m_JumpV };
			m_state = PlayerAState::jump;
		}
		cout << "Jump" << endl;
	}
	else if (m_flyTimer.getDeltaTime() > m_dblTime && 
		(m_state != PlayerAState::eat_idle && 
			m_state != PlayerAState::eat_jump && 
			m_state != PlayerAState::eat_move && 
			!m_rig->GetIsOnLand()))//���� start
	{
		m_state = PlayerAState::fly;
		m_rig->Velocity().y = -m_speed_fly;
		m_rig->SetGravity(m_flyGravity);
		cout << "Fly" << endl;
	}
	UpdateAnim(true);
}

void Player::Idle()
{
	if (m_state == PlayerAState::eat_idle ||
		m_state == PlayerAState::eat_move ||
		m_state == PlayerAState::eat_jump)
	{
		m_state = PlayerAState::eat_idle;
	}
	else
	{
		m_state = PlayerAState::idle;
	}
	m_rig->Velocity() = { 0.0f, 0.0f };
	m_curXSpeed = 0.0f;
	UpdateAnim(false);
}

void Player::Attack_default()
{
	if (m_rig->GetIsOnLand() &&
		m_state != PlayerAState::eat_idle &&
		m_state != PlayerAState::eat_move) //���(�⺻����)
	{
		m_rig->Velocity() = { 0.0f, 0.0f };
		m_state = PlayerAState::eating;
		UpdateAnim(false);
		m_atkTrigger = false;
	}
	else if (m_state == PlayerAState::eat_idle ||
		m_state == PlayerAState::eat_move) //���
	{
		if (m_atkTrigger)
		{
			GameObject* obj = new GameObject();
			AttakObject* ao = new AttakObject();
			if (m_arrow == Arrow::left)
			{
				ao->SetSpeed(-5.0f);
				obj->SetPosition({m_dOffset.x + m_gameObj->Position().x - m_cSize.x, 
					m_dOffset.y + m_gameObj->Position().y + m_cSize.y / 2 - 20});
			}
			else
			{
				ao->SetSpeed(5.0f);
				obj->SetPosition({ m_dOffset.x+ m_gameObj->Position().x + m_cSize.x, 
					m_dOffset.y + m_gameObj->Position().y + m_cSize.y / 2 - 20});
			}
			obj->AddComponent(ao);
			obj->InitializeSet();
			obj->SetOrderInLayer(m_gameObj->GetOrderInLayer() + 1);
			m_state = PlayerAState::attack;
			UpdateAnim(true);
			m_atkTrigger = false;
			return;
		}
	}
}

void Player::Attack_sword()
{
	if (m_state != PlayerAState::fly && 
		!(m_state == PlayerAState::idle && !m_rig->GetIsOnLand()))
	{
		m_state = PlayerAState::attack;
		UpdateAnim(true);
		m_atkTrigger = false;
		GameObject* atk = new GameObject();
		atk->Size() = { m_atkRange,m_cSize.y };
		atk->SetOrderInLayer(10);
		if (m_arrow == Arrow::left)
		{
			atk->SetPosition({ m_dOffset.x + m_gameObj->Position().x - m_atkRange - 5, m_dOffset.y + m_gameObj->Position().y });
		}
		else
		{
			atk->SetPosition({ m_dOffset.x + m_gameObj->Position().x + m_cSize.x + 5, m_dOffset.y + m_gameObj->Position().y });
		}
		BoxCollider* newbo = new BoxCollider();
		newbo->SetTrigger(true);
		atk->AddComponent(newbo);
		atk->AddComponent(new AttackEvent());
		atk->InitializeSet();
	}
	return;
}

void Player::CollisionEnter(Collider* other)
{
	cout << "PLAYER Col Enter" << endl;
}

void Player::Attack_stone()
{
	if (m_atkTrigger)
	{
		isStoneAtked = false;
		m_state = PlayerAState::attack;
		UpdateAnim(true);
		m_atkTrigger = false;
		m_rig->Velocity() = { 0.0f, m_rig->Velocity().y };
	}
}

void Player::CollisionExit(Collider* other)
{
	cout << "PLAYER Col Exit" << endl;
}

void Player::StoneAttacking()
{
	if (GetAsyncKeyState(m_atkKey))
	{
		if (m_atkTrigger)
		{
			Idle();
			m_rig->SetGravity(m_startGravity);
			m_atkTrigger = false;
		}
	}
	else
		m_atkTrigger = true;
}

void Player::Collision(Collider* other)
{
	if (other == nullptr)
		return;
	if (other->GetGameObject()->GetTag() == TAG_DOOR)
	{
		if (GetAsyncKeyState(m_upKey) & 0x8000) //���� ��������
		{
			cout << "NEXT STAGE" << endl;
			string stageName = StageMaker::GetInstance()->GetMapName();

			SceneChanger::Destroy();
			StageMaker::Destroy();
			if (DEBUGMODE)
				DebugWindow::Destroy();

			ObjectManager::GetInstance()->Clear();

			SceneChanger::Create();
			StageMaker::Create();
			if (DEBUGMODE)
				DebugWindow::Create();

			string nextName = to_string(atoi(stageName.c_str()) + 1);
			if (!StageMaker::GetInstance()->SetMap(nextName))
			{
				MessageBox(WindowFrame::GetInstance()->GetHWND(), TEXT("�������� �ʴ� ��"), TEXT("�˸�"), MB_OK);
			}
			StageMaker::GetInstance()->SetPlayerMode(m_mode);
			StageMaker::GetInstance()->StageStart();

			GameObject* btnObj = new GameObject();
			ColorButton* btn = new ColorButton();
			btnObj->AddComponent(btn);
			btnObj->SetOrderInLayer(10);
			btnObj->InitializeSet();
			btn->SetUIPos({ 0,400 });
			btn->SetUISize({ 200,50 });
			btn->SetText(TEXT("GameScene Load"));
			btn->SetTextColor(RGB(255, 0, 255));
			btn->SetDefaultColor(RGB(255, 255, 255));
			btn->SetHoverColor(RGB(200, 200, 200));
			btn->SetDownColor(RGB(150, 150, 150));
			btn->SetTextSize(20);
			btn->SetEvent(bind(&SceneChanger::ChangeGameScene, SceneChanger::GetInstance()));

			GameObject* btnObj2 = new GameObject();
			ColorButton* btn2 = new ColorButton();
			btnObj2->AddComponent(btn2);
			btnObj2->SetOrderInLayer(10);
			btnObj2->InitializeSet();
			btn2->SetUIPos({ 0,500 });
			btn2->SetUISize({ 200,50 });
			btn2->SetText(TEXT("StartScene Load"));
			btn2->SetTextColor(RGB(255, 0, 255));
			btn2->SetDefaultColor(RGB(255, 255, 255));
			btn2->SetHoverColor(RGB(200, 200, 200));
			btn2->SetDownColor(RGB(150, 150, 150));
			btn2->SetTextSize(20);
			btn2->SetEvent(bind(&SceneChanger::ChangeStartScene, SceneChanger::GetInstance()));
			return;
		}
	}
	else if (m_state == PlayerAState::eating &&  //���
		(other->GetGameObject()->GetTag() == TAG_CHANGE || other->GetGameObject()->GetTag() == TAG_MONSTER))
	{
		ChangeObject* co = other->GetGameObject()->GetComponent<ChangeObject>();
		if (co == nullptr) return;

		m_eatMode = co->GetMode();
		m_state = PlayerAState::eat_idle;
		UpdateAnim(true);

		other->GetGameObject()->SetDestroy(true);
	}	
	else if (m_mode == PlayerMode::mStone && m_state == PlayerAState::attack &&
		(other->GetGameObject()->GetTag() == TAG_HIT || other->GetGameObject()->GetTag() == TAG_MONSTER)) //���� �ǰ�
	{
		if (!isStoneAtked)
		{
			isStoneAtked = true;
			other->GetGameObject()->SetDestroy(true);//�� ������
		}
	}
	else if (m_state != PlayerAState::eating &&
		(other->GetGameObject()->GetTag() == TAG_HIT || other->GetGameObject()->GetTag() == TAG_MONSTER)) //�ǰ�
	{
		m_mode = PlayerMode::mDefault;
		m_state = PlayerAState::hit;
		UpdateAnim(true);
		if (m_arrow == Arrow::left)
		{
			m_rig->Velocity() = {300,-350};
		}
		else
		{
			m_rig->Velocity() = {-300,-350};
		}
	}
}

Player::Player()
	: Component(), m_mode(PlayerMode::mDefault), m_arrow(Arrow::right), m_state(PlayerAState::idle)
{
}

Player::~Player()
{
}

void Player::Initialize()
{
	m_gameObj->SetTag(TAG_PLAYER);
	m_gameObj->Size() = m_dSize;

	m_mode = PlayerMode::mDefault;
	string modeStr[(int)PlayerMode::max] = { "default", "sword", "stone"};
	string arrowStr[(int)Arrow::max] = { "left", "right" };
	string stateStr[(int)PlayerAState::max] = {};
	stateStr[(int)PlayerAState::idle] = "idle";
	stateStr[(int)PlayerAState::walk] = "walk";
	stateStr[(int)PlayerAState::jump] = "jump";
	stateStr[(int)PlayerAState::run] = "run";
	stateStr[(int)PlayerAState::fly] = "fly";
	stateStr[(int)PlayerAState::eat_idle] = "eat_idle";
	stateStr[(int)PlayerAState::eat_move] = "eat_move";
	stateStr[(int)PlayerAState::eat_jump] = "eat_jump";
	stateStr[(int)PlayerAState::change] = "change";
	stateStr[(int)PlayerAState::eat] = "eat";
	stateStr[(int)PlayerAState::eating] = "eating";
	stateStr[(int)PlayerAState::attack] = "attack";
	stateStr[(int)PlayerAState::hit] = "hit";

	m_attackFunc[(int)PlayerMode::mDefault] = &Player::Attack_default;
	m_attackFunc[(int)PlayerMode::mSword] = &Player::Attack_sword;
	m_attackFunc[(int)PlayerMode::mStone] = &Player::Attack_stone;

	for (int a = 0; a < (int)PlayerMode::max; a++)
	{
		for (int i = 0; i < (int)Arrow::max; i++)
		{
			for (int j = 0; j < (int)PlayerAState::max; j++)
			{
				string path = "Bitmaps\\Player\\" + modeStr[a] + "\\" + arrowStr[i] + "\\" + stateStr[j];
				float atime = 0.1f;
				if (j == (int)PlayerAState::jump) atime = 0.07f;
				else if (j == (int)PlayerAState::idle || j == (int)PlayerAState::eat_idle) atime = 0.25f;
				m_arrAnim[a][i][j] = AnimationManager::LoadAnimation(path, atime);
			}
		}
	}
	m_ar = new AnimationRender(m_arrAnim[(int)PlayerMode::mDefault][(int)Arrow::right][(int)PlayerAState::idle]);
	m_rig = new Rigidbody();
	m_bo = new BoxCollider();
	m_rig->SetGravity(0.0f);
	m_gameObj->AddComponent(m_bo);
	m_gameObj->AddComponent(m_ar);
	m_gameObj->AddComponent(m_rig);

	m_leftKey = VK_LEFT;
	m_rightKey = VK_RIGHT;
	m_jumpKey = VK_SPACE;
	m_atkKey = 0x5A;	//ZŰ, ATK
	m_changeKey = VK_DOWN;
	m_upKey = VK_UP;
}

void Player::Release()
{

	for (int a = 0; a < (int)PlayerMode::max; a++)
	{
		for (int i = 0; i < (int)Arrow::max; i++)
		{
			for (int j = 0; j < (int)PlayerAState::max; j++)
			{
				AnimationManager::ReleaseAnimation(m_arrAnim[a][i][j]);
			}
		}
	}
}

void Player::Start()
{
	m_rig->SetGravity(m_startGravity);
	m_bo->ColOffset() = m_dOffset;
	m_bo->ColSize() = m_cSize;
	RECT rect;
	GetClientRect(WindowFrame::GetInstance()->GetHWND(), &rect);
	Camera::GetInstance()->SetPos(
		m_gameObj->Position().x - rect.right / 2 + m_gameObj->Size().x / 2,
		m_gameObj->Position().y - rect.right / 2 + m_gameObj->Size().y / 2);
	if (m_rig)
		m_rig->SetNoFriction(true);

	m_flyTimer.tick();
	m_leftKeyTimer.tick();
	m_rightKeyTimer.tick();
}

void Player::Update()
{
	if (!m_rig)	return;
	if (!m_ar)	return;

	Vector2D playerPos = m_gameObj->Position();  // �÷��̾� ��ġ
	Vector2D camPos = Camera::GetInstance()->GetPos();  // ���� ī�޶� ��ġ
	RECT rect;
	GetClientRect(WindowFrame::GetInstance()->GetHWND(), &rect);
	if (m_state != PlayerAState::fly)m_rig->SetGravity(m_startGravity);
	// ���� ������ ����Ͽ� ī�޶� ��ġ ������Ʈ
	float smoothFactor = 0.02f;  // �ε巯�� �̵��� ���� ���� ���
	Vector2D newCamPos = {
		Lerp(camPos.x, playerPos.x - rect.right / 2 + m_gameObj->Size().x / 2, smoothFactor),
		Lerp(camPos.y, playerPos.y - rect.bottom / 2 + m_gameObj->Size().y / 2, smoothFactor)
	};
	Camera::GetInstance()->SetPos(newCamPos.x, newCamPos.y);

	if (m_mode == PlayerMode::mStone &&
		m_state == PlayerAState::attack)
	{
		m_rig->SetGravity(m_stoneGravity);
		StoneAttacking();
		return;
	}

	if (m_state == PlayerAState::change ||
		m_state == PlayerAState::attack ||
		m_state == PlayerAState::hit)
	{
		if (m_ar->IsFinishAnim())
		{
			m_state = PlayerAState::idle;
			UpdateAnim(true);
			return;
		}
		else
			return;
	}
	if (m_state == PlayerAState::eat )
	{
		if (m_ar->IsFinishAnim())
		{
			if (m_eatMode == PlayerMode::mDefault)
			{
				//ü�� ȸ��
				Idle();
				return;
			}
			m_state = PlayerAState::change;
			UpdateAnim(true);
			return;
		}
		else
			return;
	}

	if (m_state == PlayerAState::fly) //���� ���϶��� ó��
	{
		FlyAction();
		return;
	}

	if (m_rig->GetIsOnLand() && !GetAsyncKeyState(m_leftKey) && !GetAsyncKeyState(m_rightKey) && !GetAsyncKeyState(m_atkKey)) 	//Idle
		Idle();

	if (!GetAsyncKeyState(m_atkKey))
		m_atkTrigger = true;
	if (GetAsyncKeyState(m_atkKey) && m_state != PlayerAState::eat_jump) //����
	{
		(this->*m_attackFunc[(int)m_mode])();
	}
	else if (GetAsyncKeyState(m_changeKey) &&  //����
		(m_state == PlayerAState::eat_idle  || m_state == PlayerAState::eat_move))
	{
		m_mode = m_eatMode;
		m_state = PlayerAState::eat;
		UpdateAnim(true);
		return;
	}
	else if (!(GetAsyncKeyState(m_rightKey) && GetAsyncKeyState(m_leftKey))) //�̵�
	{
		if (GetAsyncKeyState(m_rightKey))//���� �̵�
		{
			if (m_rightKeyTrigger)
			{
				m_rightKeyTrigger = false;
				m_rightKeyTimer.tick();
			}
			MoveRight();
		}
		else
			m_rightKeyTrigger = true;

		if (GetAsyncKeyState(m_leftKey))//���� �̵�
		{
			if (m_leftKeyTrigger)
			{
				m_leftKeyTrigger = false;
				m_leftKeyTimer.tick();
			}
			MoveLeft();
		}
		else
			m_leftKeyTrigger = true;
	}
	if (GetAsyncKeyState(m_jumpKey)) //���� & ����
	{
		if (m_jumpFlyTrigger)
		{
			m_jumpFlyTrigger = false;
			JumpAction();
		}
	}
	else
		m_jumpFlyTrigger = true;


	//Debug
	if (DEBUGMODE && DebugWindow::GetInstance() != nullptr)
	{
		string text = "Player Velocity :\nX:" + to_string(m_rig->Velocity().x) + " \nY:" + to_string(m_rig->Velocity().y);
		DebugWindow::GetInstance()->SetText2(text);
	}
}

void Player::SetPlayerMode(PlayerMode mode)
{
	m_mode = mode;
}

PlayerMode Player::GetPlayerMode()
{
	return m_mode;
}

PlayerAState Player::GetState()
{
	return m_state;
}

Arrow Player::GetArrow()
{
	return m_arrow;
}
