#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "glm.h"

#if defined(__APPLE__) || defined(MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

/**************************************
************* CONSTANTE PI ************
**************************************/

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

/**************************************
* AUXILIARES CONVERSÃO GRAUS-RADIANOS *
**************************************/

#define RAD(x)          (M_PI*(x)/180)
#define GRAUS(x)        (180*(x)/M_PI)

/**************************************
******** MACROS AUXILIARES ********
**************************************/

#define DEBUG 1

#define DELAY_MOVIMENTO     20
#define RAIO_ROTACAO        20

#define LARGURA_BASE        4
#define COMPRIMENTO_BASE    7
#define ALTURA_BASE         1

#define LARGURA_TORRE       2
#define COMPRIMENTO_TORRE   2
#define ALTURA_TORRE        0.5

#define COMPRIMENTO_CANHAO  4
#define RAIO_CANHAO         0.2

#define RAIO_RODAS 0.5
#define ALTURA_RODAS 0.5

#define CHAO_ALTURA_EIXO_POSITIVO 25
#define CHAO_ALTURA_EIXO_NEGATIVO -25

#define QTD_TEXT 3


/**************************************
********** VARIÁVEIS GLOBAIS **********
**************************************/

typedef struct {
  GLboolean   q,a,z,x,up,down,left,right;
}Teclas;

typedef struct {
  GLfloat    x,y,z;
}Pos;

typedef struct {
  Pos      eye,center,up;
  GLfloat  fov;
}Camera;

typedef struct {
  GLboolean   doubleBuffer;
  GLint       delayMovimento;
  Teclas      teclas;
  GLboolean   menuActivo;
  Camera      camera;
  GLboolean   debug;
  GLboolean   ortho;
}Estado;

typedef struct {
  GLfloat     x,y;
  GLfloat     velocidade;
  GLfloat     direccao;
  GLfloat     direccaoRodas;
  GLfloat     angTorre;
  GLfloat     angCanhao;
}Tanque;

typedef struct {
  Tanque      tanque;
  GLboolean   parado;
  GLuint textura[QTD_TEXT];
  GLint n_textura;
  GLint n_luz;
}Modelo;


Estado estado;
Modelo modelo;

/**************************************
*** INICIALIZAÇÃO DO AMBIENTE OPENGL **
**************************************/

void inicia_modelo()
{
  modelo.tanque.x=20;
  modelo.tanque.y=0;
  modelo.tanque.velocidade=0;
  modelo.tanque.direccao=0;
  modelo.tanque.direccaoRodas=0;
  modelo.tanque.angTorre=0;
  modelo.tanque.angCanhao=0;
  modelo.n_textura = 0;
  modelo.n_luz = 1;
}  

float light0[6][4]={
    {0.1f,0.1f,0.1f,1.f},//ambiente
    {0.8f,0.8f,0.8f,1.f},//difusa 0 branco
    {1.f,0.f,0.f,1.f},//difusa 1 vermelho
    {0.f,1.f,0.f,1.f},//difusa 2 verde
    {1.f,1.f,1.f,1.f},//especular
    {0.f,0.f,1.f,1.f}//posição
};

void gerarTexturas(void)
{
   unsigned char *image = NULL;
   int w, h;

   glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

   glGenTextures(QTD_TEXT, modelo.textura);

   image = glmReadPPM("data/portugal.ppm",&w, &h);
   glBindTexture(GL_TEXTURE_2D, modelo.textura[0]);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGB, GL_UNSIGNED_BYTE, image);

   image = glmReadPPM("data/textura-2.ppm",&w, &h);
   glBindTexture(GL_TEXTURE_2D, modelo.textura[1]);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGB, GL_UNSIGNED_BYTE, image);

   image = glmReadPPM("data/textura-3.ppm",&w, &h);
   glBindTexture(GL_TEXTURE_2D, modelo.textura[2]);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, w, h, GL_RGB, GL_UNSIGNED_BYTE, image);

   glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

void init(void)
{

  srand( (unsigned)time(NULL));

  modelo.parado=GL_FALSE;

  estado.debug=DEBUG;
  estado.menuActivo=GL_FALSE;
  estado.delayMovimento=DELAY_MOVIMENTO;
  estado.camera.eye.x=40;
  estado.camera.eye.y=40;
  estado.camera.eye.z=40;
  estado.camera.center.x=0;
  estado.camera.center.y=0;
  estado.camera.center.z=0;
  estado.camera.up.x=0;
  estado.camera.up.y=0;
  estado.camera.up.z=1;
  estado.ortho=GL_TRUE;
  estado.camera.fov=60;
  
  estado.teclas.a=estado.teclas.q = estado.teclas.z=estado.teclas.x=\
  estado.teclas.up=estado.teclas.down=estado.teclas.left=estado.teclas.right=GL_FALSE;

  inicia_modelo();
    
  glClearColor(0.0, 0.0, 0.0, 0.0);

  glEnable(GL_POINT_SMOOTH);
  glEnable(GL_LINE_SMOOTH);
  glEnable(GL_POLYGON_SMOOTH);
  glEnable(GL_DEPTH_TEST);
  //glutIgnoreKeyRepeat(GL_TRUE);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_NORMALIZE);

  float globalAmb[]= {0.1f, 0.1f, 0.1f, 1.f};
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmb);//luz ambiente

  glLightfv(GL_LIGHT0,GL_AMBIENT,&light0[0][0]);
  glLightfv(GL_LIGHT0,GL_SPECULAR,&light0[4][0]);
  glLightfv(GL_LIGHT0,GL_POSITION,&light0[5][0]);

  gerarTexturas();

}

/**************************************
***** CALL BACKS DE JANELA/DESENHO ****
**************************************/

/* Callback para redimensionar janela */
void reshape(int width, int height)
{
  // glViewport(botom, left, width, height)
  // define parte da janela a ser utilizada pelo OpenGL
  glViewport(0, 0, (GLint) width, (GLint) height);
  
  // Matriz Projeccao
  // Matriz onde se define como o mundo é apresentado na janela
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  // gluOrtho2D(left,right,bottom,top); 
  // projeção ortogonal 2D, com profundidade (Z) entre -1 e 1
  
  if(estado.debug)
    printf("Reshape %s\n",(estado.ortho)? "ORTHO" : "PERSPECTIVE");

  if(estado.ortho)
  {
    if (width < height)
       glOrtho(-20, 20, -20*(GLdouble)height/width, 20*(GLdouble)height/width,-100,100);
    else
       glOrtho(-20*(GLdouble)width/height, 20*(GLdouble)width/height,-20, 20, -100,100);
  }
  else
    gluPerspective(estado.camera.fov,(GLfloat)width/height,1,100);

  // Matriz Modelview
  // Matriz onde são realizadas as tranformações dos modelos desenhados
  glMatrixMode(GL_MODELVIEW);
  
}
/**************************************
** ESPAÇO PARA DEFINIÇÃO DAS ROTINAS **
****** AUXILIARES DE DESENHO ... ******
**************************************/
void desenhaPoligono(GLfloat a[], GLfloat b[], GLfloat c[], GLfloat  d[], GLfloat cor[], GLfloat normal[])
{
  glBegin(GL_POLYGON);
    glNormal3fv(normal);
    glColor3fv(cor);
	glTexCoord2f(0.0, 0.0);
    glVertex3fv(a);
	glTexCoord2f(1.0, 0.0);
    glVertex3fv(b);
	glTexCoord2f(1.0, 1.0);
    glVertex3fv(c);
	glTexCoord2f(0.0, 1.0);
    glVertex3fv(d);
  glEnd();
}

void cubo()
{
  GLfloat normais[][3] = {
      {0, 0, -1}, // 0
      {0, 1, 0},  // 1
      {-1, 0, 0},   // 2
      {1, 0, 0},  // 3
      {0, 0, 1},  // 4
      {0, -1, 0},   // 5
  };
   /* Colocar aqui o código de desenhar um cubo da ficha anterior */
   GLfloat vertices[][3] = {  {-0.5,-0.5,-0.5}, 
                {0.5,-0.5,-0.5}, 
                {0.5,0.5,-0.5}, 
                {-0.5,0.5,-0.5}, 
                {-0.5,-0.5,0.5},  
                {0.5,-0.5,0.5}, 
                {0.5,0.5,0.5}, 
                {-0.5,0.5,0.5}};

  GLfloat cores[][3] = {{0.0,1.0,1.0},
                        {1.0,0.0,0.0},
                        {1.0,1.0,0.0}, 
                        {0.0,1.0,0.0}, 
                        {1.0,0.0,1.0}, 
                        {0.0,0.0,1.0}, 
                        {1.0,1.0,1.0}};

  float matSpecular[]={1.f,1.f,1.f,1.f};
  glMaterialfv(GL_FRONT, GL_SPECULAR,matSpecular);
  glMaterialf(GL_FRONT, GL_SHININESS, 128);                      

  glBindTexture(GL_TEXTURE_2D, modelo.textura[modelo.n_textura]);
  desenhaPoligono(vertices[1],vertices[0],vertices[3],vertices[2],cores[6], normais[4]);
  desenhaPoligono(vertices[2],vertices[3],vertices[7],vertices[6],cores[6], normais[1]);
  desenhaPoligono(vertices[3],vertices[0],vertices[4],vertices[7],cores[6], normais[2]);
  desenhaPoligono(vertices[6],vertices[5],vertices[1],vertices[2],cores[6], normais[3]);
  desenhaPoligono(vertices[4],vertices[5],vertices[6],vertices[7],cores[6], normais[0]);
  desenhaPoligono(vertices[0],vertices[1],vertices[5],vertices[4],cores[6], normais[5]);
}

void strokeString(char *str,double x, double y, double z, double s)
{
	int i,n;
	
	n = strlen(str);
	glPushMatrix();
	glColor3d(0.0, 0.0, 0.0);
	glTranslated(x,y,z);
	glScaled(s,s,s);
	for(i=0;i<n;i++)
		glutStrokeCharacter(GLUT_STROKE_ROMAN,(int)str[i]);

	glPopMatrix();

}

void bitmapString(char *str, double x, double y)
{
	int i,n;
  
  // fonte pode ser:
  // GLUT_BITMAP_8_BY_13
  // GLUT_BITMAP_9_BY_15
  // GLUT_BITMAP_TIMES_ROMAN_10
  // GLUT_BITMAP_TIMES_ROMAN_24
  // GLUT_BITMAP_HELVETICA_10
  // GLUT_BITMAP_HELVETICA_12
  // GLUT_BITMAP_HELVETICA_18
  //
  // int glutBitmapWidth  	(	void *font , int character);
  // devolve a largura de um carácter
  //
  // int glutBitmapLength 	(	void *font , const unsigned char *string );
  // devolve a largura de uma string (soma da largura de todos os caracteres)

	n = strlen(str);
	glRasterPos2d(x,y);
	for (i=0;i<n;i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,(int)str[i]);
}

void bitmapCenterString(char *str, double x, double y)
{
	int i,n;
  
	n = strlen(str);
	glRasterPos2d(x-glutBitmapLength(GLUT_BITMAP_HELVETICA_18,(const unsigned char *)str)*0.5,y);
	for (i=0;i<n;i++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,(int)str[i]);
}


// ... Definição das rotinas auxiliares de desenho ...

void desenhaTanque(Tanque t)
{
  GLUquadricObj* quadratic;
  quadratic = gluNewQuadric();
  
 glPushMatrix(); 

  // puxar tudo para cima
  glTranslatef(0,0,1);
  
  // Rodas direita
  glPushMatrix();
    glColor3f(1.0,1.0,0.0);
    glTranslatef(COMPRIMENTO_BASE / 1.75 ,LARGURA_BASE / 1.3 - ALTURA_RODAS * 2, 0);
    glRotatef(90,1,0,0);
    //gluQuadricDrawStyle(quadratic,GLU_FILL);
    for(int i = 0; i < COMPRIMENTO_BASE / (RAIO_RODAS*2); i++){
        glTranslatef(-(RAIO_RODAS*2), 0, 0); 
        gluCylinder(quadratic, RAIO_RODAS, RAIO_RODAS, ALTURA_RODAS, 32, 32); 
    }
  glPopMatrix();

  // Rodas esquerda
  glPushMatrix();
    glColor3f(0.0,1.0,0.0);
    glTranslatef((COMPRIMENTO_BASE / 1.75) ,(-(LARGURA_BASE / 6.2) - (ALTURA_RODAS * 2)), 0);
    glRotatef(90,1,0,0);
    //gluQuadricDrawStyle(quadratic,GLU_FILL);
    for(int i = 0; i < COMPRIMENTO_BASE / (RAIO_RODAS*2); i++){
        glTranslatef(-(RAIO_RODAS*2), 0, 0); 
        gluCylinder(quadratic, RAIO_RODAS, RAIO_RODAS, ALTURA_RODAS, 32, 32); 
    }
  glPopMatrix();

  // BASE DO TANQUE
  glTranslatef(0, 0, ALTURA_BASE);
  glPushMatrix();
    glScalef(COMPRIMENTO_BASE, LARGURA_BASE, ALTURA_BASE);
    cubo();
  glPopMatrix();

  // TORRE DO TANQUE
  glTranslatef(0, 0, ALTURA_TORRE * 2); 
  glRotatef(t.angTorre, 0, 0, 1);
  glPushMatrix();
    glScalef(COMPRIMENTO_TORRE, LARGURA_TORRE, ALTURA_TORRE);
    cubo();
  glPopMatrix();

  // CANHAO DO TANQUE
  glTranslatef(COMPRIMENTO_TORRE / 2, 0, 0);
  glRotatef(-t.angCanhao, 0, 1, 0);
  glTranslatef(COMPRIMENTO_CANHAO / 2, 0, 0);
  glPushMatrix();
    glScalef(COMPRIMENTO_CANHAO, RAIO_CANHAO, RAIO_CANHAO);
    cubo();
  glPopMatrix();

 glPopMatrix(); 
}

void desenhaChao(GLfloat dimensao)
{
  glBegin(GL_POLYGON);
    glVertex3f(dimensao,dimensao,0);
    glVertex3f(-dimensao,dimensao,0);
    glVertex3f(-dimensao,-dimensao,0);
    glVertex3f(dimensao,-dimensao,0);
  glEnd();
}

/* Callback de desenho */
void draw(void)
{  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // iluminação
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_COLOR_MATERIAL);

  float pos[] = {2, 2, 2, 0};
  glLightfv(GL_LIGHT0, GL_POSITION, pos);

  glLoadIdentity();

  gluLookAt(estado.camera.eye.x,estado.camera.eye.y,estado.camera.eye.z, \
            estado.camera.center.x,estado.camera.center.y,estado.camera.center.z,\
            estado.camera.up.x,estado.camera.up.y,estado.camera.up.z);

  // ... Chamada das rotinas auxiliares de desenho ...

  glColor3f(0.5f,0.5f,0.5f);
  
  glDisable(GL_TEXTURE_2D);
  desenhaChao(RAIO_ROTACAO+5);

  glEnable(GL_TEXTURE_2D);
  glPushMatrix();      
    glTranslatef(modelo.tanque.x, modelo.tanque.y, 0);
    glRotatef(modelo.tanque.direccao, 0, 0, 1);
    desenhaTanque(modelo.tanque);
  glPopMatrix();
  
  
  glFlush();

  if (estado.doubleBuffer)
    glutSwapBuffers();
}


/**************************************
******** CALLBACKS TIME/IDLE **********
**************************************/

/* Callback Idle */
void idle(void)
{
  /* Ações a tomar quando o GLUT está idle */

  /* Redesenhar o ecrã */
  // glutPostRedisplay();
}

/* Callback de temporizador */
void timer(int value)
{
  glutTimerFunc(estado.delayMovimento, timer, 0);
  // ... Ações do temporizador ... 

  if(estado.menuActivo || modelo.parado) // Sair em caso de o jogo estar parado ou menu estar activo
    return;


  glLightfv(GL_LIGHT0,GL_DIFFUSE,&light0[modelo.n_luz][0]);

    // acelrar para a frente
    if(modelo.tanque.direccaoRodas == 2){
      modelo.tanque.velocidade += 0.1;
      modelo.tanque.x += (0.02 * modelo.tanque.velocidade * cos(RAD(modelo.tanque.direccao)));
      modelo.tanque.y += (0.02 * modelo.tanque.velocidade * sin(RAD(modelo.tanque.direccao)));
    }
    // acelerar para tras
    else if(modelo.tanque.direccaoRodas == -2){
      modelo.tanque.velocidade += 0.1;
      modelo.tanque.x -= (0.02 * modelo.tanque.velocidade * cos(RAD(modelo.tanque.direccao)));
      modelo.tanque.y -= (0.02 * modelo.tanque.velocidade * sin(RAD(modelo.tanque.direccao)));
    }
    else if(modelo.tanque.velocidade > 0){
      // abrandar para a frente
      if(modelo.tanque.direccaoRodas == 1){
        modelo.tanque.velocidade -= 0.1;
        modelo.tanque.x += (0.02 * modelo.tanque.velocidade * cos(RAD(modelo.tanque.direccao)));
        modelo.tanque.y += (0.02 * modelo.tanque.velocidade * sin(RAD(modelo.tanque.direccao)));
      }
      // abrandar para tras
      else if(modelo.tanque.direccaoRodas == -1){
        modelo.tanque.velocidade -= 0.1;
        modelo.tanque.x -= (0.02 * modelo.tanque.velocidade * cos(RAD(modelo.tanque.direccao)));
        modelo.tanque.y -= (0.02 * modelo.tanque.velocidade * sin(RAD(modelo.tanque.direccao)));
      }
    }
    else
    // ficar parado
      modelo.tanque.direccaoRodas = 0;

    if(modelo.tanque.x > CHAO_ALTURA_EIXO_POSITIVO) {
      modelo.tanque.x = CHAO_ALTURA_EIXO_POSITIVO;
      modelo.tanque.velocidade = 0;
    } 

    if(modelo.tanque.x < CHAO_ALTURA_EIXO_NEGATIVO) {
      modelo.tanque.x = CHAO_ALTURA_EIXO_NEGATIVO;
      modelo.tanque.velocidade = 0;
    } 

    if(modelo.tanque.y > CHAO_ALTURA_EIXO_POSITIVO) {
      modelo.tanque.y = CHAO_ALTURA_EIXO_POSITIVO;
      modelo.tanque.velocidade = 0;
    } 

    if(modelo.tanque.y < CHAO_ALTURA_EIXO_NEGATIVO) {
      modelo.tanque.y = CHAO_ALTURA_EIXO_NEGATIVO;
      modelo.tanque.velocidade = 0;
    } 

    if (estado.teclas.q)
    {
      if (modelo.tanque.angCanhao < 45)
         modelo.tanque.angCanhao++;
         estado.teclas.q=GL_FALSE;
    }

    if (estado.teclas.a)
    {
      if (modelo.tanque.angCanhao > ALTURA_BASE)
        modelo.tanque.angCanhao--;
        estado.teclas.a=GL_FALSE;
    }

    if (estado.teclas.z){
      modelo.tanque.angTorre++;
      estado.teclas.z=GL_FALSE;
    }

    if (estado.teclas.x){
      modelo.tanque.angTorre--;
      estado.teclas.x=GL_FALSE;
    }

  // redesenhar o ecra 
  glutPostRedisplay();
}

/**************************************
*********** FUNÇÃO AJUDA **************
**************************************/

void imprime_ajuda(void)
{
  printf("\n\nDesenho de um quadrado\n");
  printf("h,H - Ajuda \n");
  printf("z,Z - Roda torre para a esquerda\n");
  printf("x,X - Roda torre para a direita\n");
  printf("q,Q - Levantar canhao\n");
  printf("a,A - Baixar canhao\n");
  printf("i,I - Reinicia modelo\n");
  printf("o,O - Alterna entre projecãoo Ortografica e Perspectiva\n");
  printf("f,F - Poligono Fill \n");
  printf("l,L - Poligono Line \n");
  printf("p,P - Poligono Point \n");
  printf("s,S - Inicia/para movimento\n");
  printf("ESC - Sair\n");
}

/**************************************
********* CALLBACKS TECLADO ***********
**************************************/

/* Callback para interação via teclado (carregar na tecla) */
void key(unsigned char key, int x, int y)
{
  switch (key) {

    case 27:
      exit(1);
    // ... Ações sobre outras teclas ... 

    case 'h' :
    case 'H' :
            imprime_ajuda();
            break;
    case 'i' :
    case 'I' :
            inicia_modelo();
            break;
    case 'o' :
    case 'O' :
            estado.ortho=!estado.ortho;
            reshape(glutGet(GLUT_WINDOW_WIDTH),glutGet(GLUT_WINDOW_HEIGHT));
            break;
            // passar para o timer
    case 'Q' : 
    case 'q' : estado.teclas.q=GL_TRUE;
            break; 
    case 'A' : 
    case 'a' : estado.teclas.a=GL_TRUE;
            break;
    case 'Z' : 
    case 'z' : estado.teclas.z=GL_TRUE;
            break;
    case 'X' : 
    case 'x' : estado.teclas.x=GL_TRUE;
            break;      
    case 'D' : 
    case 'd' : estado.debug=!estado.debug;
           if(estado.menuActivo || modelo.parado)
                glutPostRedisplay();
           printf("DEBUG is %s\n",(estado.debug)?"ON":"OFF");
           break;
    case 'f' : 
    case 'F' : 
           glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
           break;
    case 'p' : 
    case 'P' : 
           glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
           break;
    case 'l' : 
    case 'L' : 
           glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
           break;

   /* case 's' : 
    case 'S' : 
               modelo.parado=!modelo.parado;
           break;*/
  }

  if (DEBUG)
    printf("Carregou na tecla %c\n", key);
}

/* Callback para interação via teclado (largar a tecla) */
void keyUp(unsigned char key, int x, int y)
{
  switch (key) {
    // ... Ações sobre largar teclas ... 

  }

  if (DEBUG)
    printf("Largou a tecla %c\n", key);
}

/* Callback para interacção via teclas especiais (carregar na tecla) */
void specialKey(int key, int x, int y)
{
  /* Ações sobre outras teclas especiais
      GLUT_KEY_F1 ... GLUT_KEY_F12
      GLUT_KEY_UP
      GLUT_KEY_DOWN
      GLUT_KEY_LEFT
      GLUT_KEY_RIGHT
      GLUT_KEY_PAGE_UP
      GLUT_KEY_PAGE_DOWN
      GLUT_KEY_HOME
      GLUT_KEY_END
      GLUT_KEY_INSERT */

  switch (key)
  {
    /* Redesenhar o ecrã */
    case GLUT_KEY_UP :
            modelo.tanque.direccao += 4;
            break;
    case GLUT_KEY_DOWN :
            modelo.tanque.direccao -= 4;
            break;
    case GLUT_KEY_LEFT : 
      if (modelo.tanque.direccaoRodas >= 0)
        modelo.tanque.direccaoRodas = 2;
      break;       
    case GLUT_KEY_RIGHT : 
      if (modelo.tanque.direccaoRodas <= 0)
        modelo.tanque.direccaoRodas = -2;
      break;        
    //glutPostRedisplay();
  }

  if (DEBUG)
    printf("Carregou na tecla especial %d\n", key);
}

/* Callback para interação via teclas especiais (largar a tecla) */
void specialKeyUp(int key, int x, int y)
{
  switch (key)
  {
    case GLUT_KEY_LEFT :
            modelo.tanque.direccaoRodas = 1;
            break;
    case GLUT_KEY_RIGHT : 
            modelo.tanque.direccaoRodas = -1;
            break;
  }

  if (DEBUG)
    printf("Largou a tecla especial %d\n", key);
}


void menu(int op)
{
    switch (op)
    {
        case 0:
          modelo.n_luz = 0;
           break;
        case 1:
          modelo.n_luz = 1;
          break;
        case 2:
          modelo.n_luz = 2;
          break;
        case 3: 
          modelo.n_luz = 3;
          break;    
        case 4:
          modelo.n_textura = 0;
          break;
        case 5:
          modelo.n_textura = 1;
          break; 
        case 6:
          modelo.n_textura = 2;
          break;
    }

    glutPostRedisplay();
}

void menu_rato()
{
  int menu1 = glutCreateMenu(menu);
  glutAddMenuEntry("Textura 1", 4);
  glutAddMenuEntry("Textura 2", 5);
  glutAddMenuEntry("Textura 3", 6);

  int menu2 = glutCreateMenu(menu);
  glutAddMenuEntry("Iluminação 0", 0);
  glutAddMenuEntry("Iluminação 1", 1);
  glutAddMenuEntry("Iluminação 2", 2);
  glutAddMenuEntry("Iluminação 3", 3);

  int menu3 = glutCreateMenu(menu);
  glutAddSubMenu("Texturas", menu1);
  glutAddSubMenu("Luzes", menu2);
  glutAttachMenu(GLUT_RIGHT_BUTTON);
}



/**************************************
************ FUNÇÃO MAIN **************
**************************************/

int main(int argc, char **argv)
{
  estado.doubleBuffer = GL_TRUE; 

  glutInit(&argc, argv);
  glutInitWindowPosition(0, 0);
  glutInitWindowSize(640, 480);
  glutInitDisplayMode(((estado.doubleBuffer) ? GLUT_DOUBLE : GLUT_SINGLE) | GLUT_RGB | GLUT_DEPTH);
  if (glutCreateWindow("Tanque") == GL_FALSE)
    exit(1);

  init();

  imprime_ajuda();

  menu_rato();

  /* Registar callbacks do GLUT */

  /* callbacks de janelas/desenho */
  glutReshapeFunc(reshape);
  glutDisplayFunc(draw);

  /* Callbacks de teclado */
  glutKeyboardFunc(key);
  glutKeyboardUpFunc(keyUp);
  glutSpecialFunc(specialKey);
  glutSpecialUpFunc(specialKeyUp);

  /* Callbacks timer/idle */
  glutTimerFunc(estado.delayMovimento, timer, 0);
  //glutIdleFunc(idle);

  /* Começar loop */
  glutMainLoop();

  return 0;
}
