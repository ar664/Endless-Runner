#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <stdio.h>
//#include <string>
#include <iostream>
#include <ctime>
#include "Game.h"
using namespace std;

const int SDL_DELAY = 10;
const int TIME_CONSTANT = 300;
int SCREEN_HEIGHT = 480;
int SCREEN_WIDTH = 640;
//Char specifics
const int CHAR_HEIGHT = 27;
const int CHAR_WIDTH = 24;
const int CHAR_SPEED = 3;
//Rect Generation
int BOX_X_MIN;
int BOX_X_MAX;
int BOX_Y_MIN;
int BOX_Y_MAX;
int BOX_HEIGHT_MAX;
int BOX_WIDTH_MAX;
int BOX_MAX = 10;
int BOX_SPEED = 3;
//Start Up
const int START_X = SCREEN_WIDTH/3;
const int START_Y = SCREEN_HEIGHT/3 -50;
const int START_H = 250;
const int START_W = 250;
int BACKG_MAX = 4;
int BACKG_SPEED = 1;
int BACKG_WIDTH;
int BACKG_HEIGHT;
SDL_Rect BACKG_RECT_DEFAULT;

string fileName ("Config/Config.txt");
string fileNum;
const int MAX_LINES = 2;
FILE* file;

void GameData::LoadConfig()
{
	file = fopen(fileName.c_str(), "r");
	if (file == NULL)
	{
		fclose(file);
		return;
	}

	fseek(file,0,SEEK_END);
	int size = ftell(file);
	fclose(file);

	if (size < 1)
	{
		return;
	}
	printf("Size of file: %s is %d \n", fileName.c_str() , size);
	file = fopen(fileName.c_str(),"r");
	int lineCount = 0;
NextInt:
	char d = ' ';
	int currentNum= 0;

	do
	{
		fread((void*) &d, sizeof(char), 1, file);
		printf("%c\n",d);
		if (isdigit(d) && !feof(file))
		{
			fileNum += d;
		}

	}while( !feof(file) && d != '\n');
	lineCount++;
	currentNum = atoi(fileNum.c_str());

	switch(lineCount)
	{
	case 1:
		SCREEN_HEIGHT = currentNum;
		break;
	case 2:
		SCREEN_WIDTH = currentNum;
		break;
	default:
		break;
	}
	printf("%c \n",d);
	if (lineCount < MAX_LINES)
	{
		fileNum = "";
		goto NextInt;
	}
	fclose(file);

}



/**
 * Setup the variables for physics
 */
void GameData::SetPhysics()
{
	mCharX = 50;
	mCharY = SCREEN_HEIGHT - 100;
	mVelX = 0;
	mVelY = 0;
	mGrav = 9;
	mJump = -27;
	mTime = 0;
	srand(time(0));

}

/**
 * Setup Initial BackG Vars
 */
void GameData::SetBackG()
{
	for(int i = 0; i < BACKG_MAX; i++)
	{
		mTempBack[i].h = BACKG_HEIGHT;
		mTempBack[i].w = BACKG_WIDTH;
		mTempBack[i].x = i*BACKG_WIDTH;
		mTempBack[i].y = 0;
		mBackG[i] = &mTempBack[i];
	}
}

/**
 *Sets up the sound for the game
 *
 */
int GameData::SetSound()
{
	if ((mMusic = Mix_LoadMUS("Sound/beat.wav")) == NULL)
	{
		printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
		return -1;
	}
	if ((mBoom = Mix_LoadWAV("Sound/scratch.wav")) == NULL)
	{
		printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
		return -1;
	}
	if ((mPound = Mix_LoadWAV("Sound/low.wav")) == NULL)
	{
		printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
		return -1;
	}
	return 0;
}


/**
 *Sets up the loading of SDL
 *@return Creates window, renderer, and img flags
 */
int GameData::Setup()
{
	if (SDL_Init( SDL_INIT_EVERYTHING ) < 0)
	{
		printf("SDL Error: %s\n", SDL_GetError());
		return -1;
	}

	if ((mWindow = SDL_CreateWindow("Hello World",0,100,SCREEN_WIDTH,SCREEN_HEIGHT,0)) == NULL)
	{
		printf("SDL Error: %s\n", SDL_GetError());
		return -1;
	}


	 //Create renderer for mWindow 
	mRenderer = SDL_CreateRenderer( mWindow, -1, SDL_RENDERER_ACCELERATED ); 
	if( mRenderer == NULL ) 
	{
		printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() ); 
		return -1; 
	}
	else
	{
		//Initialize renderer color 
		SDL_SetRenderDrawColor( mRenderer, 0xFF, 0xFF, 0xFF, 0xFF ); 
		//Initialize PNG loading 
		int imgFlags = IMG_INIT_PNG; 
		if( !( IMG_Init( imgFlags ) & imgFlags ) ) 
		{
			printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() ); 
			return -1;
		}
	}
	if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
	{
		printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() ); 
		return -1; 
	}
	
	SetPhysics();
	mGameState = 0;
	mBox = false;
	mChanged = true;
	//Rect Generation
	BOX_X_MIN = SCREEN_WIDTH/3;
	BOX_X_MAX = 2*SCREEN_WIDTH/3;
	BOX_Y_MIN = SCREEN_HEIGHT/3;
	BOX_Y_MAX = 2*SCREEN_HEIGHT/3;
	BOX_HEIGHT_MAX = SCREEN_HEIGHT/5;
	BOX_WIDTH_MAX = SCREEN_HEIGHT/3;

	//Background
	BACKG_WIDTH = SCREEN_WIDTH/3;
	BACKG_HEIGHT = SCREEN_HEIGHT - 50;
	BACKG_RECT_DEFAULT.x = SCREEN_WIDTH;
	BACKG_RECT_DEFAULT.y = 0;
	BACKG_RECT_DEFAULT.w = BACKG_WIDTH;
	BACKG_RECT_DEFAULT.h = BACKG_HEIGHT;
	SetBackG();
	return 0;
}

/**
 *Closes all libraries and frees memory allocations
 */
int GameData::Shutdown()
{
	 //Free loaded image 
	SDL_DestroyTexture( mTexture ); 
	//Destroy mWindow 
	SDL_DestroyRenderer( mRenderer ); 
	SDL_DestroyWindow( mWindow );
	//Free Music
	Mix_FreeMusic(mMusic);
	Mix_FreeChunk(mBoom);
	Mix_FreeChunk(mPound);
	//Quit SDL subsystems 
	Mix_Quit();
	IMG_Quit();
	SDL_Quit();
	return 0;
}

/**
 * Get Rect coords based on frame
 * @param frame = current frame
 * @return SDL_Rect*
 */
SDL_Rect* GameData::GetFrameRect(int frame, int frameWidth, int frameHeight)
{
	SDL_Rect tempRect;
	tempRect.h = frameHeight;
	tempRect.w = frameWidth;
	tempRect.y = 0;
	switch(frame)
	{
		case 0: tempRect.x = 0; break;
		case 1: tempRect.x = frameWidth; break;
		case 2: tempRect.x = 2*frameWidth; break;
		default: tempRect.x = 0; break;
	}

	return &tempRect;
}


/**
 * Create Random Rect given limits
 * @param a[2] : Max/Min x location
 * @param b[2] : Max/Min y location
 * @param c : Max height
 * @param d : Max width
 * @return &mTempRect[GetRectNum()] : Adress of SDL_Rect created and stored by this function 
 */
SDL_Rect* GameData::LoadRect(int a[2], int b[2], int c, int d)
{
	SDL_Rect* newRect = NULL;
	if (GetRectNum() < 0)
	{
		return newRect;
	}
	int tempNum = GetRectNum();
	mTempRect[tempNum].x = rand() % (a[0]-a[1]) + a[1];
	mTempRect[tempNum].y = rand() % (b[0]-b[1]) + b[1];
	mTempRect[tempNum].h = rand() % c + c/2;
	mTempRect[tempNum].w = rand() % d + d/2;
	
	newRect = &mTempRect[tempNum];
	return newRect;
}


/**
 * Gets string for path of image and converts from Surface to Texture
 * @param string path : Dir of file
 * @return texture
 */
SDL_Texture* GameData::LoadTexture( std::string path ) 
{
	//The final texture 
	SDL_Texture* newTexture = NULL; 
	//Load image at specified path 
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() ); 
	if( loadedSurface == NULL ) 
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() ); 
	}
	else
	{
		//Create texture from surface pixels 
		newTexture = SDL_CreateTextureFromSurface( mRenderer, loadedSurface );
		if( newTexture == NULL ) 
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		} 
		//Get rid of old loaded surface 
		SDL_FreeSurface( loadedSurface ); 
	}
	return newTexture;
}

/**
 * Gives frame number
 * @param vel -Velocity of char
 * @param time -time stamp
 * @return frame num
 */
int GameData::CharFrameNum(int vel, int time)
{
	int tempTime = time%120;
	if(vel != 0)
	{
		if (tempTime < 40)
		{
			return 0;
		} else if (tempTime < 80)
		{
			return 1;
		} else{
			return 2;
		}
	}

	return 0;
}

/**
 * Gets a number of an empty slot from mRects[]
 * @return int i
 */
int GameData::GetRectNum()
{
	for (int i = 0; i < BOX_MAX; i++)
	{
		if (mRects[i] == NULL)
		{
			return i;
		}
	}
	return -1;
}

/**
 * Add a rectangle to the array
 *
 */
int GameData::AddRect()
{
	SDL_Rect* tempRect = NULL;
	int xBox[] = {BOX_X_MAX,BOX_X_MIN};
	int yBox[] = {BOX_Y_MAX,BOX_X_MIN};
	if ((tempRect = LoadRect(xBox,yBox,BOX_HEIGHT_MAX, BOX_WIDTH_MAX)) == NULL)
	{
		return -1;
	}
	mRects[GetRectNum()] = tempRect;
	return 0;
}

/**
 * Add a background to the array
 *
 */
int GameData::AddBackG()
{
	for (int i = 0; i < BACKG_MAX; i++)
	{
		if(mBackG[i] != NULL)
		{
			if (mBackG[i]->x < (-BACKG_WIDTH))
			{
				mBackG[i] = NULL;
			}
		}
		else
		{
			mTempBack[i] = BACKG_RECT_DEFAULT;
			mBackG[i] = &mTempBack[i];
		}
	}
	return 0;
}

/**
 * Draws floor using mGround texture
 * 
 */
int GameData::DrawFloor()
{
	SDL_Rect fillRect = { 0 , (SCREEN_HEIGHT-50), SCREEN_WIDTH, 50};
	SDL_RenderCopy( mRenderer, mGround, NULL, &fillRect);
	return 0;
}

/**
 * Draw your character on screen
 * Using mChar texture
 */
int GameData::DrawChar()
{
	SDL_Rect fillRect = {mCharX,mCharY, 50, 50};
	SDL_RenderCopy( mRenderer, mChar, GetFrameRect(CharFrameNum(mVelX,(mTime)),CHAR_WIDTH,CHAR_HEIGHT), &fillRect);
	return 0;
}

/**
 * Draw a platform
 *
 */
int GameData::DrawStuff()
{

	for(int i = 0;i < BOX_MAX; i++)
	{
		if (mRects[i] != NULL)
		{
			SDL_RenderCopy( mRenderer, mGround, NULL, mRects[i]);
		}
	}
	//std::cout << "RectNum = " << mRects[RectNum] << std::endl;
	return 0;
}

/**
*Drawing Background
*/
int GameData::DrawBackG()
{
	for(int i = 0;i < BACKG_MAX; i++)
	{
		if (mBackG[i] != NULL)
		{
			SDL_RenderCopy( mRenderer, mTexture, NULL, mBackG[i]);
		}
	}
	return 0;
}

/**
 * Draw StartUp screen
 *
 */
int GameData::DrawStartUp()
{
	SDL_Rect fillRect = {START_X,START_Y,START_W,START_H};
	SDL_RenderCopy(mRenderer, mStartText,NULL,&fillRect);
	return 0;

}

int GameData::DrawEnd()
{
	SDL_RenderCopy(mRenderer, mBye, NULL, NULL);
	return 0;
}


/**
 *Drawing to screen function using SDL Rendering
 *
 */
int GameData::Draw()
{
	if (mGameState == 0)
	{
		SDL_RenderClear( mRenderer ); 
		DrawBackG();
		DrawStartUp();
		SDL_RenderPresent( mRenderer );
		return 0;
	} 
	else if (mGameState == 1)
	{
		//Clear screen 
		SDL_RenderClear( mRenderer ); 

		//Render texture to screen
		DrawBackG();
		DrawFloor();
		DrawChar();
		DrawStuff();

		//Update screen
		SDL_RenderPresent( mRenderer );
		return 0;
	}
	else if (mGameState == 2)
	{
		SDL_RenderClear( mRenderer );
		DrawEnd();
		SDL_RenderPresent( mRenderer );
		SDL_Delay(500);
		return 0;
	}

}



 /**
  * Loads the textures and sounds
  */
 int GameData::LoadMedia() 
 {
	 //Load PNG texture 
	 if(( mTexture = LoadTexture( "Image/Right.png" )) == NULL ) 
	 { 
		 printf( "Failed to load texture image!\n" ); 
		 return -1;
	 }
	 if ((mChar = LoadTexture("Image/Bowser.png" )) == NULL)
	 {
		 printf( "Failed to load texture image!\n" ); 
		 return -1;
	 }
	 if ((mGround = LoadTexture("Image/Ground.png" )) == NULL)
	 {
		 printf( "Failed to load texture image!\n" ); 
		 return -1;
	 }
	 if ((mStartText = LoadTexture("Image/PressEnter.png" )) == NULL)
	 {
		 printf( "Failed to load texture image!\n" ); 
		 return -1;
	 }
	 if ((mBye = LoadTexture("Image/Bye.png" )) == NULL)
	 {
		 printf( "Failed to load texture image!\n" ); 
		 return -1;
	 }
	 if (SetSound() < 0)
	 {
		 return -1;
	 }
	 Mix_PlayMusic(mMusic,-1);
	 return 0;
 }
	 
 /**
  *Gets input from user
  *Quits if quit button is pressed
  */
int GameData::Input()
{
	SDL_Event e;
	SDL_PollEvent(&e);
	if (e.type == SDL_QUIT){
		mDone = true;
	} else if (e.type == SDL_KEYDOWN)
	{
		switch(e.key.keysym.sym)
		{
		case SDLK_SPACE:
			mVelY += mJump;
			printf("Jump \n");
			break;
		case SDLK_a:
			mVelX -= CHAR_SPEED;
			printf("Left \n");
			break;
		case SDLK_d:
			mVelX += CHAR_SPEED;
			printf("Right \n");
			break;
		case SDLK_m:
			if(Mix_PlayingMusic() == 0)
			{
				Mix_PlayMusic(mMusic,-1);
			} else
			{
				Mix_HaltMusic();
			}
			break;
		case SDLK_RETURN:
			printf("Return \n");
			mGameState = 1;
			break;
		case SDLK_ESCAPE:
			printf("Escape \n");
			mGameState = 2;
			mDone = true;
			break;
		case SDLK_p:
			printf("Pause \n");
			if (mGameState == 1)
			{
				mGameState = -1;
			} else if (mGameState == -1)
			{
				mGameState = 1;
			}
			break;
		default:
			printf("Nothing \n");
			break;
		};
		mChanged = true;
	}
	return 0;
}

/**
 *Moves the Character based on grav, position, input, and floor
 *
 */
int GameData::Move(){
	//X Velocity Control
	mCharX += mVelX;
	if  (( mCharX < 0 ) || ( mCharX + CHAR_WIDTH > SCREEN_WIDTH ) )
	{
		mCharX -= mVelX;
		mVelX = 0;
	}
	
	// Y Velocity Control
	if (mCharY != 0)
	{
		mCharY += mVelY;
	}
	if (mCharY + CHAR_WIDTH + 50 < SCREEN_HEIGHT)
	{
		mVelY += mGrav;
	}
	if  (( mCharY < 0 ) || ( mCharY + CHAR_WIDTH + 50 > SCREEN_HEIGHT ) )
	{
		mCharY -= mVelY;
		mVelY = 0;
	}

	return 0;
}

/**
 * Manages the positions and memory of the Rects on screen
 *
 */

void GameData::RectHandler()
{
	
	for(int i = 0; i < 10; i++)
	{
		if(mRects[i] != NULL)
		{
			
			mRects[i]->x -= BOX_SPEED;
			if ((mRects[i]->x) < BOX_SPEED)
			{
				mRects[i] = NULL;	
			}
		}
	}
	if (!(mTime % (TIME_CONSTANT) == 0))
	{
		return;
	}
	AddRect();
}

/**
 * Manages the positions and memory of the BackGrounds on screen
 *
 */

int GameData::BackGHandler()
{
	
	for(int i = 0; i < BACKG_MAX; i++)
	{
		if(mBackG[i] != NULL)
		{
			
			mBackG[i]->x -= BACKG_SPEED;
			//if ((mBackG[i]->x) < BACKG_SPEED)
			//{
			//	mBackG[i] = NULL;	
			//}
		}
	}
	AddBackG();
	mChanged = true;
	return 0;
}

/**
 * Update the characters position
 * Calls Move() if character is moving.
 */
int GameData::Update()
{
	if (mGameState == -1)
	{
		return -1;
	}
	mTime = SDL_GetTicks();
	RectHandler();
	BackGHandler();
	Move();
	return 0;
}

/**
 *Main game loop : Gets input and updates/draws
 * @return 0 : When done
 */

int GameData::Run()
{
	do
	{
		Input();
		if (Update() || mChanged)
		{
			Draw();
		}
		SDL_Delay(SDL_DELAY);
	}while(!mDone);

	return 0;
}