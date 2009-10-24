#include "directx.h"

CD3DManager::CD3DManager()
{
	pDevice = 0;
	m_font = 0;
	screenWidth = 0;
	screenHeight = 0;

	printFlags = PF_MESSAGES | PF_CVARS;

	AddCvar<int>("FPS", &fps);
	AddCvar<int>("Resolution X", &screenWidth);
	AddCvar<int>("Resolution Y", &screenHeight);
	AddCvar<bool>("Fullscreen", &bFullscreen);
}

CD3DManager::~CD3DManager()
{
	std::list<CCvarBase*>::iterator i;

	for (i = cvars.begin(); i != cvars.end(); ++i) {
		delete (*i);
	}
}

void CD3DManager::SetDevice(IDirect3DDevice8* device)
{
	pDevice = device;
	ReloadFont();
}

int CD3DManager::Update()
{
	if (!pDevice) return 0;

	ThreadRequestResources();

	// ---------------------------------------
	// Recalculate FPS
	// ---------------------------------------
	fps = CalculateFPS();

	// ---------------------------------------
	// Handle messages
	// ---------------------------------------
	{
		std::list<CMessage> expiredMessages;
		std::list<CMessage>::iterator i;

		// Draw Current Messages
		int yMod = 5;
		for (i = messages.begin(); i != messages.end(); ++i) {
			if ((*i).HasExpired()) {	// Will update the message's alpha value etc asd well
				expiredMessages.push_back(*i);
			}

			TextOut(10, yMod, (*i).msg, (*i).color);

			yMod += 20;
		}

		// kill dead messages
		for (i = expiredMessages.begin(); i != expiredMessages.end(); ++i) {
			messages.remove(*i);
		}
	}

	// ---------------------------------------
	// Handle cvars
	// ---------------------------------------
	if (printFlags & PF_CVARS) {
		std::list<CCvarBase*>::iterator i;

		int yMod = screenHeight - cvars.size()*20;
		for (i = cvars.begin(); i != cvars.end(); ++i) {
			TextOut(10, yMod, (*i)->ToString(), D3DCOLOR_XRGB(240, 150, 50));

			yMod += 20;
		}
	}

	ThreadReleaseResources();

	return 1;
}

int CD3DManager::OnLostDevice()
{
	// call OnLostDevice for all created resources like sprites / fonts
	m_font->OnLostDevice();

	return 1;
}

int CD3DManager::OnResetDevice()
{
	// call OnResetDevice for all created resources like sprites / fonts
	m_font->OnResetDevice();

	return 1;
}

int CD3DManager::ReloadFont()
{
	if (!pDevice) return 0;

	if (m_font) {
		m_font->Release();
		m_font = 0;
	}

	HFONT m_hfont = CreateFont(18, //height
		0, //width
		0, //escapment
		0, //orientation
		FW_NORMAL, //weight dontcare maybe bold etc.
		false, //itallic
		false, //underline
		false, //strikeout
		DEFAULT_CHARSET, //charset
		OUT_DEFAULT_PRECIS, //output precision
		CLIP_DEFAULT_PRECIS, // clip precision
		ANTIALIASED_QUALITY, //quality
		DEFAULT_PITCH | FF_DONTCARE, //pitch
		TEXT("MS Sans Serif"));

	D3DXCreateFont(pDevice, m_hfont, &m_font );

	DeleteObject(m_hfont);

	return 1;
}

int CD3DManager::TakeScreenshot(std::string file_name)
{
	// http://www.gamedev.net/reference/articles/article1844.asp

	if (!pDevice) return 0;

	IDirect3DSurface8* frontbuf;

	pDevice->CreateImageSurface(screenWidth, screenHeight, D3DFMT_A8R8G8B8, &frontbuf);

	HRESULT hr = pDevice->GetFrontBuffer(frontbuf);

	if(hr != D3D_OK)
	{
	  frontbuf->Release();
	  return 0;
	}

	D3DXSaveSurfaceToFileA(file_name.c_str(), D3DXIFF_BMP, frontbuf, NULL, NULL);

	frontbuf->Release();

	return 1;
}

int CD3DManager::PrintMessage(std::string msg, int duration, DWORD color)
{
	if (!(printFlags & PF_MESSAGES)) return 0;

	srand(GetTickCount());

	CMessage message(msg, duration, color);
	messages.push_back(message);

	return 1;
}

int CD3DManager::TextOut(int x, int y, std::string text, DWORD color)
{
	if (!pDevice || !m_font) return 0;

	RECT rect_Text = {x, y, x + text.length()*15, y + 25};

	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, true);
	pDevice->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_INVSRCALPHA);

	m_font->DrawTextA(text.c_str(), -1, &rect_Text, 0, color ); //DT_CENTER was 0

	return 1;
}

int CD3DManager::PutPixel(int x, int y, D3DCOLOR color)
{
	if (!pDevice) return 0;

	D3DRECT rect_Clear = {x, y, x+1, y+1};
	pDevice->Clear(1, &rect_Clear, D3DCLEAR_TARGET, color, 1.0f, 0);

	return 1;
}

int CD3DManager::CalculateFPS()
{
	static int oldTime = GetTickCount();
	static int frames = 0;
	static int result = 0;

	++frames;
	int dT = GetTickCount() - oldTime;

	if (dT >= 200) {
		oldTime = GetTickCount();
		result = static_cast<int>((double)frames / ((double)dT/1000));
		frames = 0;
	}

	return result;
}

int CD3DManager::SetScreenDimensions(int x, int y, bool fullscreen)
{
	screenWidth = x;
	screenHeight = y;
	bFullscreen = fullscreen;

	return 1;
}

int CD3DManager::GetScreenDimension(int* x, int* y, bool* fullscreen)
{
	if (x) *x = screenWidth;
	if (y) *y = screenHeight;
	if (fullscreen) *fullscreen = bFullscreen;

	return 1;
}

int CD3DManager::Fullscreen()
{
	return static_cast<int>(bFullscreen);
}

int CD3DManager::TogglePrintFlags(int flags)
{
	printFlags ^= flags;

	return 1;
}