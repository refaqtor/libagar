/*	$Csoft: widget.h,v 1.94 2005/05/21 05:53:30 vedge Exp $	*/
/*	Public domain	*/

#ifndef _AGAR_WIDGET_H_
#define _AGAR_WIDGET_H_

#include <config/have_opengl.h>

#include <engine/widget/style.h>
#include <engine/widget/colors.h>
#include <engine/view.h>

#include "begin_code.h"

#define WIDGET_TYPE_MAX		32
#define WIDGET_BINDING_NAME_MAX	16

struct widget_ops {
	const struct object_ops	obops;
	void (*draw)(void *);
	void (*scale)(void *, int, int);
	void (*scale_default)(void *);
	void (*scale_minimum)(void *);
	int (*spacing)(void *, void *);
};

enum widget_binding_type {
	WIDGET_NONE,
	WIDGET_BOOL,
	WIDGET_UINT,
	WIDGET_INT,
	WIDGET_UINT8,
	WIDGET_SINT8,
	WIDGET_UINT16,
	WIDGET_SINT16,
	WIDGET_UINT32,
	WIDGET_SINT32,
	WIDGET_FLOAT,
	WIDGET_DOUBLE,
	WIDGET_STRING,
	WIDGET_POINTER,
	WIDGET_PROP
};

enum widget_size_spec {
	WIDGET_BAD_SPEC,
	WIDGET_PIXELS,			/* Pixel count */
	WIDGET_PERCENT,			/* % of available space */
	WIDGET_STRINGLEN,		/* Width of given string */
	WIDGET_FILL			/* Fill remaining space */
};

struct widget_binding {
	char	name[WIDGET_BINDING_NAME_MAX];
	int	type;
	int	vtype;

	pthread_mutex_t	*mutex;	
	void		*p1, *p2;
	size_t		 size;

	SLIST_ENTRY(widget_binding) bindings;
};

struct widget {
	struct object obj;

	char	 type[WIDGET_TYPE_MAX];
	int	 flags;
#define WIDGET_FOCUSABLE	  0x01	/* Can grab focus */
#define WIDGET_FOCUSED		  0x02	/* Holds focus (optimization) */
#define WIDGET_UNFOCUSED_MOTION	  0x04	/* Unfocused mousemotion events */
#define WIDGET_UNFOCUSED_BUTTONUP 0x08	/* Unfocused mousebuttonup events */
#define WIDGET_CLIPPING		  0x10	/* Enable clipping */
#define WIDGET_WFILL		  0x20	/* Expand to fill available width */
#define WIDGET_HFILL		  0x40	/* Expand to fill available height */
#define WIDGET_EXCEDENT		  0x80	/* Used internally for scale */

	int	 cx, cy;		/* Coordinates in view (upper left) */
	int	 cx2, cy2;		/* Coordinates in view (lower right) */
	int	 x, y;			/* Coordinates in container */
	int	 w, h;			/* Allocated geometry */

	const struct style *style;	/* Style properties (inherited from
					   parent by default) */

	SDL_Surface	**surfaces;	/* Registered surfaces */
	u_int		 nsurfaces;
#ifdef HAVE_OPENGL
	GLuint		*textures;	/* Cached OpenGL textures */
	GLfloat		*texcoords;	/* Cached texture coordinates */
#endif
	pthread_mutex_t			 bindings_lock;
	SLIST_HEAD(, widget_binding)	 bindings;
};

#define WIDGET(wi)		((struct widget *)(wi))
#define WIDGET_OPS(ob)		((struct widget_ops *)OBJECT(ob)->ops)
#define WIDGET_SCALE(wi, w, h)	WIDGET_OPS(wi)->scale((wi), (w), (h))
#define WIDGET_SURFACE(wi, ind)	WIDGET(wi)->surfaces[ind]
#define WIDGET_TEXTURE(wi, ind)	WIDGET(wi)->textures[ind]
#define WIDGET_TEXCOORD(wi, ind) WIDGET(wi)->texcoords[(ind)*4]

__BEGIN_DECLS
extern int kbd_delay;
extern int kbd_repeat;
extern int mouse_dblclick_delay;
extern int mouse_spin_delay;
extern int mouse_spin_ival;

void	 widget_init(void *, const char *, const void *, int);
void	 widget_destroy(void *);
void	 widget_draw(void *);
void	 widget_scale(void *, int, int);

#define widget_focused(w) ((w)->flags & WIDGET_FOCUSED)

void		 widget_set_type(void *, const char *);
void		 widget_focus(void *);
void		 widget_unset_focus(void *);
struct widget	*widget_find_focus(void *);
__inline__ int	 widget_holds_focus(void *);
__inline__ int	 widget_relative_area(void *, int, int);
__inline__ int	 widget_area(void *, int, int);
void		 widget_update_coords(void *, int, int);
struct window	*widget_parent_window(void *);

int		 widget_map_surface(void *, SDL_Surface *);
__inline__ void	 widget_replace_surface(void *, int, SDL_Surface *);
__inline__ void	 widget_update_surface(void *, int);

void	 widget_blit(void *, SDL_Surface *, int, int);
void	 widget_blit_from(void *, void *, int, SDL_Rect *, int, int);

#define widget_unmap_surface(w, n) widget_replace_surface((w),(n),NULL)
#define	widget_blit_surface(p,n,x,y) widget_blit_from((p),(p),(n),NULL,(x),(y))

__inline__ void widget_put_pixel(void *, int, int, Uint32);
__inline__ void widget_blend_pixel(void *, int, int, Uint8 [4],
		                   enum view_blend_func);

void  widget_mousemotion(struct window *, struct widget *, int, int, int, int,
	                 int);
void  widget_mousebuttonup(struct window *, struct widget *, int, int, int);
int   widget_mousebuttondown(struct window *, struct widget *, int, int, int);

struct widget_binding	*widget_bind(void *, const char *,
			             enum widget_binding_type, ...);
struct widget_binding	*widget_bind_protected(void *, const char *,
			                       pthread_mutex_t *,
					       enum widget_binding_type, ...);
struct widget_binding	*widget_get_binding(void *, const char *, ...);
__inline__ void		 widget_binding_lock(struct widget_binding *);
__inline__ void		 widget_binding_unlock(struct widget_binding *);
__inline__ void		 widget_binding_modified(struct widget_binding *);
__inline__ int		 widget_copy_binding(void *, const char *, void *,
			                     const char *);

__inline__ u_int	 widget_get_uint(void *, const char *);
__inline__ int		 widget_get_int(void *, const char *);
#define			 widget_get_bool widget_get_int
__inline__ Uint8	 widget_get_uint8(void *, const char *);
__inline__ Sint8	 widget_get_sint8(void *, const char *);
__inline__ Uint16	 widget_get_uint16(void *, const char *);
__inline__ Sint16	 widget_get_sint16(void *, const char *);
__inline__ Uint32	 widget_get_uint32(void *, const char *);
__inline__ Sint32	 widget_get_sint32(void *, const char *);
__inline__ float	 widget_get_float(void *, const char *);
__inline__ double	 widget_get_double(void *, const char *);

__inline__ void	  *widget_get_pointer(void *, const char *);
__inline__ char	  *widget_get_string(void *, const char *);
__inline__ size_t  widget_copy_string(void *, const char *, char *, size_t)
		       BOUNDED_ATTRIBUTE(__string__, 3, 4);

__inline__ void	 widget_set_uint(void *, const char *, u_int);
__inline__ void	 widget_set_int(void *, const char *, int);
#define		 widget_set_bool widget_set_int
__inline__ void	 widget_set_uint8(void *, const char *, Uint8);
__inline__ void	 widget_set_sint8(void *, const char *, Sint8);
__inline__ void	 widget_set_uint16(void *, const char *, Uint16);
__inline__ void	 widget_set_sint16(void *, const char *, Sint16);
__inline__ void	 widget_set_uint32(void *, const char *, Uint32);
__inline__ void	 widget_set_sint32(void *, const char *, Sint32);
__inline__ void	 widget_set_float(void *, const char *, float);
__inline__ void	 widget_set_double(void *, const char *, double);
__inline__ void	 widget_set_string(void *, const char *, const char *);
__inline__ void	 widget_set_pointer(void *, const char *, void *);

enum widget_size_spec widget_parse_sizespec(const char *, int *);
__END_DECLS

#include "close_code.h"
#endif /* _AGAR_WIDGET_H_ */
