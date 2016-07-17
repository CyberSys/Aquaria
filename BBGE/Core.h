/*
Copyright (C) 2007, 2010 - Bit-Blot

This file is part of Aquaria.

Aquaria is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/
#ifndef __core__
#define __core__

#include "Base.h"
#include "RenderObject.h"
#include "SoundManager.h"
#include "Event.h"
#include "StateManager.h"
#include "Effects.h"
#include "Localization.h"

#include "DarkLayer.h"

#include "GameKeys.h"


class ParticleEffect;
class Joystick;

class ParticleManager;

struct ScreenMode
{
	ScreenMode() { idx = x = y = hz = 0; }
	ScreenMode(int i, int x, int y, int hz) : idx(i), x(x), y(y), hz(hz) {}

	int idx, x, y, hz;
};

struct CoreSettings
{
	CoreSettings() { renderOn = true; updateOn = true; runInBackground = false; prebufferSounds = false; }
	bool renderOn;
	bool runInBackground;
	bool updateOn; // NOT IMPLEMENTED YET
	bool prebufferSounds;
};

enum CoreLayers
{
	LR_NONE		= -1
};

const int NO_FOLLOW_CAMERA = -999;

class AfterEffectManager;

class Texture;

const int baseVirtualWidth		= 800;
const int baseVirtualHeight		= 600;

enum ButtonState { UP = 0, DOWN };

struct MouseButtons
{
	MouseButtons ()
	{
		left = UP;
		right = UP;
		middle = UP;
	}

	ButtonState left, right, middle;
	ButtonState extra[mouseExtraButtons];
};

struct Mouse
{
	Mouse()
	{
		scrollWheel = scrollWheelChange = lastScrollWheel = 0;
		buttonsEnabled = true;
	}
	Vector position, lastPosition;
	MouseButtons buttons;
	MouseButtons pure_buttons;
	unsigned rawButtonMask;
	Vector change;
	bool buttonsEnabled;

	int scrollWheel, scrollWheelChange, lastScrollWheel;
};

enum FollowCameraLock
{
	FCL_NONE		= 0,
	FCL_HORZ		= 1,
	FCL_VERT		= 2
};

typedef std::vector <RenderObject*> RenderObjects;

class RenderObjectLayer
{
public:
	RenderObjectLayer();
	~RenderObjectLayer();
	void add(RenderObject* r);
	void remove(RenderObject* r);
	void moveToFront(RenderObject *r);
	void moveToBack(RenderObject *r);
	void setCull(bool cull);
	void setOptimizeStatic(bool opt);
	void sort();
	void renderPass(int pass);
	void reloadDevice();

	inline bool empty()
	{
		return objectCount == 0;
	}

	inline RenderObject *getFirst()
	{
		iter = 0;
		return getNext();
	}

	RenderObject *getNext()
	{
		const int size = renderObjects.size();
		int i;
		for (i = iter; i < size; i++)
		{
			if (renderObjects[i] != 0)
				break;
		}
		if (i < size)
		{
			iter = i+1;
			return renderObjects[i];
		}
		else
		{
			iter = i;
			return 0;
		}
		return 0;
	}

	//inclusive
	int startPass, endPass;
	bool visible;
	float followCamera;

	int followCameraLock;
	bool cull;
	bool update;

	int mode;

	Vector color;

protected:

	void clearDisplayList();
	void generateDisplayList();
	inline void renderOneObject(RenderObject *robj);

	bool optimizeStatic;
	bool displayListValid;
	int displayListGeneration;
	struct DisplayListElement {
		DisplayListElement() {isList = false; u.robj = 0;}
		bool isList;  // True if this is a GL display list
		union {
			RenderObject *robj;
			unsigned listID;
		} u;
	};
	std::vector<DisplayListElement> displayList;

	RenderObjects renderObjects;
	int objectCount;
	int firstFreeIdx;
	int iter;
};

class Core : public ActionMapper, public StateManager
{
public:

	// init
	Core(const std::string &filesystem, const std::string& extraDataDir, int numRenderLayers, const std::string &appName="BBGE", int particleSize=1024, std::string userDataSubFolder="");
	void initPlatform(const std::string &filesystem);
	~Core();

	virtual void init();

	void initRenderObjectLayers(int num);

	void applyState(const std::string &state);

	bool createWindow(int width, int height, int bits, bool fullscreen, std::string windowTitle="");

	void clearBuffers();
	void render(int startLayer=-1, int endLayer=-1, bool useFrameBufferIfAvail=true);
	void showBuffer();
	void quit();
	bool isShuttingDown();
	bool isWindowFocus();

	void instantQuit();

	void cacheRender();

	void setSDLGLAttributes();

	void reloadResources();
	void unloadResources();

	std::string getPreferencesFolder();
	std::string getUserDataFolder();

	void resetCamera();

	virtual void shutdown();

	void run(float runTime = -1); // can use main



	// state functions

	std::string getTextureLoadName(const std::string &texture);

	void setMousePosition(const Vector &p);

	void toggleScreenMode(int t=0);

	void enable2D(int pixelScaleX=0, int pixelScaleY=0, bool forcePixelScale=false);
	void addRenderObject(RenderObject *o, int layer=0);
	void switchRenderObjectLayer(RenderObject *o, int toLayer);
	void addTexture(Texture *r);
	CountedPtr<Texture> findTexture(const std::string &name);
	void removeTexture(Texture *res);
	void clearResources();

	CountedPtr<Texture> addTexture(const std::string &texture);

	PostProcessingFX postProcessingFx;

	enum RemoveRenderObjectFlag { DESTROY_RENDER_OBJECT=0, DO_NOT_DESTROY_RENDER_OBJECT };
	void removeRenderObject(RenderObject *r, RemoveRenderObjectFlag flag = DESTROY_RENDER_OBJECT);

	void setMouseConstraint(bool on);
	void setMouseConstraintCircle(const Vector& pos, float mouseCircle);

	void setReentryInputGrab(int on);

	virtual void action(int id, int state, int source){}

	bool exists(const std::string &file);

	void enqueueRenderObjectDeletion(RenderObject *object);
	void clearGarbage();


	bool isNested() { return nestedMains > 1; }
	int getNestedMains() { return nestedMains; }
	void quitNestedMain();

	int getWindowWidth() { return width; }
	int getWindowHeight() { return height; }

	unsigned getTicks();

	void resetGraphics(int w, int h, int fullscreen=-1, int vsync=-1, int bpp=-1);



	void setDockIcon(const std::string &ident);

	Vector getGameCursorPosition();
	Vector getGamePosition(const Vector &v);

	Vector screenCenter;

	void print(int x, int y, const char *str, float sz=1);

	std::vector<Texture*> resources;

	RenderObjectLayer *getRenderObjectLayer(int i);
	std::vector <int> renderObjectLayerOrder;

	typedef std::vector<RenderObjectLayer> RenderObjectLayers;
	RenderObjectLayers renderObjectLayers;

	RenderObjects garbage;

	SoundManager *sound;

	float aspect;

	int width, height;

	enum Modes { MODE_NONE=-1, MODE_3D=0, MODE_2D };

	InterpolatedVector globalScale;
	Vector globalResolutionScale;
	Vector screenCapScale;

	virtual void onResetScene(){}

	virtual void onPlayedVoice(const std::string &name){}

	Vector cameraPos;

	int fps;
	bool loopDone;

	Mouse mouse;

	AfterEffectManager *afterEffectManager;

	ParticleManager *particleManager;



	void setBaseTextureDirectory(const std::string &baseTextureDirectory)
	{ this->baseTextureDirectory = baseTextureDirectory; }
	std::string getBaseTextureDirectory()
	{
		return baseTextureDirectory;
	}


	virtual bool canChangeState();
	void resetTimer();

	inline int getVirtualWidth()
	{
		return virtualWidth;
	}

	inline int getVirtualHeight()
	{
		return virtualHeight;
	}

	unsigned char *grabScreenshot(int x, int y, int w, int h);
	unsigned char *grabCenteredScreenshot(int w, int h);
	int saveScreenshotTGA(const std::string &filename);
	void save64x64ScreenshotTGA(const std::string &filename);
	void saveSizedScreenshotTGA(const std::string &filename, int sz, int crop34);
	void saveCenteredScreenshotTGA(const std::string &filename, int sz);

	bool minimized;
	std::string getEnqueuedJumpState();
	float cullRadius;
	float cullRadiusSqr;
	Vector cullCenter;
	unsigned int renderObjectCount, processedRenderObjectCount, totalRenderObjectCount;
	float invGlobalScale, invGlobalScaleSqr;

	void globalScaleChanged();

	void screenshot();

	void clearRenderObjects();

	bool getKeyState(int k);
	bool getMouseButtonState(int m);

	int currentLayerPass;
	int keys[KEY_MAXARRAY];
	virtual void debugLog(const std::string &s);
	virtual void errorLog(const std::string &s);
	void messageBox(const std::string &title, const std::string &msg);
	bool getShiftState();
	bool getAltState();
	bool getCtrlState();

	virtual void generateCollisionMask(RenderObject *r){}

	DarkLayer darkLayer;

	void setupRenderPositionAndScale();
	void setupGlobalResolutionScale();


	int particlesPaused;


	bool joystickEnabled;

	bool debugLogTextures;

	void setClearColor(const Vector &c);
	Vector getClearColor();
	int flipMouseButtons;
	void initFrameBuffer();
	FrameBuffer frameBuffer;
	void updateRenderObjects(float dt);
	bool joystickAsMouse;
	virtual void prepScreen(bool t){}

	bool updateMouse;
	bool frameOutputMode;

	int overrideStartLayer, overrideEndLayer;

	void setWindowCaption(const std::string &caption, const std::string &icon);

	ParticleEffect* createParticleEffect(const std::string &name, const Vector &position, int layer, float rotz=0);

	std::string secondaryTexturePath;

	bool hasFocus();

	float aspectX, aspectY;

	float get_old_dt() { return old_dt; }
	float get_current_dt() { return current_dt; }

	bool debugLogActive;

	void setInputGrab(bool on);

	bool isFullscreen();

	int viewOffX, viewOffY;

	int getVirtualOffX();
	int getVirtualOffY();

	void centerMouse();

	Vector center;

	void enable2DWide(int rx, int ry);

	void enumerateScreenModes();

	std::vector<ScreenMode> screenModes;

	void pollEvents();

	CoreSettings settings;

	int tgaSave(const char *filename, short int width, short int height, unsigned char	pixelDepth, unsigned char	*imageData);
	int zgaSave(const char *filename, short int width, short int height, unsigned char	pixelDepth, unsigned char	*imageData);

	volatile int dbg_numThreadDecoders;

	virtual void onBackgroundUpdate();

	void initLocalization();

protected:

	void updateCullData();

	std::string userDataFolder;

	int grabInputOnReentry;

	int virtualOffX, virtualOffY;

	void initIcon();

	float old_dt;
	float current_dt;

	std::string debugLogPath;

	virtual void onReloadResources();

	CountedPtr<Texture> doTextureAdd(const std::string &texture, const std::string &name, std::string internalTextureName);

	void deleteRenderObjectMemory(RenderObject *r);
	bool _hasFocus;
	bool lib_graphics, lib_sound, lib_input;
	Vector clearColor;
	bool updateCursorFromMouse;
	virtual void unloadDevice();
	virtual void reloadDevice();

	std::string appName;
	bool mouseConstraint;
	float mouseCircle;
	Vector mouseConstraintCenter;

	bool doMouseConstraint();

	virtual void onMouseInput(){}
	bool doScreenshot;
	float baseCullRadius;
	bool initSoundLibrary(const std::string &defaultDevice);
	bool initInputLibrary();
	void initJoystickLibrary();
	bool initGraphicsLibrary(int w, int h, bool fullscreen, int vsync, int bpp, bool recreate=true);
	void shutdownInputLibrary();
	void shutdownJoystickLibrary();
	void shutdownGraphicsLibrary(bool kill=true);
	void shutdownSoundLibrary();

	void detectJoysticks();
	void clearJoysticks();
	virtual void onJoystickAdded(int deviceID);
	virtual void onJoystickRemoved(int instanceID);

	int afterEffectManagerLayer;
	Vector cameraOffset;
	std::vector<float> avgFPS;
	virtual void modifyDt(float &dt){}
	void setPixelScale(int pixelScaleX, int pixelScaleY);


	int virtualHeight, virtualWidth;

	bool shuttingDown;
	bool quitNestedMainFlag;
	int nestedMains;
	std::string baseTextureDirectory;

	int nowTicks, thenTicks;

	int _vsync, _bpp;
	bool _fullscreen;

	int numSavedScreenshots;

	CountedPtr<Texture> texError;

	int tgaSaveSeries(char	*filename,  short int width, short int height, unsigned char pixelDepth, unsigned char *imageData);
	virtual void onUpdate(float dt);
	virtual void onRender(){}

	void setupFileAccess();
	std::string _extraDataDir;

	std::vector<ActionButtonStatus*> actionStatus;
	void updateActionButtons();
	void clearActionButtons();

public:
	// inclusive!
	inline int getMaxActionStatusIndex() const { return int(actionStatus.size()) - 2; }
	// pass -1 for is a sentinel that captures all input
	inline ActionButtonStatus *getActionStatus(int idx) { return actionStatus[idx + 1]; }

	Joystick *getJoystick(int idx); // warning: may return NULL/contain holes
	// not the actual number of joysticks!
	size_t getNumJoysticks() const { return joysticks.size(); }
	Joystick *getJoystickForSourceID(unsigned sourceID);
private:
	std::vector<Joystick*> joysticks;
};

extern Core *core;

#include "RenderObject_inline.h"

#endif
