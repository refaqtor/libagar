/*	Public domain	*/

#ifndef _AGAR_WIDGET_SLIDER_H_
#define _AGAR_WIDGET_SLIDER_H_

#include <agar/gui/widget.h>

#include <agar/begin.h>

enum ag_slider_type {
	AG_SLIDER_HORIZ,
	AG_SLIDER_VERT
};

enum ag_slider_button {
	AG_SLIDER_BUTTON_NONE,
	AG_SLIDER_BUTTON_DEC,
	AG_SLIDER_BUTTON_INC,
	AG_SLIDER_BUTTON_SCROLL
};

typedef struct ag_slider {
	struct ag_widget wid;
	Uint flags;
#define AG_SLIDER_HFILL	0x01
#define AG_SLIDER_VFILL	0x02
#define AG_SLIDER_FOCUSABLE	0x04
#define AG_SLIDER_EXPAND	(AG_SLIDER_HFILL|AG_SLIDER_VFILL)

	int value;			/* Default value binding */
	int min, max;			/* Default range bindings */
	enum ag_slider_type type;	/* Style of scrollbar */
	int ctlPressed;			/* Control is pressed */
	int wControlPref;		/* Control size (preferred) */
	int wControl;			/* Control size (effective) */
	AG_Timeout incTo, decTo;	/* Timers for keyboard motion */
	int xOffs;			/* Cursor offset for scrolling */
	int extent;			/* Available area for scrolling */
	double rInc;			/* Base increment for real bindings */
	int    iInc;			/* Base increment for int bindings */
} AG_Slider;

#define AGSLIDER(p) ((AG_Slider *)p)

__BEGIN_DECLS
extern AG_WidgetClass agSliderClass;

AG_Slider *AG_SliderNew(void *, enum ag_slider_type, Uint);

AG_Slider *AG_SliderNewInt(void *, enum ag_slider_type, Uint, int *,
                           int *, int *);
AG_Slider *AG_SliderNewIntR(void *, enum ag_slider_type, Uint, int *,
                            int, int);
AG_Slider *AG_SliderNewUint(void *, enum ag_slider_type, Uint, Uint *,
                            Uint *, Uint *);
AG_Slider *AG_SliderNewUintR(void *, enum ag_slider_type, Uint, Uint *,
                             Uint, Uint);
AG_Slider *AG_SliderNewUint8(void *, enum ag_slider_type, Uint, Uint8 *,
                             Uint8 *, Uint8 *);
AG_Slider *AG_SliderNewUint8R(void *, enum ag_slider_type, Uint, Uint8 *,
                              Uint8, Uint8);
AG_Slider *AG_SliderNewSint8(void *, enum ag_slider_type, Uint, Sint8 *,
                             Sint8 *, Sint8 *);
AG_Slider *AG_SliderNewSint8R(void *, enum ag_slider_type, Uint, Sint8 *,
                              Sint8, Sint8);
AG_Slider *AG_SliderNewUint16(void *, enum ag_slider_type, Uint, Uint16 *,
                              Uint16 *, Uint16 *);
AG_Slider *AG_SliderNewUint16R(void *, enum ag_slider_type, Uint, Uint16 *,
                               Uint16, Uint16);
AG_Slider *AG_SliderNewSint16(void *, enum ag_slider_type, Uint, Sint16 *,
                              Sint16 *, Sint16 *);
AG_Slider *AG_SliderNewSint16R(void *, enum ag_slider_type, Uint, Sint16 *,
                               Sint16, Sint16);
AG_Slider *AG_SliderNewUint32(void *, enum ag_slider_type, Uint, Uint32 *,
                              Uint32 *, Uint32 *);
AG_Slider *AG_SliderNewUint32R(void *, enum ag_slider_type, Uint, Uint32 *,
                               Uint32, Uint32);
AG_Slider *AG_SliderNewSint32(void *, enum ag_slider_type, Uint, Sint32 *,
                              Sint32 *, Sint32 *);
AG_Slider *AG_SliderNewSint32R(void *, enum ag_slider_type, Uint, Sint32 *,
                               Sint32, Sint32);
#ifdef HAVE_64BIT
AG_Slider *AG_SliderNewUint64(void *, enum ag_slider_type, Uint, Uint64 *,
                              Uint64 *, Uint64 *);
AG_Slider *AG_SliderNewUint64R(void *, enum ag_slider_type, Uint, Uint64 *,
                               Uint64, Uint64);
AG_Slider *AG_SliderNewSint64(void *, enum ag_slider_type, Uint, Sint64 *,
                              Sint64 *, Sint64 *);
AG_Slider *AG_SliderNewSint64R(void *, enum ag_slider_type, Uint, Sint64 *,
                               Sint64, Sint64);
#endif

AG_Slider *AG_SliderNewFlt(void *, enum ag_slider_type, Uint, float *,
                           float *, float *);
AG_Slider *AG_SliderNewFltR(void *, enum ag_slider_type, Uint, float *,
                            float, float);
AG_Slider *AG_SliderNewDbl(void *, enum ag_slider_type, Uint, double *,
                           double *, double *);
AG_Slider *AG_SliderNewDblR(void *, enum ag_slider_type, Uint, double *,
                            double, double);
#ifdef HAVE_LONG_DOUBLE
AG_Slider *AG_SliderNewLongDbl(void *, enum ag_slider_type, Uint,
                               long double *, long double *, long double *);
AG_Slider *AG_SliderNewLongDblR(void *, enum ag_slider_type, Uint,
                                long double *, long double, long double);
#endif

void AG_SliderSetControlSize(AG_Slider *, int);
void AG_SliderSetIntIncrement(AG_Slider *, int);
void AG_SliderSetRealIncrement(AG_Slider *, double);
__END_DECLS

#include <agar/close.h>
#endif	/* _AGAR_WIDGET_SLIDER_H_ */
