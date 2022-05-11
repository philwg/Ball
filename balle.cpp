#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <GL/glut.h>
#include <cmath>

#define PHI 1.61803398875

using namespace std;


/* ------------------------------------------------------------------- */
/*				Les variables globales de la classe			   		   */
/* ------------------------------------------------------------------- */

char presse;										// Le caractère lu au clavier
GLint xold, yold, anglex, angley;					// Les contrôles des éléments gérés à la souris
GLdouble angleScene {5.0}, angleBalle {0.0};		// Les angles de rotation des sous-scènes	

GLdouble minx {-7.0}, miny { -2.0}, minz {-7.0};	// Les paramètres min de la fenêtre de vue ...
GLdouble maxx {+7.0}, maxy {+12.0}, maxz {+7.0};	// Les paramètres max de la fenêtre de vue ...

GLdouble Ball[4][4]	{0.0, 0.0, 0.0, 0.0,			// Les paramètres de rotation de la balle
	 				 1.0, 1.0, 1.0, 0.6,			// Les paramètres d'échelle et le rayon
					 0.0, 0.0, 0.0, 36.0,			// Les paramètres de translation et la finesse du tracé
					 1.0, 0.0, 1.0, 0.0};			// Les paramètres de couleur et la distance au sol
					  							
const GLdouble	sFactorMax 		{PHI},				// La limite d'étirement
				sFactorMin 		{PHI-1.0},			// la limite d'écrasement
				deltaZ 	   		{0.25},				// l'incrément de zoom de la fenêtre de vue
				dispMaxHeight	{9.0};				// l'amplitude du rebond

GLint crntIndex {0}, rangeMax {120};				// les contrôles du rebond
GLdouble factorUp, factorDown, chapSize;			//


/* ------------------------------------------------------------------- */
/*						Prototypes des fonctions	   				   */
/* ------------------------------------------------------------------- */

void drawSol();
void drawBall();
void ballScene();

void idle();
void reshape(int width, int height);
void mouse(int button, int state, int x, int y);
void mousemotion(int x, int y);

void zoomIO(GLdouble d);
GLdouble sqr(GLdouble x);
void adjustFloorPosition();
void setSSParams(GLdouble f);
void updateSSParams(GLdouble f);
GLdouble getNewHeight();
void moveBalle();

void clavier(unsigned char touche, int x, int y);
void affichage();


/* ------------------------------------------------------------------- */
/*						Fonctions de Tracé			   				   */
/* ------------------------------------------------------------------- */

void drawSol()		//--------------------------------------- Dessine un rectangle pour symboliser le sol
{
	glColor3d(0.8, 0.8, 0.8);
	glBegin(GL_QUADS);
		glVertex3d(-4.0, 0.0, -8.0);
		glVertex3d(-4.0, 0.0, +8.0);
		glVertex3d(+4.0, 0.0, +8.0);
		glVertex3d(+4.0, 0.0, -8.0);
	glEnd();
}

void drawBall() 	//--------------------------------------- Le tracé de la balle
{
	glPushMatrix();
		glColor3d		(Ball[3][0], Ball[3][1], Ball[3][2]); 					// Sa couleur
		glTranslated	(Ball[2][0], Ball[2][1], Ball[2][2]);					// Ses translations
		glScaled		(Ball[1][0], Ball[1][1], Ball[1][2]);					// Ses échelles
		glRotated		(Ball[0][0], Ball[0][1], Ball[0][2], Ball[0][3]);		// Sa rotation
		glutSolidSphere	(Ball[1][3], round(Ball[2][3]), round(Ball[2][3]));		// L'objet lui-même
	glPopMatrix();
}

void ballScene()	//------------------------------------------- la structure OpenGL de la scène
{
	glPushMatrix();
	
		glRotated(angleScene, 1.0, 1.0, 1.0);		//--- Préparer la rotation de la scene
		drawSol();									//--- Le sol
		glTranslated(0.0, Ball[1][3], 0.0);			//--- Translation vers la scène (balle au sol)
		
		glPushMatrix();
			glRotatef(angleBalle, 0.0, 0.0, 1.0);			//--- Préparer la rotation de la balle
			glTranslated(0.0, Ball[3][3], 0.0);				//--- Translation vers la balle
			drawBall();										//--- La balle
		glPopMatrix();
		
	glPopMatrix();	
}

/* ------------------------------------------------------------------- */
/*							Fonction IDLE						       */
/* ------------------------------------------------------------------- */

void idle()		//------------------------------------------- Définition du rebond comme tâche de fond
{
	if (true) {
		if (crntIndex > rangeMax) {					//------- si le compteur dépasse le maximum
			crntIndex = 0;							//-------	on le fait boucler à zéro
			setSSParams(sFactorMin);				//------- 	on repart de la boule écrasée au sol
		}
		moveBalle();								//------- on calcule la position suivante
		crntIndex++;								//------- on augmente l'incrément
	}
	usleep(20000);
	glutPostRedisplay();
}

/* ------------------------------------------------------------------- */
/*					Fonction de Redimensionnement				       */
/* ------------------------------------------------------------------- */

void reshape(int width, int height)		//------------------- Pour s'adapter aux redimensionnement de la fenêtre
{
	if (width < height) {
		glViewport(0, (height - width) / 2, width, width);	
	}
	else {
		glViewport((width - height)/2, 0, height, height);	
	}
}


/* ------------------------------------------------------------------- */
/*					Fonctions de Gestion de la Souris			       */
/* ------------------------------------------------------------------- */

void mouse(int button, int state, int x, int y)	//----------- Gestion du clic de la souris
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		presse = 1;
		xold = x;
		yold = y;
	}
	if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		presse = 0;
}

void mousemotion(int x, int y)	//--------------------------- Gestion du mouvement de la souris (rotation de la scène)
{
	if (presse) {
		anglex = anglex + (x - xold);
		angley = angley + (y - yold);
		glutPostRedisplay();
	}
	xold = x;
	yold = y;
}


/* ------------------------------------------------------------------- */
/*					Fonctions de Gestion du clavier			       	   */
/* ------------------------------------------------------------------- */

void zoomIO(GLdouble d)		//------------------------------- Mise à jour des paramètres de zoom
{
	minx += d;	maxx -= d;
	miny += d;	maxy -= d;
	minz += d;	maxz -= d;
	glOrtho(minx, maxx, miny, maxy, minz, maxz);
}

GLdouble sqr(GLdouble x)		//--------------------------- Fonction carré
{
	return (double)x*x;
}

void adjustFloorPosition()		//--------------------------- Ajustement de la position de la balle au sol
{
	if (Ball[1][1] != 1.0) {
		Ball[2][1] = Ball[1][3] * (Ball[1][1] - 1.0);
	}
	else {
		Ball[2][1] = 0.0;
	}
}

void setSSParams(GLdouble f)		//----------------------- Affectation des paramètres Squash & Stretch
{
	Ball[1][1] = (double)f;
	Ball[1][0] = (double)(1.0/sqrt(f));
	Ball[1][2] = (double)(1.0/sqrt(f));
	adjustFloorPosition();
}

void updateSSParams(GLdouble f)		//----------------------- Mise à jour des paramètres Squash & Stretch
{
	Ball[1][1] *= (double)f;
	Ball[1][0] /= (double)sqrt(f);
	Ball[1][2] /= (double)sqrt(f);
	adjustFloorPosition();
}

GLdouble getNewHeight()		//------------------------------- La trajectoire de la balle (parabole de profil)
{
	if ((crntIndex>=chapSize) && (crntIndex<=(rangeMax-chapSize))) { 
		return dispMaxHeight*(((crntIndex-chapSize)*((rangeMax-chapSize)-crntIndex))/sqr((rangeMax-2*chapSize)/2));
	}
	return 0.0;	// Dans le moment ou la balle s'écrase et rebondit on la laisse au sol ...
}

void moveBalle()		//----------------------------------- Gestion du déplacement de la Balle
{
	Ball[3][3] = getNewHeight();
	
	if (crntIndex<chapSize) {									// Premier douzième (au sol) :
		updateSSParams(sqr(factorUp));							//		la balle s'étire
	}
	if ((crntIndex>=chapSize)&&(crntIndex<=(5*chapSize))) {		// jusqu'au 5 douzièmes (en élévation) :
		updateSSParams(sqrt(sqrt(factorDown)));					//		la balle reprend progressivement sa forme
	}
	if ((crntIndex>(5*chapSize))&&(crntIndex<(7*chapSize))) {	// jusqu'au 7 douzièmes (passage du sommet) :
		setSSParams(1.0);										//		la balle est ronde
	}
	if ((crntIndex>=(7*chapSize))&&(crntIndex<(11*crntIndex))) {// jusqu'au 11 douzième (la descente) :
		updateSSParams(sqrt(sqrt(factorUp)));					//		la balle s'étire
	}
	if (crntIndex>=(11*chapSize)) {								// dernier douzième (au sol) :
		updateSSParams(sqr(factorDown));						// 		la balle s'écrase
	}
}

void clavier(unsigned char touche, int x, int y)	//------- Gestion des saisies clavier
{
	switch(touche) {
	
		/* Quitter */
		case 'q': 
			exit(0);
			
		/* affichage en mode plein */
		case 'p': 
      		glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      		break;
      		
    	/* affichage en mode fil de fer */
    	case 'f': 
      		glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      		break;
      		
      	/* Affichage en mode sommets seuls */	
    	case 's': 
      		glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
     		break;
     		
		/* Zoomer */
		case 'Z': 
			zoomIO(deltaZ);
			break;
			
		/* Dézoomer */
		case 'z': 
			zoomIO(-deltaZ);
			break;
			
		/* augmenter le rayon */
		case 'R':
			if (Ball[1][3]<2.0) Ball[1][3] += 0.1;
			break;
		
		/* diminuer le rayon */
		case 'r':
			if (Ball[1][3]>0.2) Ball[1][3] -= 0.1;
			break;
			
		default :
			break;
	}
	glutPostRedisplay();
}


/* ------------------------------------------------------------------- */
/*			Fonctions display d'OpenGL & de lancement			       */
/* ------------------------------------------------------------------- */

void affichage()	//--------------------------------------- Gestion de l'affichage
{
	/* Effacement de l'image avec la couleur de fond */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	/* Objets à afficher */
	ballScene();					
	glLoadIdentity();
	glRotated(anglex, 1.0, 0.0, 1.0);
	glRotated(angley, 0.0, 1.0, 1.0);
	
	/* Recadrage de l'espace de vue */
	glOrtho(minx, maxx, miny, maxy, minz, maxz);
	glFlush();
	glutSwapBuffers();
}

int main(int argc, char** argv)		//----------------------- Fonction de lancement de l'application
{
	/* Initialisation de glut et creation de la fenêtre */
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(1000, 1000);
	glutCreateWindow("Balle rebondissante");

	/* Initialisation des styles */
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glPointSize(3.0);
	glLineWidth(2.0);
	glEnable(GL_DEPTH_TEST);
	
	crntIndex = 0;									//------- Initialisation de l'incrément à 0
	chapSize = rangeMax/12.0;						//------- Calcul de la taille d'un douzième dans le range
	factorUp = pow(sFactorMax,1/chapSize);			//------- Calcul du facteur d'étirement pour un incrément
	factorDown = pow(sFactorMin,1/chapSize);		//------- 	"	  "		"	d'écrasement  "   "     "
	setSSParams(sFactorMin);						//------- La balle est écrasée au sol au départ
	
	/* Enregistrement des fonctions de rappel */
	glutDisplayFunc(affichage);
	glutKeyboardFunc(clavier);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(mousemotion);
	glutIdleFunc(idle);

	/* Entree dans la boucle principale glut */
	glutMainLoop();
	return 0;
}
