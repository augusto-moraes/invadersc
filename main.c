#include <windows.h>
#include <stdio.h>
#include <gl/gl.h>

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);

// Struct referente a nave
struct Nave{
    int pontos, vidas, dir;
    double x, y;
};

// Struct referente ao tiro
struct Tiro{
    int shoot, tela, visu;
    double x, y, xi, yi;
};

//Variaveis guia para os inimigos (Posicao e movimento)
double xi[9] = {-0.525, -0.375, -0.225, -0.075, 0.075, 0.225, 0.375, 0.525, 0.675};
double y[3] = {0.925, 0.775, 0.625};
int alienDown=0, alienDir=0;

//Bool referente ao gamemode
int gamemode = 3, round=0;

//Limites da tela
int limDir=8, limEsq=0;

//Variaveis referentes ao tiro
double velocidadeColisao=0, velocidadeTiro=0;

void IniciaPlayer();
void DesenhaTela();
void DesenhaNave();
void DesenhaAliens();
void DesenhaTiro();
void Colisao();
void GameMode();
void GameOver();
void Menu();
void Movimenta();
void replay();
void reset();

struct Nave nave;
struct Tiro tiro;

//status dos aliens
int wave[3][9] ={{1,1,1,1,1,1,1,1,1},
                 {1,1,1,1,1,1,1,1,1},
                 {1,1,1,1,1,1,1,1,1}};

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
    //float theta = 0.0f;

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;


    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "Space Invaders",
                          WS_OVERLAPPEDWINDOW,
                          CW_USEDEFAULT,
                          CW_USEDEFAULT,
                          500,
                          500,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);

    IniciaPlayer();
    tiro.tela=0;
    nave.pontos=0;
    nave.vidas=2;
    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            GameMode();
            if(gamemode==1){
                DesenhaTela();
                DesenhaNave();
                DesenhaAliens();
                Movimenta();
                if(tiro.shoot == 1){
                    DesenhaTiro();
                    tiro.y += 0.1;
                }
            }
            else if(gamemode==2) GameOver();
            else Menu();

            Sleep(1);
            SwapBuffers(hDC);
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    if(gamemode == 1)
                        gamemode = 2;
                    else PostQuitMessage(0);
                break;

                case VK_LEFT:
                    nave.dir = -1;
                break;

                case VK_RIGHT:
                    nave.dir = 1;
                break;

                case VK_SPACE:
                    if(gamemode != 1) reset();
                    else tiro.shoot = 1;
                break;

            }
        }
        break;

        case WM_KEYUP:
        {
            switch (wParam)
            {
                case VK_LEFT:
                    nave.dir = 0;
                break;

                case VK_RIGHT:
                    nave.dir = 0;
                break;
               }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

void IniciaPlayer(){
    nave.x = 0;
    nave.y = -0.975;
}

void reset(){
    gamemode = 1;
    for(int j=0;j<3;j++)
        for(int x=0;x<9;x++)
            wave[j][x] = 1;

    for(int j=0;j<9;j++){
        if(j<3)
            y[j] = 0.925 - (j*0.15);
        xi[j] = -0.525 + (j*0.15);
    }

    alienDown=0;
    alienDir=0;
    limDir=8;
    limEsq=0;
    round=0;
    nave.pontos=0;
}

void replay(){
    DesenhaAliens();
    gamemode = 1;
    for(int j=0;j<3;j++)
        for(int x=0;x<9;x++)
            wave[j][x] = 1;

    for(int j=0;j<9;j++){
        if(j<3)
            y[j] = 0.925 - (j*0.15);
        xi[j] = -2.525 + (j*0.15);
    }

    alienDown=0;
    alienDir=0;
    limDir=8;
    limEsq=0;
    round++;
}

void GameMode(){
    int i, k=0;
    for(i=0;i<9;i++){
        if((wave[0][i] == 1 && y[0]-0.05 <= nave.y + 0.2) || (wave[1][i] == 1 && y[1]-0.05 <= nave.y + 0.2) || (wave[2][i] == 1 && y[2]-0.05 <= nave.y + 0.2)){
            if(!nave.vidas) gamemode = 2;
            else{
                nave.vidas--;
                round--;
                Sleep(1000);
                replay();
            }
        }
        if(!wave[0][i] && !wave[1][i] && !wave[2][i]) k++;
        if(k>=9){
            k=0;
            replay();
        }
    }
    printf("Pontuacao: %d \t Vidas: %d \t Round: %d\n", nave.pontos, nave.vidas, round);
}

void DesenhaTela(){
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

void DesenhaNave(){
    glBegin(GL_QUADS);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(nave.x+0.05, nave.y+0.1);
    glVertex2f(nave.x+0.05, nave.y+0.2);
    glVertex2f(nave.x-0.05, nave.y+0.2);
    glVertex2f(nave.x-0.05, nave.y+0.1);
    glEnd();

    if(nave.dir == -1 && nave.x-0.05 > -0.95)
        nave.x -= 0.01;
    if(nave.dir == 1 && nave.x+0.05 < 0.95)
        nave.x += 0.01;
}

void DesenhaAliens(){
    int i, j;
    for(j=0;j<3;j++){
        for(i=0;i<9;i++){
            if(wave[j][i]){
                glBegin(GL_QUADS);
                glColor3f(0.0f, 1.0f, 0.0f);
                glVertex2f(xi[i]+0.05, y[j]-0.05);
                glVertex2f(xi[i]+0.05, y[j]+0.05);
                glVertex2f(xi[i]-0.05, y[j]+0.05);
                glVertex2f(xi[i]-0.05, y[j]-0.05);
                glEnd();

                if(tiro.tela && ((tiro.x-0.01 >= xi[i]-0.05 && tiro.x - 0.01 <= xi[i] + 0.05 && tiro.y <= y[j]+0.05 && tiro.y+0.01 >= y[j]-0.05) || (tiro.x+0.01 >= xi[i]-0.05 && tiro.x+0.01 <= xi[i] + 0.05 && tiro.y <= y[j]+0.05 && tiro.y+0.01 >= y[j]-0.05))){
                    tiro.tela = 0;
                    wave[j][i] = 0;
                    tiro.shoot = 0;
                    velocidadeColisao -= 0.25;
                    nave.pontos+=10;
                }
            }
        }
    }
}

void Movimenta(){
    int i;
    if(!wave[0][limDir] && !wave[1][limDir] && !wave[2][limDir]) limDir--;
    if(!wave[0][limEsq] && !wave[1][limEsq] && !wave[2][limEsq]) limEsq++;

    if(xi[limDir]+0.05 < 0.95 && !alienDir){
        for(i=0;i<9;i++){
            xi[i] += 0.005 + 0.0015*round;
        }
    }
    else if(!alienDown){
        for(i=0;i<3;i++)
            y[i]-=0.075;
            alienDir = 1;
            alienDown = 1;
    }
    if(xi[limEsq]-0.05 > -0.95 && alienDir){
        for(i=0;i<9;i++){
            xi[i] -= 0.005 + 0.0015*round;
        }
    }
    else if(alienDown){
        for(i=0;i<3;i++)
            y[i]-=0.075;
            alienDir = 0;
            alienDown = 0;
    }
}

void DesenhaTiro(){
    if(tiro.shoot && !tiro.tela){
        tiro.tela = 1;
        tiro.visu = 0;
    }
    if(!tiro.visu){
        tiro.xi = nave.x;
        tiro.yi = nave.y +0.1;
        tiro.visu = 1;
        tiro.y = tiro.yi;
        tiro.x = tiro.xi;
    }
    if(tiro.tela){
        glBegin(GL_QUADS);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(tiro.x - 0.01, tiro.y);
        glVertex2f(tiro.x + 0.01, tiro.y);
        glVertex2f(tiro.x + 0.01, tiro.y+0.07);
        glVertex2f(tiro.x - 0.01, tiro.y+0.07);
        glEnd();
    }
    if(tiro.y > 1){
        tiro.tela = 0;
        tiro.shoot = 0;
        tiro.visu = 0;
    }
}

void GameOver(){
    glClearColor(1.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    //printf("GAME OVER\nPontuacao: %d\nPressione ESPACO para jogar novaente\nPressione Esc para ir para o menu\n", nave.pontos);
}

void Menu(){
    glClearColor(0.0, 0.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    //printf("Space Invaders\nPressione ESPACO para iniciar o jogo\nPressione Esc para sair\n");
}
