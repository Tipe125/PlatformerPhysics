#include "Game.h"
//DXTK headers
#include "SimpleMath.h"

//system headers
#include <windows.h>
#include <time.h>

//our headers
#include "ObjectList.h"
#include "GameData.h"
#include "drawdata.h"
#include "DrawData2D.h"
#include "AntTweakBar.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Game::Game(ID3D11Device* _pd3dDevice, HWND _hWnd, HINSTANCE _hInstance) 
{
	//set up audio
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
	eflags = eflags | AudioEngine_Debug;
#endif
	m_audioEngine.reset(new AudioEngine(eflags));

	//Create DirectXTK spritebatch stuff
	ID3D11DeviceContext* pd3dImmediateContext;
	_pd3dDevice->GetImmediateContext(&pd3dImmediateContext);
	m_DD2D = new DrawData2D();
	m_DD2D->m_Sprites.reset(new SpriteBatch(pd3dImmediateContext));
	m_DD2D->m_Font.reset(new SpriteFont(_pd3dDevice, L"..\\Assets\\italic.spritefont"));

	//seed the random number generator
	srand((UINT)time(NULL));

	//Direct Input Stuff
	m_hWnd = _hWnd;
	m_pKeyboard = nullptr;
	m_pDirectInput = nullptr;

	HRESULT hr = DirectInput8Create(_hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDirectInput, NULL);
	hr = m_pDirectInput->CreateDevice(GUID_SysKeyboard, &m_pKeyboard, NULL);
	hr = m_pKeyboard->SetDataFormat(&c_dfDIKeyboard);
	hr = m_pKeyboard->SetCooperativeLevel(m_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	hr = m_pDirectInput->CreateDevice(GUID_SysMouse, &m_pMouse, NULL);
	hr = m_pMouse->SetDataFormat(&c_dfDIMouse);
	hr = m_pMouse->SetCooperativeLevel(m_hWnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	//create GameData struct and populate its pointers
	m_GD = new GameData;
	m_GD->m_keyboardState = m_keyboardState;
	m_GD->m_prevKeyboardState = m_prevKeyboardState;
	m_GD->m_GS = GS_PLAY_TPS_CAM;
	m_GD->m_mouseState = &m_mouseState;

	//set up DirectXTK Effects system
	m_fxFactory = new EffectFactory(_pd3dDevice);

	//Tell the fxFactory to look to the correct build directory to pull stuff in from
#ifdef DEBUG
	((EffectFactory*)m_fxFactory)->SetDirectory(L"../Debug");
#else
	((EffectFactory*)m_fxFactory)->SetDirectory(L"../Release");
#endif

	// Create other render resources here
	m_states = new CommonStates(_pd3dDevice);

	//init render system for VBGOs
	VBGO::Init(_pd3dDevice);

	//find how big my window is to correctly calculate my aspect ratio
	RECT rc;
	GetClientRect(m_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;
	float AR = (float)width / (float)height;

	//create a base camera
	m_cam = new Camera(0.25f * XM_PI, AR, 1.0f, 10000.0f, Vector3::UnitY, Vector3::Zero);
	m_cam->SetPos(Vector3(0.0f, 100.0f, 700.0f));
	m_GameObjects.push_back(m_cam);

	//create a base light
	m_light = new Light(Vector3(0.0f, 100.0f, 160.0f), Color(1.0f, 1.0f, 1.0f, 1.0f), Color(0.4f, 0.1f, 0.1f, 1.0f));
	m_GameObjects.push_back(m_light);

	//add Player
	Player* pPlayer = new Player("yoh.cmo", _pd3dDevice, m_fxFactory);
	//pPlayer->SetPitch(2);
	pPlayer->SetScale(3.0f);
	pPlayer->SetHeight(25);
	pPlayer->SetWidth(10);
	pPlayer->setType(PLAYER);
	m_GameObjects.push_back(pPlayer);
	m_collidables.push_back(pPlayer);

	//add a secondary camera
	m_TPScam = new TPSCamera(0.25f * XM_PI, AR, 1.0f, 10000.0f, pPlayer,Vector3::UnitY, Vector3(0.0f, 25.0f, 200.0f));
	m_GameObjects.push_back(m_TPScam);

	//create DrawData struct and populate its pointers
	m_DD = new DrawData;
	m_DD->m_pd3dImmediateContext = nullptr;
	m_DD->m_states = m_states;
	m_DD->m_cam = m_cam;
	m_DD->m_light = m_light;

	//add random content to show the various what you've got here
	Terrain* terrain = new Terrain("table.cmo", _pd3dDevice, m_fxFactory, Vector3(-100.0f, 50.0f, 0.0f), 0.0f, 0.0f, 0.0f, 0.25f * Vector3::One);
	terrain->setType(SPRING);
	terrain->SetHeight(25);
	terrain->SetWidth(25);
	m_GameObjects.push_back(terrain);
	m_collidables.push_back(terrain);

	//add some stuff to show off

	FileVBGO* terrainBox = new FileVBGO("../Assets/terrainTex.txt", _pd3dDevice);
	terrainBox->SetScale(20);
	terrainBox->SetPos(Vector3(0, -20, 0));
	m_GameObjects.push_back(terrainBox);

	/*FileVBGO* Box = new FileVBGO("../Assets/cube.txt", _pd3dDevice);
	m_GameObjects.push_back(Box);
	Box->SetPos(Vector3(0.0f, 0.0f, -100.0f));
	Box->SetPitch(XM_PIDIV4);
	Box->SetScale(20.0f);*/

	//L-system like tree
	//m_GameObjects.push_back(new Tree(4, 4, .6f, 10.0f *Vector3::Up, XM_PI/6.0f, "JEMINA vase -up.cmo", _pd3dDevice, m_fxFactory));

	VBCube* cube = new VBCube();
	cube->init(11, _pd3dDevice);
	cube->setType(PLATFORM);
	cube->SetHeight(25);
	cube->SetWidth(25);
	cube->SetPos(Vector3(100.0f, 50.0f, 0.0f));
	cube->SetScale(4.0f);
	m_GameObjects.push_back(cube);
	m_collidables.push_back(cube);


	VBCube* cube2 = new VBCube();
	cube2->init(11, _pd3dDevice);
	cube2->setType(CONVEYERLEFT);
	cube2->SetHeight(25);
	cube2->SetWidth(25);
	cube2->SetPos(Vector3(200.0f, 100.0f, 0.0f));
	cube2->SetScale(4.0f);
	m_GameObjects.push_back(cube2);
	m_collidables.push_back(cube2);

	VBCube* cube3 = new VBCube();
	cube3->init(11, _pd3dDevice);
	cube3->setType(ICE);
	cube3->SetHeight(25);
	cube3->SetWidth(25);
	cube3->SetPos(Vector3(300.0f, 75.0f, 0.0f));
	cube3->SetScale(4.0f);
	m_GameObjects.push_back(cube3);
	m_collidables.push_back(cube3);

	VBCube* cube4 = new VBCube();
	cube4->init(11, _pd3dDevice);
	cube4->setType(LADDER);
	cube4->SetHeight(25);
	cube4->SetWidth(25);
	cube4->SetPos(Vector3(400.0f, 75.0f, -50.0f));
	cube4->SetScale(4.0f);
	m_GameObjects.push_back(cube4);
	m_collidables.push_back(cube4);

	


	//example basic 2D stuff
	/*ImageGO2D* logo = new ImageGO2D("logo_small", _pd3dDevice);
	logo->SetPos(200.0f * Vector2::One);
	m_GameObject2Ds.push_back(logo);*/

	TextGO2D* text = new TextGO2D("Press E to change camera");
	text->SetPos(Vector2(10, 500));
	text->SetColour(Color((float*)&Colors::Yellow));
	m_GameObject2Ds.push_back(text);

	TwInit(TW_DIRECT3D11, _pd3dDevice);
	TwWindowSize(width, height);

	TwBar* physicsBar = TwNewBar("Physics");
	TwDefine("Global help = 'Change the values to change the physics of the player. '");
	int barSize[2] = { 250, 150 };
	TwSetParam(physicsBar, NULL, "size", TW_PARAM_INT32, 2, barSize);

	TwAddVarRW(physicsBar, "Gravity Multiplier", TW_TYPE_INT32, pPlayer->twGetGravityMultiplier(), "Group = 'Multipliers' min=0 max=100 step=1");
	TwAddVarRW(physicsBar, "Friction Multiplier", TW_TYPE_INT32, pPlayer->twGetFrictionMultiplier(), "Group = 'Multipliers' min=0 max=100 step=1");
	TwAddVarRW(physicsBar, "Acceleration Multiplier", TW_TYPE_INT32, pPlayer->twGetAccelMultiplier(), "Group = 'Multipliers' min=0 max=100 step=1");
	TwAddVarRW(physicsBar, "Deceleration Multiplier", TW_TYPE_INT32, pPlayer->twGetDecelMultiplier(), "Group = 'Multipliers' min=0 max=100 step=1");
	TwAddVarRW(physicsBar, "Jump Multiplier", TW_TYPE_INT32, pPlayer->twGetJumpMultiplier(), "Group = 'Multipliers' min=0 max=100 step=1");
	TwAddVarRW(physicsBar, "Top Speed Multiplier", TW_TYPE_INT32, pPlayer->twGetSpeedXMultiplier(), "Group = 'Multipliers' min=0 max=100 step=1");
	
};


Game::~Game() 
{
	//delete Game Data & Draw Data
	delete m_GD;
	delete m_DD;

	//tidy up VBGO render system
	VBGO::CleanUp();

	//tidy away Direct Input Stuff
	if (m_pKeyboard)
	{
		m_pKeyboard->Unacquire();
		m_pKeyboard->Release();
	}
	if (m_pMouse)
	{
		m_pMouse->Unacquire();
		m_pMouse->Release();
	}
	if (m_pDirectInput)
	{
		m_pDirectInput->Release();
	}

	//get rid of the game objects here
	for (list<GameObject *>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
	{
		delete (*it);
	}
	m_GameObjects.clear();

	m_collidables.clear();

	//and the 2D ones
	for (list<GameObject2D *>::iterator it = m_GameObject2Ds.begin(); it != m_GameObject2Ds.end(); it++)
	{
		delete (*it);
	}
	m_GameObject2Ds.clear();
	

	//clear away CMO render system
	delete m_states;
	delete m_fxFactory;

	delete m_DD2D;

	TwTerminate();

};

bool Game::Tick() 
{
	//tick audio engine
	if (!m_audioEngine->Update())
	{
		// No audio device is active
		if (m_audioEngine->IsCriticalError())
		{
			//something has gone wrong with audio so QUIT!
			return false;
		}
	}

	//Poll Keyboard & Mouse
	ReadInput();

	//Upon pressing escape QUIT
	if (m_keyboardState[DIK_ESCAPE] & 0x80)
	{
		return false;
	}

	//lock the cursor to the centre of the window
	RECT window;
	GetWindowRect(m_hWnd, &window);
	//SetCursorPos((window.left + window.right) >> 1, (window.bottom + window.top) >> 1);

	//calculate frame time-step dt for passing down to game objects
	DWORD currentTime = GetTickCount();
	m_GD->m_dt = min((float)(currentTime - m_playTime) / 1000.0f, 0.1f);
	m_playTime = currentTime;

	

	//start to a VERY simple FSM
	switch (m_GD->m_GS)
	{
	case GS_ATTRACT:
		break;
	case GS_PAUSE:
		break;
	case GS_GAME_OVER:
		break;
	case GS_PLAY_MAIN_CAM:
	case GS_PLAY_TPS_CAM:
		PlayTick();
		break;
	}
	
	return true;
}
//bool Game::CheckForCollision(Vector3 object1pos, Vector3 object2pos)
//{
	//return false;
//}
//;

void Game::PlayTick()
{
	//upon space bar switch camera state
	if ((m_keyboardState[DIK_E] & 0x80) && !(m_prevKeyboardState[DIK_E] & 0x80))
	{
		if (m_GD->m_GS == GS_PLAY_MAIN_CAM)
		{
			m_GD->m_GS = GS_PLAY_TPS_CAM;
		}
		else
		{
			m_GD->m_GS = GS_PLAY_MAIN_CAM;
		}
	}

	CollisionManagement();

	//update all objects
	for (list<GameObject *>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
	{
		(*it)->Tick(m_GD);
	}
	for (list<GameObject2D *>::iterator it = m_GameObject2Ds.begin(); it != m_GameObject2Ds.end(); it++)
	{
		(*it)->Tick(m_GD);
	}
}

void Game::CollisionManagement()
{
	for each(GameObject* object1 in m_collidables)
	{
		if (object1->GetType() != ObjectType::PLAYER)
		{
			for each(GameObject* object2 in m_collidables)
			{
				if (object2->GetType() == ObjectType::PLAYER)
				{

					if (CheckCollision(object1->GetPos(), object2->GetPos()))
					{
						//if (object1->GetType() == ObjectType::PLATFORM)
					//	{
							CollisionResolution(object1, object2);
							return;
					//	}
						
					}
					else
					{
						//if not colliding with any objects, must be mid air
						object2->setIsGrounded(false);
						object2->setOnLadder(false);						
						//object2->setFrictionMultiplier(40);

						
						
					}
				}
				
			}
		}
	}
}

bool Game::CheckCollision(Vector3 object1Pos, Vector3 object2Pos)
{
	/*if (Vector3::Distance(object1Pos, object2Pos) <= 25)
	{
		return true;
	}

	return false;*/

	if ((object2Pos.x + 25) >= object1Pos.x &&
		//if the player x pos + width is more than or = to the x pos of the platform - left collision
		object2Pos.x <= (object1Pos.x + 25) &&
		//right collision 
		(object2Pos.y + 47) >= object1Pos.y &&
		//top collision
		object2Pos.y <= (object1Pos.y + 30))
		//bottom collision
	{
		return true;
	}

	return false;
	
}

void Game::CollisionResolution(GameObject* object1, GameObject* object2)
{

	if (object1->GetType() != ObjectType::LADDER)
	{

		if (object2->GetPos().y >= object1->GetPos().y + 20 && object2->GetPos().y <= object1->GetPos().y + 24)
		{
			//Top collision
			if (object1->GetType() == ObjectType::CONVEYERLEFT)
			{
				printf("Jumped on Conveyer Belt Left \n");
				object2->setSpeedY(0.0f);
				object2->setIsGrounded(true);
				object2->setHasJumped(false);
				Vector3 currentPos = object2->GetPos();
				object2->SetPos(currentPos - Vector3(0.05, 0, 0));
			}
			else if (object1->GetType() == ObjectType::CONVEYERRIGHT)
			{
				printf("Jumped on Coveyer Belt Right \n");
				object2->setSpeedY(0.0f);
				object2->setIsGrounded(true);
				object2->setHasJumped(false);
				Vector3 currentPos = object2->GetPos();
				object2->SetPos(currentPos + Vector3(0.05, 0, 0));
			}
			else if (object1->GetType() == ObjectType::PLATFORM)
			{
				printf("Jumped on Default platform \n");
				object2->setSpeedY(0.0f);
				object2->setIsGrounded(true);
				object2->setHasJumped(false);
			}
			else if (object1->GetType() == ObjectType::ICE)
			{
				printf("Jumped on ice platform \n");
				object2->setSpeedY(0.0f);
				object2->setIsGrounded(true);
				object2->setHasJumped(false);
				object2->setFrictionMultiplier(25);

			}
			else if (object1->GetType() == ObjectType::SPRING)
			{
				printf("Jumped on spring \n");
				float springVel = object2->GetJumpSpeed();
				object2->setSpeedY(springVel * 2);
				object2->setHasJumped(false);
			}



		}
		//if the player is
		// x pos lower than left bound + 1
		// y pos lower than top bound
		//y pos higher than bottom bound
		else if (object2->GetPos().x < object1->GetPos().x - 20 && object2->GetPos().y < object1->GetPos().y + 25
			&& object2->GetPos().y > object1->GetPos().y - 45)
		{
			//left collision
			
				float currentY = object2->GetPos().y;
				float wallX = object1->GetPos().x - 25;
				object2->setSpeed(0);
				object2->SetPos(Vector3(wallX, currentY, 0));
			


		}
		//if the player is:
		// x Pos > left bound
		// x pos > right bound - 1
		// y pos < top bound
		//y pos > bottom bound
		else if (object2->GetPos().x > object1->GetPos().x + 20 && object2->GetPos().y < object1->GetPos().y + 25
			&& object2->GetPos().y > object1->GetPos().y - 45)
		{
			//right collision
			
				float currentY = object2->GetPos().y;
				float wallX = object1->GetPos().x + 25;
				object2->setSpeed(0);
				object2->SetPos(Vector3(wallX, currentY, 0));
			
			
		}


		else if (object2->GetPos().y <= object1->GetPos().y - 10)
		{
			
				float currentSpeedY = object2->GetYSpeed();
				object2->setSpeedY(-20);
		}

	}
	else
	{
		printf("Jumped on ladder \n");
		object2->setIsGrounded(true);
		object2->setHasJumped(false);
		object2->setSpeedY(0.0f);
		//object2->setSpeed(0.0f);
		object2->setOnLadder(true);
	}
	

}





void Game::Draw(ID3D11DeviceContext* _pd3dImmediateContext) 
{
	//set immediate context of the graphics device
	m_DD->m_pd3dImmediateContext = _pd3dImmediateContext;

	//set which camera to be used
	m_DD->m_cam = m_cam;
	if (m_GD->m_GS == GS_PLAY_TPS_CAM)
	{
		m_DD->m_cam = m_TPScam;
	}

	//update the constant buffer for the rendering of VBGOs
	VBGO::UpdateConstantBuffer(m_DD);

	//draw all objects
	for (list<GameObject *>::iterator it = m_GameObjects.begin(); it != m_GameObjects.end(); it++)
	{
		(*it)->Draw(m_DD);
	}

	// Draw sprite batch stuff 
	m_DD2D->m_Sprites->Begin();
	for (list<GameObject2D *>::iterator it = m_GameObject2Ds.begin(); it != m_GameObject2Ds.end(); it++)
	{
		(*it)->Draw(m_DD2D);
	}
	m_DD2D->m_Sprites->End();

	//drawing text screws up the Depth Stencil State, this puts it back again!
	_pd3dImmediateContext->OMSetDepthStencilState(m_states->DepthDefault(), 0);

	TwDraw();

};



bool Game::ReadInput()
{
	//copy over old keyboard state
	memcpy(m_prevKeyboardState, m_keyboardState, sizeof(unsigned char) * 256);

	//clear out previous state
	ZeroMemory(&m_keyboardState, sizeof(unsigned char) * 256);
	ZeroMemory(&m_mouseState, sizeof(DIMOUSESTATE));

	// Read the keyboard device.
	HRESULT hr = m_pKeyboard->GetDeviceState(sizeof(m_keyboardState), (LPVOID)&m_keyboardState);
	if (FAILED(hr))
	{
		// If the keyboard lost focus or was not acquired then try to get control back.
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
		{
			m_pKeyboard->Acquire();
		}
		else
		{
			return false;
		}
	}

	// Read the Mouse device.
	hr = m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
	if (FAILED(hr))
	{
		// If the Mouse lost focus or was not acquired then try to get control back.
		if ((hr == DIERR_INPUTLOST) || (hr == DIERR_NOTACQUIRED))
		{
			m_pMouse->Acquire();
		}
		else
		{
			return false;
		}
	}

	return true;
}