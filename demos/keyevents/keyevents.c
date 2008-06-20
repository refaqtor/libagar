/*	Public domain	*/
/*
 * This application demonstrates a simple way to catch and process arbitrary
 * keyboard events.
 */

#include <agar/core.h>
#include <agar/gui.h>

/*
 * This is our keyup/keydown event handler. The arguments are documented in
 * the EVENTS section of the AG_Window(3) manual page.
 */
static void
MyKeyboardHandler(AG_Event *event)
{
	SDLKey sym = AG_SDLKEY(1);
	SDLMod mod = AG_SDLMOD(2);
	Uint32 unicode = (Uint32)AG_INT(3);

	printf("%s: sym=%u, modifier=0x%x, unicode=0x%x\n",
	    event->name, (unsigned)sym, (unsigned)mod, unicode);
}

static void
Quit(AG_Event *event)
{
	AG_Quit();
}

static void
CreateWindow(void)
{
	AG_Window *win;

	win = AG_WindowNew(AG_WINDOW_PLAIN);
	AG_LabelNew(win, 0, "Press any key.");

	/*
	 * Attach our event handler function to both keydown and keyup
	 * events of the Window object. Note that we could have used
	 * any other object derived from the Widget class.
	 */
	AG_SetEvent(win, "window-keydown", MyKeyboardHandler, NULL);
	AG_SetEvent(win, "window-keyup", MyKeyboardHandler, NULL);
	
	/*
	 * Enable reception of keydown/keyup events by the window, regardless
	 * of whether it is currently focused or not.
	 */
	AGWIDGET(win)->flags |= AG_WIDGET_UNFOCUSED_KEYUP;
	AGWIDGET(win)->flags |= AG_WIDGET_UNFOCUSED_KEYDOWN;

	AG_ButtonNewFn(win, 0, "Quit", Quit, NULL);

	AG_WindowMaximize(win);
	AG_WindowShow(win);
}

int
main(int argc, char *argv[])
{
	if (AG_InitCore("agar-keyevents-demo", 0) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (1);
	}
	if (AG_InitVideo(200, 100, 32, AG_VIDEO_RESIZABLE) == -1) {
		fprintf(stderr, "%s\n", AG_GetError());
		return (-1);
	}
	AG_BindGlobalKey(SDLK_ESCAPE, KMOD_NONE, AG_Quit);
	AG_BindGlobalKey(SDLK_F8, KMOD_NONE, AG_ViewCapture);
	CreateWindow();
	AG_EventLoop();
	AG_Destroy();
	return (0);
}